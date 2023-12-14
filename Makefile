CC = gcc
CFLAGS = -Wall -w

SRC = src/
INCLUDE = include/
BIN = bin/

.PHONY: application
application: download

download: $(SRC)/download.c $(wildcard $(SRC)/*.c)
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE)

.PHONY: clean
clean:
	rm -rf download



