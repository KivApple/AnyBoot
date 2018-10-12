include ../commons.mk

all: bin2hex

bin2hex: bin2hex.c Makefile
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $<

clean:
	$(RM_RFV) bin2hex

.PHONY: all clean
