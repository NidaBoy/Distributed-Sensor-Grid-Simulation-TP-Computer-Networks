CC = gcc
INCLUDE = include
CFLAGS = -Wall -pthread -I$(INCLUDE)
MATH_LIBRARY = -lm
BIN_DIR = bin

all: $(BIN_DIR)/client $(BIN_DIR)/server

$(BIN_DIR)/client: src/client.c src/sensor_utils.c src/message_handler.c src/neighbor_manager.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client src/client.c src/sensor_utils.c src/message_handler.c src/neighbor_manager.c $(MATH_LIBRARY)

$(BIN_DIR)/server: src/server.c src/sensor_utils.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server src/server.c src/sensor_utils.c $(MATH_LIBRARY)

clean:
	rm -f $(BIN_DIR)/client $(BIN_DIR)/server