ifeq ($(OS),WIN32)
else
	UNAME_S := $(shell uname -s)
	UNAME_M := $(shell uname -m)
	
	ifeq ($(UNAME_S),Linux)
		OS = LINUX
		ifeq ($(UNAME_M),armv7l)
			TARGET_BUILD = TRUE
		else
			TARGET_BUILD =
		endif
	endif
	ifeq ($(UNAME_S),Darwin)
		OS = OSX
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
	CPPFLAGS_TARGET=$(CPPFLAGS_COMMON) -DSTAGE_LIGHTS_LINUX -DSTAGE_LIGHTS_USE_TARGET_HARNESS -Icross-dep/include
	LDFLAGS=-g -lSDL2 -lm -lGLESv2 -lsndfile -lasound -lfftw3f -ljson
	LDFLAGS_TARGET=-g -lm -Lcross-dep/lib -Wl,-rpath=cross-dep/lib -lsndfile -lasound -lfftw3f
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

ifneq ($(TARGET_BUILD),)
	CFLAGS := $(CFLAGS_TARGET)
	CPPFLAGS := $(CPPFLAGS_TARGET)
	LDFLAGS := $(LDFLAGS_TARGET)
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

build/%.d: src/%.c Makefile
	@$(MAKEDEPEND)

build/%.target.d: src/%.c Makefile
	@$(MAKEDEPEND_TARGET)

STAGE_LIGHTS_SRC := $(basename $(notdir $(wildcard src/*.c)))
STAGE_LIGHTS_O := $(addprefix build/,$(addsuffix .o, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_O_TARGET := $(addprefix build/,$(addsuffix .target.o, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_D := $(addprefix build/,$(addsuffix .d, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_D_TARGET := $(addprefix build/,$(addsuffix .target.d, $(STAGE_LIGHTS_SRC)))

-include $(STAGE_LIGHTS_D)
-include $(STAGE_LIGHTS_D_TARGET)

build/%.o: src/%.c Makefile
	@mkdir -p build
	@$(MAKEDEPEND)
	$(CC) -c -g $(CFLAGS) -o $@ $(CPPFLAGS) $<

build/%.target.o: src/%.c Makefile
	@mkdir -p build
	@$(MAKEDEPEND_TARGET)
	$(CC_TARGET) -c -g $(CFLAGS_TARGET) -o $@ $(CPPFLAGS_TARGET) $<

stage_lights: $(STAGE_LIGHTS_O)
	@mkdir -p bin
	$(CC) -o bin/$@ $^ $(LDFLAGS)

stage_lights.target: $(STAGE_LIGHTS_O_TARGET)
	$(CC_TARGET) -o $@ $^ $(LDFLAGS_TARGET)

cleanall: clean

clean:
	@rm -f build/* bin/*
