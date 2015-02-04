ifeq ($(OS),WIN32)
else
	UNAME_S := $(shell uname -s)
	
	ifeq ($(UNAME_S),Linux)
		OS = LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		OS = OSX
	endif
endif


ifeq ($(OS), LINUX)
	PACKAGE_BUILD = TRUE
	TARGET_BUILD = TRUE
	
	UNAME_M := $(shell uname -m)
	ifeq ($(UNAME_M),armv7l)
		BUILD_ON_TARGET = TRUE
	endif

	CC=gcc
	CC_TARGET=arm-linux-gnueabihf-gcc
	CFLAGS_COMMON=-W -Wall -Werror
	CFLAGS=-O0 $(CFLAGS_COMMON)
	CFLAGS_TARGET=-O2 $(CFLAGS_COMMON)
	CPPFLAGS_COMMON=--std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L -D__USE_MISC
	CPPFLAGS=$(CPPFLAGS_COMMON) -DRB_LINUX -DGLUS_ES2 -DRB_USE_SDL2_GLES2_HARNESS -DRB_USE_SNDFILE_INPUT -DRB_USE_ALSA_DEVICE -DRB_USE_PIXELPUSHER_OUTPUT -DRB_USE_OPENGL_OUTPUT
	CPPFLAGS_TARGET=$(CPPFLAGS_COMMON) -DRB_LINUX -DRB_USE_TARGET_HARNESS -DRB_USE_SNDFILE_INPUT -DRB_USE_ALSA_DEVICE -DRB_USE_PIXELPUSHER_OUTPUT -DRB_USE_SPIDEV_OUTPUT -Icross-dep/include
	LDFLAGS=-g -lSDL2 -lm -lGLESv2 -lsndfile -lasound -ljson-c
	LDFLAGS_TARGET=-g -lm -Lcross-dep/lib -Wl,-rpath=cross-dep/lib -Wl,-rpath-link=cross-dep/lib -lsndfile -lasound -ljson -lrt
endif
ifeq ($(OS), OSX)
	CC=gcc
	CFLAGS=-O0 -W -Wall -Werror --std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L
	CPPFLAGS=-DRB_OSX -DGLUS_ES2 -DRB_USE_GLFW3_GLES2_HARNESS -DRB_USE_SNDFILE_INPUT -DRB_USE_OPENAL_DEVICE -DRB_USE_PIXELPUSHER_OUTPUT -DRB_USE_OPENGL_OUTPUT -I/usr/local/include
	LDFLAGS=-g -framework OpenGL -framework Cocoa -framework QuartzCore -framework IOKit -framework OpenAL -L/usr/local/lib -lm -lsndfile -ljson-c -lglfw3
endif
ifeq ($(OS), WIN32)
	CC=!
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
endif

ifneq ($(BUILD_ON_TARGET),)
	CC_TARGET := $(CC)
	CFLAGS := $(CFLAGS_TARGET)
	CPPFLAGS := $(CPPFLAGS_TARGET)
	LDFLAGS := $(LDFLAGS_TARGET)
	INSTALL_DIR := /usr/local/lib/rasterblocks
endif

MAKEDEPEND=@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,build/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

MAKEDEPEND_TARGET=@set -e; rm -f $@; \
	$(CC_TARGET) -M $(CPPFLAGS_TARGET) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,build/\1.target.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


.PHONY: all cleanall clean

ifneq ($(PACKAGE_BUILD),)
all: bin/rasterblocks-1.0_armhf.deb
else
all: bin/rasterblocks
endif

