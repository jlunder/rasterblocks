ifeq ($(OS),WIN32)
else
	UNAME_S := $(shell uname -s)
	UNAME_M := $(shell uname -m)
	
	ifeq ($(UNAME_S),Linux)
		OS = LINUX
		PACKAGE_BUILD = TRUE
		ifeq ($(UNAME_M),armv7l)
			TARGET_BUILD = TRUE
		endif
	endif
	ifeq ($(UNAME_S),Darwin)
		OS = OSX
	endif
	
endif


ifeq ($(OS), LINUX)
	CC=gcc
	CC_TARGET=arm-linux-gnueabihf-gcc
	CFLAGS_COMMON=-W -Wall -Werror
	CFLAGS=-O0 $(CFLAGS_COMMON)
	CFLAGS_TARGET=-O0 $(CFLAGS_COMMON)
	CPPFLAGS_COMMON=--std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L -D__USE_MISC
	CPPFLAGS=$(CPPFLAGS_COMMON) -DSL_LINUX -DGLUS_ES2 -DSL_USE_SDL2_GLES2_HARNESS -DSL_SNDFILE_SUPPORTED
	CPPFLAGS_TARGET=$(CPPFLAGS_COMMON) -DSL_LINUX -DSL_USE_TARGET_HARNESS -Icross-dep/include -DSL_SNDFILE_SUPPORTED
	LDFLAGS=-g -lSDL2 -lm -lGLESv2 -lsndfile -lasound -ljson-c
	LDFLAGS_TARGET=-g -lm -Lcross-dep/lib -Wl,-rpath=cross-dep/lib -Wl,-rpath-link=cross-dep/lib -lsndfile -lasound -ljson -lrt
endif
ifeq ($(OS), OSX)
	CC=gcc
	CC_TARGET=arm-linux-gnueabi-gcc
	CFLAGS_COMMON=-W -Wall -Werror --std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L
	CFLAGS=-O0 $(CFLAGS_COMMON)
	CFLAGS_TARGET=-O2 $(CFLAGS_COMMON)
	CPPFLAGS=-DSL_OSX -DGLUS_ES2 -DSL_USE_GLFW3_GLES2_HARNESS
	CPPFLAGS_TARGET=-DSL_LINUX -DSL_USE_TARGET_HARNESS
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
	CC_TARGET := $(CC)
	CFLAGS := $(CFLAGS_TARGET)
	CPPFLAGS := $(CPPFLAGS_TARGET)
	LDFLAGS := $(LDFLAGS_TARGET)
	INSTALL_DIR := /usr/local/lib/stage-lights
endif

MAKEDEPEND=@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

MAKEDEPEND_TARGET=@set -e; rm -f $@; \
	$(CC_TARGET) -M $(CPPFLAGS_TARGET) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.target.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


.PHONY: all cleanall clean

all: bin/stage-lights-1.0_armhf.deb

STAGE_LIGHTS_SRC := $(basename $(notdir $(wildcard src/*.c)))

STAGE_LIGHTS_O := $(addprefix build/,$(addsuffix .o, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_D := $(addprefix build/,$(addsuffix .d, $(STAGE_LIGHTS_SRC)))

build/%.d: src/%.c Makefile
	@$(MAKEDEPEND)

ifeq ($(MAKECMDGOALS),stage-lights)
	-include $(STAGE_LIGHTS_D)
endif

build/%.o: src/%.c Makefile
	@mkdir -p build
	@$(MAKEDEPEND)
	$(CC) -c -g $(CFLAGS) -o $@ $(CPPFLAGS) $<

bin/stage-lights: $(STAGE_LIGHTS_O)
	@mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(CROSS_BUILD)$(TARGET_BUILD),)

# Either we're building on target or we're in the cross-build env -- enable
# the cross-build target directly.
STAGE_LIGHTS_O_TARGET := $(addprefix build/,$(addsuffix .target.o, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_D_TARGET := $(addprefix build/,$(addsuffix .target.d, $(STAGE_LIGHTS_SRC)))

build/%.target.d: src/%.c Makefile
	@$(MAKEDEPEND_TARGET)

ifeq ($(MAKECMDGOALS),stage-lights.target)
	-include $(STAGE_LIGHTS_D_TARGET)
endif

build/%.target.o: src/%.c Makefile
	@mkdir -p build
	@$(MAKEDEPEND_TARGET)
	$(CC_TARGET) -c -g $(CFLAGS_TARGET) -o $@ $(CPPFLAGS_TARGET) $<

bin/stage-lights.target: $(STAGE_LIGHTS_O_TARGET)
	$(CC_TARGET) -o $@ $^ $(LDFLAGS_TARGET)

else

# We're in the host build env, use the cross-build script to perform the
# target build.
bin/stage-lights.target: Makefile precise-chroot
	./target-build

endif

ifneq ($(PACKAGE_BUILD),)
bin/stage-lights-1.0_armhf.deb: $(wildcard package/DEBIAN/control/*) $(wildcard nodeserver/*) stage-lights.service stage-lights-nodeserver.service bin/stage-lights.target
	mkdir -p package/usr/lib/stage-lights
	mkdir -p package/var/lib/stage-lights
	cp -r nodeserver package/usr/lib/stage-lights/
	rm -r package/usr/lib/stage-lights/nodeserver/node_modules
	cp bin/stage-lights.target package/usr/lib/stage-lights/stage-lights
	cp stage-lights.service package/usr/lib/stage-lights
	cp stage-lights-nodeserver.service package/usr/lib/stage-lights
	dpkg-deb --build package $@
endif

cleanall: clean
	sudo umount precise-chroot/home/stage-lights
	sudo rm -rf precise-chroot
	rm -rf package/usr
	rm -rf package/var

clean:
	@rm -f build/* bin/*

