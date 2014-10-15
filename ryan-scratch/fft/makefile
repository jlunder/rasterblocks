OUTFILE = fftwav
OUTDIR = $(CURDIR)

#CROSS_COMPILE = arm-linux-gnueabi-

CC_C = $(CROSS_COMPILE)gcc

CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror
#CFLAGS = -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror 
PFLAGS = -lpthread -lm -lsndfile -lasound -lncurses -lfftw3
all: buildTarget

buildTarget:
	$(CC_C) $(CFLAGS) fftwav.c -o $(OUTDIR)/$(OUTFILE) $(PFLAGS)
	ls -I $(OUTDIR)/$(OUTFILE)

clean:
	rm -rf $(OUTDIR)/$(OUTFILE)