STAGE_LIGHTS_SRC := $(basename $(notdir $(wildcard src/*.c)))

STAGE_LIGHTS_O := $(addprefix build/,$(addsuffix .o, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_D := $(addprefix build/,$(addsuffix .d, $(STAGE_LIGHTS_SRC)))

src/icons.h: $(wildcard icons/[0-9][0-9]*)
	python make-icons.py $(wildcard icons/[0-9][0-9]*)

bin:
	@mkdir -p bin

build:
	@mkdir -p build

build/%.d: src/%.c src/icons.h Makefile
	@mkdir -p build
	@$(MAKEDEPEND)

ifeq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(STAGE_LIGHTS_D)
endif
endif
endif

build/%.o: src/%.c build Makefile
	@$(MAKEDEPEND)
	$(CC) -c -g $(CFLAGS) -o $@ $(CPPFLAGS) $<

bin/pasm: $(wildcard pasm/*.c) bin Makefile
	$(CC) -g $(CFLAGS) -Wno-unused-parameter -Wno-sign-compare -D_UNIX_ -o $@ $(wildcard pasm/*.c) $(LDFLAGS)

bin/rasterblocks: $(STAGE_LIGHTS_O) bin Makefile
	$(CC) -o $@ $(STAGE_LIGHTS_O) $(LDFLAGS)

ifneq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)

# Either we're building on target or we're in the cross-build env -- enable
# the cross-build target directly.
STAGE_LIGHTS_O_TARGET := $(addprefix build/,$(addsuffix .target.o, $(STAGE_LIGHTS_SRC)))
STAGE_LIGHTS_D_TARGET := $(addprefix build/,$(addsuffix .target.d, $(STAGE_LIGHTS_SRC)))

build/%.target.d: src/%.c src/icons.h Makefile
	@mkdir -p build
	@$(MAKEDEPEND_TARGET)

ifneq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(STAGE_LIGHTS_D_TARGET)
endif
endif
endif

build/%.target.o: src/%.c build Makefile
	@$(MAKEDEPEND_TARGET)
	$(CC_TARGET) -c -g $(CFLAGS_TARGET) -o $@ $(CPPFLAGS_TARGET) $<

bin/rasterblocks.target: $(STAGE_LIGHTS_O_TARGET) bin/pasm bin Makefile
	$(CC_TARGET) -o $@ $(STAGE_LIGHTS_O_TARGET) $(LDFLAGS_TARGET)

else

# We're in the host build env, use the cross-build script to perform the
# target build.
ifneq ($(TARGET_BUILD),)
bin/rasterblocks.target: Makefile precise-chroot
	./target-build
endif

endif

ifneq ($(PACKAGE_BUILD),)

ifeq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)
# Neither the cross-build env nor the target has tools to build a host package
bin/rasterblocks-1.0_x64.deb: $(wildcard package/DEBIAN/*) $(wildcard nodeserver/*) rasterblocks.service rasterblocks-nodeserver.service bin/rasterblocks
	mkdir -p package/usr/lib/rasterblocks
	mkdir -p package/var/lib/rasterblocks
	cp -r nodeserver package/usr/lib/rasterblocks/
	rm -r package/usr/lib/rasterblocks/nodeserver/node_modules
	cp config/host_config.json package/usr/lib/rasterblocks/config.json
	cp bin/rasterblocks package/usr/lib/rasterblocks/rasterblocks
	cp rasterblocks.service package/usr/lib/rasterblocks
	cp rasterblocks-nodeserver.service package/usr/lib/rasterblocks
	dpkg-deb --build package $@
endif

ifeq ($(CROSS_BUILD),)
# The cross-build env doesn't have enough toolchain to build a package
bin/rasterblocks-1.0_armhf.deb: $(wildcard package/DEBIAN/*) $(wildcard nodeserver/*) rasterblocks.service rasterblocks-nodeserver.service bin/rasterblocks.target
	mkdir -p package/usr/lib/rasterblocks
	mkdir -p package/var/lib/rasterblocks
	cp -r nodeserver package/usr/lib/rasterblocks/
	rm -rf package/usr/lib/rasterblocks/nodeserver/node_modules
	cp config/target_config.json package/usr/lib/rasterblocks/config.json
	cp bin/rasterblocks.target package/usr/lib/rasterblocks/rasterblocks
	cp rasterblocks.service package/usr/lib/rasterblocks
	cp rasterblocks-nodeserver.service package/usr/lib/rasterblocks
	dpkg-deb --build package $@
endif

endif

cleanall: clean
	-sudo umount precise-chroot/home/rasterblocks
	sudo rm -rf precise-chroot
	rm -rf package/usr
	rm -rf package/var

clean:
	@rm -f build/* bin/*
	@rm -f src/icons.h

