CC=gcc
MD=mkdir
CFLAGS=-c -Wall -I./include -I./thirdparty/bcm2835/include
LDFLAGS=-ltelebot
SOURCES=\
	./src/bot/main.c
STATIC_LIB=./thirdparty/bcm2835/lib/libbcm2835.a

BUILD_DIR=./build
SRC_BOT_DIR=/src/bot

CREATE_DIRS=$(BUILD_DIR)/$(SRC_BOT_DIR)

OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=terrarium-bot

all: $(CREATE_DIRS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(STATIC_LIB) $(BUILD_DIR)/$(OBJECTS) $(LDFLAGS) -o $(BUILD_DIR)/$@

.c.o: $(CREATE_DIR)
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@

$(CREATE_DIRS):
	$(MD) -p $(BUILD_DIR)/$(SRC_BOT_DIR)
	cp ./service/.token $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
