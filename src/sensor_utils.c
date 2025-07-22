#include "sensor_utils.h"


/**
 * Gera uma medição aleatória com base no tipo de sensor fornecido.
 * 
 * @param type O tipo do sensor (temperature, humidity, air_quality).
 * @return Um valor flutuante representando a medição gerada.
 */
float generate_measurement(const char *type) {
    if (strcmp(type, "temperature") == 0) {
        return 20.0 + (rand() % 2000) / 100.0; // 20.0 - 40.0
    } else if (strcmp(type, "humidity") == 0) {
        return 10.0 + (rand() % 8000) / 100.0; // 10.0 - 90.0
    } else if (strcmp(type, "air_quality") == 0) {
        return 15.0 + (rand() % 1500) / 100.0; // 15.0 - 30.0
    }
    return 0.0;
}

/**
 * Calcula a distância euclidiana entre dois pontos no plano.
 * 
 * @param x1 Coordenada X do primeiro ponto.
 * @param y1 Coordenada Y do primeiro ponto.
 * @param x2 Coordenada X do segundo ponto.
 * @param y2 Coordenada Y do segundo ponto.
 * @return Um valor flutuante representando a distância entre os dois pontos.
 */
float calculate_distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

void validate_arguments(int argc, char **argv) {
    // Verifica o número de argumentos
    if (argc != 8) { 
        fprintf(stderr, "Error: Invalid number of arguments\n");
        fprintf(stderr, "Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
        exit(EXIT_FAILURE);
    }

    // Verifica se o argumento '-type' está presente
    if (strcmp(argv[3], "-type") != 0) {
        fprintf(stderr, "Error: Expected '-type' argument\n");
        fprintf(stderr, "Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
        exit(EXIT_FAILURE);
    }

    // Verifica o tipo de sensor
    if (strcmp(argv[4], "temperature") != 0 && 
        strcmp(argv[4], "humidity") != 0 && 
        strcmp(argv[4], "air_quality") != 0) {
        fprintf(stderr, "Error: Invalid sensor type\n");
        fprintf(stderr, "Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[5], "-coords") != 0) {
        fprintf(stderr, "Error: Expected '-coords' argument\n");
        fprintf(stderr, "Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
        exit(EXIT_FAILURE);
    }

    int x = atoi(argv[6]);
    int y = atoi(argv[7]);
    if (x < 0 || x > 9 || y < 0 || y > 9) {
        fprintf(stderr, "Error: Coordinates must be in the range 0-9\n");
        fprintf(stderr, "Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
        exit(EXIT_FAILURE);
    }
}