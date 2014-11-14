#makefile
CC_C = gcc
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror
OUTFILE = demo

all: tcp

tcp:
	$(CC_C) $(CGLAGS) demo_tcpListen.c -o $(OUTFILE)

	ls -l demo
	@echo "Done building"

clean:
	rm $(OUTFILE)
