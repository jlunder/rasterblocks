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
	CC_TARGET_PREFIX=arm-linux-gnueabihf-
	CC_TARGET=$(CC_TARGET_PREFIX)gcc
	CFLAGS_COMMON=-W -Wall -Werror
	CFLAGS=-O0 $(CFLAGS_COMMON)
	CFLAGS_TARGET=-O3 -mtune=cortex-a8 -march=armv7-a $(CFLAGS_COMMON)
	CFLAGS_TARGET_DEBUG=-O0 -mtune=cortex-a8 -march=armv7-a $(CFLAGS_COMMON)
	CPPFLAGS_COMMON=--std=gnu99 -fgnu89-inline -D_POSIX_C_SOURCE=200809L -D__USE_MISC
	CPPFLAGS=$(CPPFLAGS_COMMON) -DRB_LINUX -DGLUS_ES2 -DRB_USE_SDL2_GLES2_HARNESS -DRB_USE_SNDFILE_INPUT -DRB_USE_ALSA_DEVICE -DRB_USE_PIXELPUSHER_OUTPUT -DRB_USE_OPENGL_OUTPUT
	CPPFLAGS_TARGET=$(CPPFLAGS_COMMON) -DRB_LINUX -DRB_USE_TARGET_HARNESS -DRB_USE_SNDFILE_INPUT -DRB_USE_ALSA_DEVICE -DRB_USE_BBB_UART4_MIDI_CONTROL_INPUT -DRB_USE_PIXELPUSHER_OUTPUT -DRB_USE_SPIDEV_OUTPUT -DRB_USE_PRUSS_IO -Icross-dep/include -Ipruss/include
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
	CC_TARGET_PREFIX :=
	CFLAGS := $(CFLAGS_TARGET)
	CPPFLAGS := $(CPPFLAGS_TARGET)
	LDFLAGS := $(LDFLAGS_TARGET)
	INSTALL_DIR := /usr/local/lib/rasterblocks
endif

RB_VERSION=1.2
DT_NAME=rb-pruss-io-00A0

MAKEDEPEND=@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,build/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

MAKEDEPEND_TARGET=@set -e; rm -f $@; \
	$(CC_TARGET) -M $(CPPFLAGS_TARGET) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,build/\1.target.o build/\1.target-debug.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


.PHONY: all cleanall clean clean-dtc

ifneq ($(PACKAGE_BUILD),)
all: bin/rasterblocks-$(RB_VERSION)_armhf.deb
else
all: bin/rasterblocks
endif

