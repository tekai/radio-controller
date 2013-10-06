CC=gcc
CFLAGS=-Wall -l mpdclient

all:
	$(CC) $(CFLAGS) -o radio-controller mouse-radio-controller.c

test:
	$(CC) $(CFLAGS) -o usb-test test.c
