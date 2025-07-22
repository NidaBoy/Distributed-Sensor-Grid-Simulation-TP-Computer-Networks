#include "message_handler.h"

int receive_message(int sock, SensorMessage *msg) {
    int bytes_received;

    // Receber dados do socket
    bytes_received = recv(sock, msg, sizeof(SensorMessage), 0);

    if (bytes_received <= 0) {
        // Erro na recepção ou conexão fechada
        perror("Erro ao receber mensagem");
        return 0;
    }

    // Verificar se todos os bytes da mensagem foram recebidos
    if (bytes_received != sizeof(SensorMessage)) {
        fprintf(stderr, "Tamanho da mensagem inválido\n");
        return 0;
    }

    // Mensagem recebida com sucesso
    return 1;
}