include ../commons.mk

all: bin2hex

bin2hex: bin2hex.c
	$(HOST_CC) $(HOST_CFLAGS) -o bin2hex bin2hex.c

clean:
	$(RM_RFV) bin2hex

.PHONY: all clean
