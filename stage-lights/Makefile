ifeq ($(OS),WIN32)
	CCFLAGS += -D WIN32
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		CCFLAGS += -D AMD64
	endif
	ifeq ($(PROCESSOR_ARCHITECTURE),x86)
		CCFLAGS += -D IA32
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OS = LINUX
		CCFLAGS += -D LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		OS = OSX
		CCFLAGS += -D OSX
	endif
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		CCFLAGS += -D AMD64
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
		CCFLAGS += -D IA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
		CCFLAGS += -D ARM
	endif
endif


ifeq ($(OS), LINUX)
	CC=gcc
	CC_TARGET=arm-linux-gnueabi-gcc
	CFLAGS_COMMON=-W -Wall -Werror
	CFLAGS=-O0 $(CFLAGS_COMMON)
	CFLAGS_TARGET=-O2 $(CFLAGS_COMMON)
	CPPFLAGS_COMMON=--std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L -D__USE_MISC
	CPPFLAGS=$(CPPFLAGS_COMMON) -DSTAGE_LIGHTS_LINUX -DGLUS_ES2 -DSTAGE_LIGHTS_USE_SDL2_GLES2_HARNESS
	CPPFLAGS_TARGET=$(CPPFLAGS_COMMON) -DSTAGE_LIGHTS_LINUX -DSTAGE_LIGHTS_USE_TARGET_HARNESS
	LDFLAGS=-g -lSDL2 -lm -lGLESv2 -lsndfile
	LDFLAGS_TARGET=-g -lm
endif
ifeq ($(OS), OSX)
	CC=gcc
	CC_TARGET=arm-linux-gnueabi-gcc
	CFLAGS_COMMON=-W -Wall -Werror --std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L
	CFLAGS=-O0 $(CFLAGS_COMMON)
	CFLAGS_TARGET=-O2 $(CFLAGS_COMMON)
	CPPFLAGS=-DSTAGE_LIGHTS_OSX -DGLUS_ES2 -DSTAGE_LIGHTS_USE_GLFW3_GLES2_HARNESS
	CPPFLAGS_TARGET=-DSTAGE_LIGHTS_LINUX -DSTAGE_LIGHTS_USE_TARGET_HARNESS
	LDFLAGS=-g -framework OpenGL -framework Cocoa -framework QuartzCore -framework IOKit -lm -lglfw3
	LDFLAGS_TARGET=-g -lm
endif
ifeq ($(OS), WIN32)
	CC=!
	CC_TARGET=!
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
endif


MAKEDEPEND=@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

MAKEDEPEND_TARGET=@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS_TARGET) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.target.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY: all cleanall clean

all: stage_lights

%.d: %.c Makefile
	@$(MAKEDEPEND)

%.target.d: %.c Makefile
	@$(MAKEDEPEND_TARGET)

STAGE_LIGHTS_SRC = $(wildcard *.c)

-include $(STAGE_LIGHTS_SRC:.c=.d)
-include $(STAGE_LIGHTS_SRC:.c=.target.d)

%.o: %.c Makefile
	@$(MAKEDEPEND)
	$(CC) -c -g $(CFLAGS) -o $@ $(CPPFLAGS) $<

%.target.o: %.c Makefile
	@$(MAKEDEPEND_TARGET)
	$(CC_TARGET) -c -g $(CFLAGS_TARGET) -o $@ $(CPPFLAGS_TARGET) $<

stage_lights: $(STAGE_LIGHTS_SRC:.c=.o)
	$(CC) -o $@ $^ $(LDFLAGS)

stage_lights.target: $(STAGE_LIGHTS_SRC:.c=.target.o)
	$(CC_TARGET) -o $@ $^ $(LDFLAGS_TARGET)

cleanall: clean

clean:
	for X in stage_lights *.o *.o.* *.d *.d.*; do \
		if [ -f $$X ]; then rm $$X; fi \
		done

