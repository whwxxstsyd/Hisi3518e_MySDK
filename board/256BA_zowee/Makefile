##
## Makefile
##
## History:
##    2014/03/02 - [Li Wei] Create
##
## Copyright (C) 2011-2015, Tenvis, Inc.
##
##

.PHONY: all

all:

export SDK_TOPDIR:=$(shell pwd)/../..

include $(SDK_TOPDIR)/build/core/build_define.mk

# for hisi platform 位置不能随意改变
include $(PLATFORM_DIR)/platform.param

include $(SDK_TOPDIR)/build/core/definition.mk
include $(SDK_TOPDIR)/build/core/configure.mk
#include $(SDK_TOPDIR)/build/core/menuconfig.mk

#include $(SDK_TOPDIR)/build/core/build_bootloader.mk
#include $(SDK_TOPDIR)/build/core/build_kernel.mk
#include $(SDK_TOPDIR)/build/core/build_busybox.mk
#include $(SDK_TOPDIR)/build/core/build_rootfs.mk
#include $(SDK_TOPDIR)/build/core/build_image.mk

# Find all of Makefile
MAKEFILE_SDK := $(shell $(SDK_TOPDIR)/build/bin/findleaves.py --prune=.git \
		--prune=out --prune=build --prune=scripts \
		$(SDK_TOPDIR) Makefile.sdk)	
 
include $(MAKEFILE_SDK)

include $(SDK_TOPDIR)/build/core/dependency.mk

#	Make从左至右执行依赖关系
all: $(ALL_TARGETS)
	@echo "Build SDK Done"

.PHONY: clean distclean

clean:
	@rm -rf $(SDK_TOPDIR)/out

distclean: clean
	@rm -rf $(dir $(DOT_CONFIG))/.config* $(dir $(DOT_CONFIG))/config.h

