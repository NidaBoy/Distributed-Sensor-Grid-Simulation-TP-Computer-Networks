#include "client.h"
#include "message_handler.h"
#include "sensor_utils.h"
#include "neighbor_manager.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t neighbor_manager_mutex; // Mutex para sincronização
NeighborManager neighbor_manager; // NeighborManager compartilhado

void *receive_messages(void *args);


void start_client(const char *server_ip, int port, const char *type, int x, int y) {
    int sock;
    struct sockaddr_storage server_addr;
    socklen_t addr_len;
    SensorMessage msg;

    // Inicializar gerenciador de vizinhos
    float inicial_measurement = generate_measurement(type);

    pthread_mutex_init(&neighbor_manager_mutex, NULL); // Inicializar o mutex

    init_neighbor_manager(&neighbor_manager, x, y, inicial_measurement);
    // Configuração inicial da mensagem
    strncpy(msg.type, type, sizeof(msg.type) - 1);
    msg.coords[0] = neighbor_manager.x;
    msg.coords[1] = neighbor_manager.y;
    msg.measurement = neighbor_manager.measurement;

    // Determinar se o endereço é IPv4 ou IPv6
    if (strchr(server_ip, ':')) { // IPv6
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&server_addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        if (inet_pton(AF_INET6, server_ip, &addr6->sin6_addr) <= 0) {
            perror("Endereço IPv6 inválido");
            exit(EXIT_FAILURE);
        }
        addr_len = sizeof(struct sockaddr_in6);
    } else { // IPv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&server_addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        if (inet_pton(AF_INET, server_ip, &addr4->sin_addr) <= 0) {
            perror("Endereço IPv4 inválido");
            exit(EXIT_FAILURE);
        }
        addr_len = sizeof(struct sockaddr_in);
    }

    // Criar socket
    if ((sock = socket(server_addr.ss_family, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Conectar ao servidor
    if (connect(sock, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Thread para receber mensagens
    pthread_t receiver_thread;
    int *sock_ptr = malloc(sizeof(int));
    *sock_ptr = sock;
    if (pthread_create(&receiver_thread, NULL, receive_messages, sock_ptr) != 0) {
        perror("Erro ao criar thread de recebimento");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Determinar intervalo de envio baseado no tipo
    int sleep_time;
    if (strcmp(type, "temperature") == 0) {
        sleep_time = 5;
    } else if (strcmp(type, "humidity") == 0) {
        sleep_time = 7;
    } else if (strcmp(type, "air_quality") == 0) {
        sleep_time = 10;
    } else {
        fprintf(stderr, "Tipo de sensor inválido: %s\n", type);
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Loop de envio periódico
    while (1) {
        sleep(sleep_time);

        pthread_mutex_lock(&neighbor_manager_mutex);
        msg.measurement = neighbor_manager.measurement;
        pthread_mutex_unlock(&neighbor_manager_mutex);

        send(sock, &msg, sizeof(msg), 0);
        //printf("Mensagem enviada:\n");
        //printf("Tipo: %s, Coordenadas: (%d, %d), Medição: %.4f\n\n",
        //       msg.type, msg.coords[0], msg.coords[1], msg.measurement);
    }

    pthread_join(receiver_thread, NULL);
    pthread_mutex_destroy(&neighbor_manager_mutex); // Destruir o mutex
    close(sock);
}

void *receive_messages(void *args) {
    int sock = *(int *)args;
    free(args);
    SensorMessage msg;

    while (1) {
        if (receive_message(sock, &msg)) {
            pthread_mutex_lock(&neighbor_manager_mutex); // Travar o mutex

            // Processar mensagem recebida
            float distance = calculate_distance(neighbor_manager.x, neighbor_manager.y, msg.coords[0], msg.coords[1]);

            if (is_the_same_sensor(&neighbor_manager, msg.coords[0], msg.coords[1])){
                printf("log:\n%s sensor in (%d,%d)\nmeasurement: %.4f\naction: same location\n\n",
                       msg.type, msg.coords[0], msg.coords[1], msg.measurement);
                pthread_mutex_unlock(&neighbor_manager_mutex); // Destravar o mutex
                continue;
            }

            if (msg.measurement == SENSOR_DESCONECTED){
                remove_neighbor(&neighbor_manager, msg.coords[0], msg.coords[1]);
                printf("log:\n%s sensor in (%d,%d)\nmeasurement: %.4f\naction: removed\n\n",
                   msg.type, msg.coords[0], msg.coords[1], msg.measurement);
                pthread_mutex_unlock(&neighbor_manager_mutex); // Destravar o mutex
                continue;
            }

            if (is_sensor_not_in_neighbors(&neighbor_manager, msg.coords[0], msg.coords[1])){
                if (neighbor_manager.neighbor_count < MAX_SENSORS) {
                    add_sensor_if_not_exists(&neighbor_manager, msg.coords[0], msg.coords[1]);
                }
                pthread_mutex_unlock(&neighbor_manager_mutex); // Destravar o mutex
                continue;
            }

            if (is_not_a_neighbor(&neighbor_manager, msg.coords[0], msg.coords[1])) {
                printf("log:\n%s sensor in (%d,%d)\nmeasurement: %.4f\naction: not neighbor\n\n",
                        msg.type, msg.coords[0], msg.coords[1], msg.measurement);
                pthread_mutex_unlock(&neighbor_manager_mutex); // Destravar o mutex
                continue;
            }

            float correction = (msg.measurement - neighbor_manager.measurement) / (distance + 1) * 0.1;
            neighbor_manager.measurement += correction;

            printf("log:\n%s sensor in (%d,%d)\nmeasurement: %.4f\naction: correction of %.4f\n\n",
                   msg.type, msg.coords[0], msg.coords[1], msg.measurement, correction);

            pthread_mutex_unlock(&neighbor_manager_mutex); // Destravar o mutex
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    validate_arguments(argc, argv);

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *type = argv[4];
    int x = atoi(argv[6]);
    int y = atoi(argv[7]);

    srand(time(NULL));
    start_client(server_ip, port, type, x, y);

    return EXIT_SUCCESS;
}