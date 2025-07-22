#include "server.h"
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CLIENTS 12
#define MAX_TOPICS 3

typedef struct {
    char type[12]; // Tópico
    int client_socks[MAX_CLIENTS]; // Lista de clientes assinantes
    int subscriber_count; // Contador de assinantes
} Topic;

static Topic topics[MAX_TOPICS] = {
    {"temperature", {0}, 0},
    {"humidity", {0}, 0},
    {"air_quality", {0}, 0},
};

pthread_mutex_t topic_lock = PTHREAD_MUTEX_INITIALIZER;

void subscribe_client(const char *type, int client_sock) {
    pthread_mutex_lock(&topic_lock);
    for (int i = 0; i < MAX_TOPICS; ++i) {
        if (strcmp(topics[i].type, type) == 0) {
            topics[i].client_socks[topics[i].subscriber_count] = client_sock;
            topics[i].subscriber_count++;
            break;
        }
    }
    pthread_mutex_unlock(&topic_lock);
}

void remove_client_and_notify(const char *type, int client_sock, int x, int y) {
    sensor_message disconnect_msg;
    strncpy(disconnect_msg.type, type, sizeof(disconnect_msg.type));
    disconnect_msg.coords[0] = x;
    disconnect_msg.coords[1] = y;
    disconnect_msg.measurement = -1.0000;

    pthread_mutex_lock(&topic_lock);
    for (int i = 0; i < MAX_TOPICS; ++i) {
        if (strcmp(topics[i].type, type) == 0) {
            // Remover o cliente do array
            for (int j = 0; j < topics[i].subscriber_count; ++j) {
                if (topics[i].client_socks[j] == client_sock) {
                    // Log do sensor desconectado
                    printf("log:\n%s sensor in (%d,%d)\nmeasurement: %.4f\n\n", 
                           disconnect_msg.type, x, y, disconnect_msg.measurement);

                    // Notificar outros clientes
                    for (int k = 0; k < topics[i].subscriber_count; ++k) {
                        if (topics[i].client_socks[k] != client_sock) {
                            if (send(topics[i].client_socks[k], &disconnect_msg, sizeof(disconnect_msg), 0) < 0) {
                                printf("Erro ao notificar cliente %d sobre desconexão.\n", topics[i].client_socks[k]);
                            }
                        }
                    }

                    // Remover cliente desconectado do array
                    for (int k = j; k < topics[i].subscriber_count - 1; ++k) {
                        topics[i].client_socks[k] = topics[i].client_socks[k + 1];
                    }
                    topics[i].subscriber_count--;
                    break;
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&topic_lock);
}

void publish_message(const char *type, sensor_message *msg) {
    pthread_mutex_lock(&topic_lock);
    for (int i = 0; i < MAX_TOPICS; ++i) {
        if (strcmp(topics[i].type, type) == 0) {
            for (int j = 0; j < topics[i].subscriber_count; ++j) {
                int client_sock = topics[i].client_socks[j];
                if (send(client_sock, msg, sizeof(*msg), 0) < 0) {
                    printf("Erro ao enviar mensagem para o cliente %d. Removendo cliente.\n", client_sock);
                    // Remover o cliente e notificar outros
                    remove_client_and_notify(type, client_sock, msg->coords[0], msg->coords[1]);
                    j--; // Ajustar índice após a remoção
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&topic_lock);
}

/*
    Avalia se o tipo é válido
*/
int is_valid_type(const char *type) {
    return strcmp(type, "temperature") == 0 || 
           strcmp(type, "humidity") == 0 || 
           strcmp(type, "air_quality") == 0;
}

/*
    Essa função é chamada para lidar com cada thread criada para cada cliente conectado
*/
void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    sensor_message msg;

    // Receber a primeira mensagem para validar tipo e inscrever no tópico
    if (recv(client_sock, &msg, sizeof(msg), 0) > 0) {
        if (!is_valid_type(msg.type)) {
            printf("Tipo inválido: %s. Conexão encerrada.\n", msg.type);
            close(client_sock);
            return NULL;
        }

        // Inscreve o cliente no tópico
        subscribe_client(msg.type, client_sock);
    }

    // Continua ouvindo as mensagens subsequentes
    while (recv(client_sock, &msg, sizeof(msg), 0) > 0) {
        if (msg.measurement == -1.0000) {
            printf("Sensor in (%d,%d) saiu da rede.\n", msg.coords[0], msg.coords[1]);
            break;
        }
        
        // Exibe a mensagem enviado pelo sensor no terminal
        printf("log:\n%s sensor in (%d,%d)\nmeasurement: %.4f\n\n", 
                           msg.type, msg.coords[0], msg.coords[1], msg.measurement);

        publish_message(msg.type, &msg);
    }

    // Remover cliente desconectado e notificar outros
    remove_client_and_notify(msg.type, client_sock, msg.coords[0], msg.coords[1]);
    close(client_sock);
    return NULL;
}

/*
    Função para iniciar o servidor
*/
void start_server(const char *ip_version, int port) {
    int server_sock, *new_sock;
    struct sockaddr_storage server_addr;
    socklen_t addr_len;

    // Escolha do protocolo com base no argumento ip_version
    if (strcmp(ip_version, "v4") == 0) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&server_addr;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = htons(port);
        addr_len = sizeof(struct sockaddr_in);

        if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Falha ao criar socket IPv4");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(ip_version, "v6") == 0) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&server_addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = htons(port);
        addr_len = sizeof(struct sockaddr_in6);

        if ((server_sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
            perror("Falha ao criar socket IPv6");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Versão de IP inválida. Use 'v4' ou 'v6'.\n");
        exit(EXIT_FAILURE);
    }

    // Fazer bind
    if (bind(server_sock, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Falha ao fazer bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Escutar por conexões
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Falha ao escutar");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Loop para aceitar conexões
    while (1) {
        new_sock = malloc(sizeof(int));
        if ((*new_sock = accept(server_sock, NULL, NULL)) < 0) {
            perror("Falha ao aceitar conexão");
            free(new_sock);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, new_sock);
        pthread_detach(thread_id);
    }

    close(server_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <v4|v6> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_version = argv[1];
    int port = atoi(argv[2]);

    start_server(ip_version, port);

    return 0;
}