RASTERBLOCKS_SRC := $(basename $(notdir $(wildcard src/*.c)))

RASTERBLOCKS_O := $(addprefix build/,$(addsuffix .o, $(RASTERBLOCKS_SRC)))
RASTERBLOCKS_D := $(addprefix build/,$(addsuffix .d, $(RASTERBLOCKS_SRC)))

src/icons.h: $(wildcard icons/[0-9][0-9]*)
	python make-icons.py $(wildcard icons/[0-9][0-9]*)

build/%.d: src/%.c src/icons.h
	@mkdir -p build
	@$(MAKEDEPEND)

ifeq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(RASTERBLOCKS_D)
endif
endif
endif

build/%.o: src/%.c
	@mkdir -p build
	@$(MAKEDEPEND)
	$(CC) -c -g $(CFLAGS) -o $@ $(CPPFLAGS) $<

ifeq ($(CROSS_BUILD),)
# Don't build these tools with the cross-compiler!

# We build pasm ourselves instead of using the shipped makefile.
bin/pasm: $(wildcard pasm/*.{c,h})
	@mkdir -p bin
	$(CC) -g -Wall -D_UNIX_ -o $@ $(wildcard pasm/*.c) $(LDFLAGS)

# This is super hacky. Instead of using the shipped makefiles for dtc, we
# build it ourselves using this makefile. Fortunately the build is very
# simple.
# Things to note:
# - Instead of using lex/yacc, we add a target to just copy the shipped
#   preprocessed files (the dtc/% target)
# - There are 4 binaries in this source: dtc, fdtdump and a couple others. The
#   $(patsubst dtc/fdt%.c,,... strips those extras out of the list of source!
# - There's an extra clean-dtc target to remove the lex/yacc .c/.h files
dtc/%: dtc/%_shipped
	cp $< $@
bin/dtc: $(wildcard dtc/*.c dtc/*.h) $(patsubst %_shipped,%,$(wildcard dtc/*_shipped)) $(wildcard dtc/libfdt/*.c dtc/libfdt/*.h)
	@mkdir -p bin
	$(CC) -g -Wall -o $@ -I dtc/libfdt $(patsubst dtc/fdt%.c,,$(patsubst %.h,,$(sort $^)))
clean-dtc:
	rm -f $(patsubst %_shipped,%,$(wildcard dtc/*.c_shipped dtc/*.h_shipped))

endif

bin/rasterblocks: $(RASTERBLOCKS_O)
	@mkdir -p bin
	$(CC) -o $@ $(RASTERBLOCKS_O) $(LDFLAGS)

ifneq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)

# Either we're building on target or we're in the cross-build env -- enable
# the cross-build target directly.
RASTERBLOCKS_O_TARGET := $(addprefix build/,$(addsuffix .target.o, $(RASTERBLOCKS_SRC)))
RASTERBLOCKS_O_TARGET_DEBUG := $(addprefix build/,$(addsuffix .target-debug.o, $(RASTERBLOCKS_SRC)))
RASTERBLOCKS_D_TARGET := $(addprefix build/,$(addsuffix .target.d, $(RASTERBLOCKS_SRC)))


build/%.target.d: src/%.c src/icons.h src/pruss_io_pru0_bin.h src/pruss_io_pru1_bin.h
	@mkdir -p build
	@$(MAKEDEPEND_TARGET)

ifneq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(RASTERBLOCKS_D_TARGET)
endif
endif
endif

src/pruss_io_pru0_bin.h: src/pruss_io_pru0.p src/pruss_io.hp bin/pasm
	bin/pasm -c -CrbPrussIoPru0Code src/pruss_io_pru0.p src/pruss_io_pru0

src/pruss_io_pru1_bin.h: src/pruss_io_pru1.p src/pruss_io.hp bin/pasm
	bin/pasm -c -CrbPrussIoPru1Code src/pruss_io_pru1.p src/pruss_io_pru1

build/%.target.o: src/%.c
	@mkdir -p build
	@$(MAKEDEPEND_TARGET)
	$(CC_TARGET) -c -g $(CFLAGS_TARGET) -o $@ $(CPPFLAGS_TARGET) $<

build/%.target-debug.o: src/%.c
	@mkdir -p build
	@$(MAKEDEPEND_TARGET)
	$(CC_TARGET) -c -g $(CFLAGS_TARGET_DEBUG) -o $@ $(CPPFLAGS_TARGET) $<

bin/rasterblocks.target: $(RASTERBLOCKS_O_TARGET) pruss/lib/libprussdrv.a bin/pasm
	@mkdir -p bin
	$(CC_TARGET) -o $@ $(RASTERBLOCKS_O_TARGET) pruss/lib/libprussdrv.a $(LDFLAGS_TARGET)

bin/rasterblocks.target-debug: $(RASTERBLOCKS_O_TARGET_DEBUG) pruss/lib/libprussdrvd.a bin/pasm
	@mkdir -p bin
	$(CC_TARGET) -o $@ $(RASTERBLOCKS_O_TARGET_DEBUG) pruss/lib/libprussdrv.a $(LDFLAGS_TARGET)

pruss/lib/libprussdrv.a: $(wildcard pruss/src/*.{c,h}) pruss/src/Makefile
	CROSS_COMPILE=$(CC_TARGET_PREFIX) make -C pruss/src ../lib/libprussdrv.a

pruss/lib/libprussdrvd.a: $(wildcard pruss/src/*.{c,h}) pruss/src/Makefile
	CROSS_COMPILE=$(CC_TARGET_PREFIX) make -C pruss/src ../lib/libprussdrvd.a

bin/$(DT_NAME).dtbo: $(DT_NAME).dts bin/dtc
	bin/dtc -I dts -O dtb -o $@ -b 0 -@ $<

else

# We're in the host build env, use the cross-build script to perform the
# target build.
ifneq ($(TARGET_BUILD),)
bin/rasterblocks.target bin/$(DT_NAME).dtbo: precise-chroot bin/pasm bin/dtc
	./target-build
endif

endif

ifneq ($(PACKAGE_BUILD),)

bin/package-rasterblocks: $(wildcard package-rasterblocks/DEBIAN/*) rasterblocks.service bin/$(DT_NAME).dtbo
	rm -rf bin/package-rasterblocks
	cp -r package-rasterblocks bin/package-rasterblocks
	sed "s/%VERSION%/$(RB_VERSION)/g" < package-rasterblocks/DEBIAN/control > bin/package-rasterblocks/DEBIAN/control
	mkdir -p bin/package-rasterblocks/usr/lib/rasterblocks
	mkdir -p bin/package-rasterblocks/var/lib/rasterblocks
	mkdir -p bin/package-rasterblocks/lib/firmware
	cp rasterblocks.service bin/package-rasterblocks/usr/lib/rasterblocks
	cp bin/$(DT_NAME).dtbo bin/package-rasterblocks/lib/firmware

bin/package-rasterblocks-nodeserver: $(wildcard package-rasterblocks-nodeserver/DEBIAN/*) $(wildcard nodeserver/*) rasterblocks-nodeserver.service
	rm -rf bin/package-rasterblocks-nodeserver
	cp -r package-rasterblocks-nodeserver bin/package-rasterblocks-nodeserver
	sed "s/%VERSION%/$(RB_VERSION)/g" < package-rasterblocks-nodeserver/DEBIAN/control > bin/package-rasterblocks-nodeserver/DEBIAN/control
	mkdir -p bin/package-rasterblocks-nodeserver/usr/lib/rasterblocks
	cp -r nodeserver bin/package-rasterblocks-nodeserver/usr/lib/rasterblocks/
	cp rasterblocks-nodeserver.service bin/package-rasterblocks-nodeserver/usr/lib/rasterblocks

ifeq ($(CROSS_BUILD)$(BUILD_ON_TARGET),)
# Neither the cross-build env nor the target has tools to build a host package
bin/rasterblocks-$(RB_VERSION)_x64.deb: bin/package-rasterblocks bin/rasterblocks
	cp bin/rasterblocks bin/package-rasterblocks/usr/lib/rasterblocks/rasterblocks
	cp config/host_config.json bin/package-rasterblocks/usr/lib/rasterblocks/config.json
	dpkg-deb --build bin/package-rasterblocks $@
endif

ifeq ($(CROSS_BUILD),)
# The cross-build env doesn't have enough toolchain to build a package
bin/rasterblocks-$(RB_VERSION)_armhf.deb: bin/package-rasterblocks bin/rasterblocks.target
	cp bin/rasterblocks.target bin/package-rasterblocks/usr/lib/rasterblocks/rasterblocks
	cp config/target_config.json bin/package-rasterblocks/usr/lib/rasterblocks/config.json
	dpkg-deb --build bin/package-rasterblocks $@

bin/rasterblocks-nodeserver-$(RB_VERSION)_all.deb: bin/package-rasterblocks-nodeserver
	dpkg-deb --build bin/package-rasterblocks-nodeserver $@
endif

endif

cleanall: clean
	-sudo umount precise-chroot/home/rasterblocks
	sudo rm -rf precise-chroot

clean: clean-dtc
	@rm -rf build/* bin/*
	@rm -f src/icons.h

