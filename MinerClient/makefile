#create on 2018-05-21 by wjun

CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wextra -Wall -std=gnu99 -Iinclude -Iinclude/curl -Wno-unused-parameter -Wunused-variable

MQTT_C_SOURCES = src/mqtt.c src/mqtt_pal.c
MQTT_C_BIN = bin/mqtt_client 
BINDIR = bin
MQTT_LIB = -lpthread
MQTT_LIB += $(wildcard lib/*.a)

all: $(BINDIR) $(MQTT_C_BIN)

bin/mqtt_client: src/mqtt_client.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) $^ -o $@ $(MQTT_LIB)
	cp minerclient.sh bin/
	cp mqtt_client_cfgset.sh bin/
	arm-linux-gnueabihf-strip bin/mqtt_client
	mv bin/mqtt_client bin/minerclient

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR)
