#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição da estrutura de mensagem do sensor
typedef struct {
    char type[12];        // Tipo do sensor ou comando de inscrição (ex.: "temperature", "subscribe:humidity")
    int coords[2];        // Coordenadas do sensor (linha, coluna)
    float measurement;    // Medição do sensor ou -1.0 para saída da rede
} sensor_message;

// Protótipos das funções
void start_server(const char *ip_version, int port);
void subscribe_client(const char *type, int client_sock);
void publish_message(const char *type, sensor_message *msg);

#endif