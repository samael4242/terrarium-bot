CC=gcc
MD=mkdir
CFLAGS=-c -Wall -I./include -DDHT21
PLATFORM=wiringpi
#PLATFORM=bcm2835
LDFLAGS=-ltelebot -pthread -lrt
SOURCES=\
	src/core.c \
	src/driver.c \
	src/device.c \
	src/bot.c \
	src/timer.c \
	src/timerHandler.c


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
	cp ./service/terrarium-bot.service $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

install:
	install -d /etc/terrarium-bot
	#In case sservice is already running
	systemctl stop terrarium-bot.service
	systemctl disable terrarium-bot.service
	cp $(BUILD_DIR)/.token /etc/terrarium-bot/token
	cp $(BUILD_DIR)/terrarium-bot.service /etc/systemd/system/terrarium-bot.service
	cp $(BUILD_DIR)/terrarium-bot /usr/bin/terrarium-bot
	systemctl enable terrarium-bot.service
	systemctl start terrarium-bot.service
