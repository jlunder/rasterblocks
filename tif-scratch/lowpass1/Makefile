#makefile
CC_C = gcc
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror

all: two

two:
	$(CC_C) $(CGLAGS) demofilt2.c -o demofilt2 -lsndfile -lm

	#ls -l demofilt
	@echo "Done building"

clean:
	#rm $(OUTFILE)
	rm demofilt
