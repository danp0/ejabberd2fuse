
MKDIR := mkdir
SHELL := /bin/bash
RM    := rm -rf

all: ./build/Makefile
	@ $(MAKE) -C build

./build/Makefile: ./build
	@ (cd build > /dev/null 2>&1 && cmake ..)

./build:
	@ $(MKDIR) build

distclean:
	@- (cd build > /dev/null 2>&1 && cmake .. > /dev/null 2>&1)
	@- $(MAKE) --silent -C build clean || true
	@- $(RM) ./build/Makefile
	@- $(RM) ./build/CMake*
	@- $(RM) ./build/cmake.*
	@- $(RM) ./build/*.cmake
	@- $(RM) ./build/bin


ifeq ($(filter distclean,$(MAKECMDGOALS)),)

        $(MAKECMDGOALS): ./build/Makefile
	                @ $(MAKE) -C build $(MAKECMDGOALS)

endif
