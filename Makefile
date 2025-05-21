CC = gcc
CFLAGS = -Wall -fPIC -I./include
LDFLAGS = -shared

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

SRCS_LED = $(SRC_DIR)/led.c
SRCS_BUZZER = $(SRC_DIR)/buzzer.c
SRCS_LIGHT = $(SRC_DIR)/light_sensor.c
SRCS_SEGMENT = $(SRC_DIR)/segment.c

OBJ_LED = $(BUILD_DIR)/led.o
OBJ_BUZZER = $(BUILD_DIR)/buzzer.o
OBJ_LIGHT = $(BUILD_DIR)/light_sensor.o
OBJ_SEGMENT = $(BUILD_DIR)/segment.o

TARGET_LED = $(BUILD_DIR)/libled.so
TARGET_BUZZER = $(BUILD_DIR)/libbuzzer.so
TARGET_LIGHT = $(BUILD_DIR)/liblight_sensor.so
TARGET_SEGMENT = $(BUILD_DIR)/libsegment.so

SERVER_OBJ = $(BUILD_DIR)/server.o
CLIENT_OBJ = $(BUILD_DIR)/client.o

SERVER_TARGET = $(BUILD_DIR)/server
CLIENT_TARGET = $(BUILD_DIR)/client

all: $(BUILD_DIR) $(TARGET_LED) $(TARGET_BUZZER) $(TARGET_LIGHT) $(TARGET_SEGMENT) $(SERVER_TARGET) $(CLIENT_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_LED): $(OBJ_LED)
	$(CC) $(LDFLAGS) -o $@ $^ -lwiringPi

$(TARGET_BUZZER): $(OBJ_BUZZER)
	$(CC) $(LDFLAGS) -o $@ $^ -lwiringPi

$(TARGET_LIGHT): $(OBJ_LIGHT)
	$(CC) $(LDFLAGS) -o $@ $^ -lwiringPi

$(TARGET_SEGMENT): $(OBJ_SEGMENT)
	$(CC) $(LDFLAGS) -o $@ $^ -lwiringPi

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/device_control.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_TARGET): $(SERVER_OBJ) $(TARGET_LED) $(TARGET_BUZZER) $(TARGET_LIGHT) $(TARGET_SEGMENT)
	$(CC) -o $@ $(SERVER_OBJ) -L$(BUILD_DIR) -lled -lbuzzer -llight_sensor -lsegment -lwiringPi -lpthread

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) -o $@ $^ -lpthread

$(SERVER_OBJ): $(SRC_DIR)/server.c $(INCLUDE_DIR)/device_control.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_OBJ): $(SRC_DIR)/client.c $(INCLUDE_DIR)/device_control.h
	$(CC) $(CFLAGS) -c $< -o $@

run_server:
	LD_LIBRARY_PATH=$(BUILD_DIR):$$LD_LIBRARY_PATH ./$(SERVER_TARGET)

run_client:
	./$(CLIENT_TARGET)

clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.so $(SERVER_TARGET) $(CLIENT_TARGET)