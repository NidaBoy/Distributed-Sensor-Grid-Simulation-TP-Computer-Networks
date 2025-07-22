#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição da estrutura de mensagem do sensor (mesma do servidor)
typedef struct {
    char type[12];        // Tipo do sensor (ex.: "temperature")
    int coords[2];        // Coordenadas do sensor (linha, coluna)
    float measurement;    // Medição gerada pelo sensor
} sensor_message;

// Protótipos das funções
void start_client(const char *server_ip, int port, const char *type, int x, int y);

// Função de exemplo para gerar medições (precisa ser implementada pelo usuário)
float generate_measurement(const char *type);

#endif