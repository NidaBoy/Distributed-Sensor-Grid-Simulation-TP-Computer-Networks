#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "neighbor_manager.h"

// Estrutura de mensagem do sensor
typedef struct {
    char type[12];       // Tipo do sensor (temperature, humidity, air_quality)
    int coords[2];       // Coordenadas no grid (linha, coluna)
    float measurement;   // Valor da medição ou -1.0000 para remoção
} SensorMessage;

// Recebe uma mensagem de outro sensor
int receive_message(int sock, SensorMessage* message);

#endif