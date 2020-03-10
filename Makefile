CC=gcc
MD=mkdir
CFLAGS=-c -Wall -I./include
PLATFORM=wiringpi
#PLATFORM=bcm2835
LDFLAGS=-ltelebot -pthread
SOURCES=\
	src/core.c \
	src/driver.c \
	src/device.c \
	src/bot.c


ifeq ($(PLATFORM), wiringpi)
	SOURCES+=src/wiringpi.c
	LDFLAGS+=-lwiringPi
else
	SOURCES+=src/bcm2835.c
	STATIC_LIB=./thirdparty/bcm2835/lib/libbcm2835.a
	CFLAGS+=-I./thirdparty/bcm2835/include
endif

BUILD_DIR=./build
SRC_DIR=src/

CREATE_DIRS=$(BUILD_DIR)/$(SRC_BOT_DIR)

OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=terrarium-bot

all: $(CREATE_DIRS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(addprefix $(BUILD_DIR)/, $(OBJECTS)) $(STATIC_LIB) $(LDFLAGS) -o $(BUILD_DIR)/$@

.c.o: $(CREATE_DIR)
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@

$(CREATE_DIRS):
	$(MD) -p $(BUILD_DIR)/$(SRC_DIR)
	cp ./service/.token $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
