#
# create by liwei
#MODIFY BY ZHANG

DOT_CONFIG	:= $(BOARD_DIR)/.config
export DOT_CONFIG

ifeq ($(wildcard $(DOT_CONFIG)), $(DOT_CONFIG))
include $(DOT_CONFIG)
endif

#SDK_TOPDIR		:= $(shell cd $(SDK_TOPDIR) && pwd)
#export SDK_TOPDIR

########################### BUILD COMMAND ####################
CLEAR_VARS		:= $(SDK_TOPDIR)/build/core/clear_vars.mk
BUILD_APP			:= $(SDK_TOPDIR)/build/core/build_app.mk
#BUILD_DRIVER	:= $(SDK_TOPDIR)/build/core/build_driver.mk
#BUILD_PLATFORM_DRIVER	:= $(SDK_TOPDIR)/build/core/build_platform_driver.mk
#BUILD_PREBUILD		:= $(SDK_TOPDIR)/build/core/build_prebuild.mk
#BUILD_BACKUP_PREBUILD := $(SDK_TOPDIR)/build/core/build_backup_prebuild.mk

########################### BUILD IMAGE TOOLS ######################
#CPU_BIT_WIDTH		:= $(shell uname -m)

#ifeq ($(CPU_BIT_WIDTH), x86_64)
#MAKEDEVS		:= $(SDK_TOPDIR)/rootfs/bin/makedevs-64
#MKUBIFS			:= $(SDK_TOPDIR)/rootfs/bin/mkfs.ubifs-64
#UBINIZE			:= $(SDK_TOPDIR)/rootfs/bin/ubinize-64
#else
#MAKEDEVS		:= $(SDK_TOPDIR)/rootfs/bin/makedevs-32
#MKUBIFS			:= $(SDK_TOPDIR)/rootfs/bin/mkfs.ubifs-32
#UBINIZE			:= $(SDK_TOPDIR)/rootfs/bin/ubinize-32
#endif
#MKYAFFS2		:= $(SDK_TOPDIR)/rootfs/bin/mkfs.yaffs2
#MKSQUASHFS		:= $(SDK_TOPDIR)/rootfs/bin/mkfs.squashfs

############################ GLOBAL BUILD FLAGS ###########################
ifeq ($(DOT_CONFIG), $(wildcard $(DOT_CONFIG)))
CHIP		:= $(shell grep ^CONFIG_ARCH $(DOT_CONFIG) | \
				sed -e s/^CONFIG_ARCH_// | \
				sed -e s/=y// | \
				tr [:upper:] [:lower:])
export CHIP
endif

SDK_CFLAGS	:= -I$(BOARD_DIR)
							 
SDK_CFLAGS  += -include sdk_config.h
SDK_CFLAGS	+= -Wno-unused-variable -Wno-unused-function
# for [-Werror=unused-but-set-variable]
SDK_CFLAGS	+= -Wno-unused-but-set-variable
PLATFORM_CFLAGS := -I$(SDK_TOPDIR)/include/bsp/ $(HI_CFLAGS)
SDK_CFLAGS 	+= $(PLATFORM_CFLAGS)
							 
SDK_AFLAGS	:=
SDK_LDFLASG	:= -lstdc++


############################ APP BUILD FLAGS #######################
SDK_APP_AFLAGS := $(SDK_AFLAGS)
# for debug
SDK_APP_CFLAGS := $(SDK_CFLAGS) -O3 -Wall -fPIC -D_REENTRENT -D_GNU_SOURCE -D__LINUX__
ifeq ($(WITH_GDB_INFO), y)
	SDK_APP_CFLAGS += -g
endif
ifeq ($(MEMORY_WATCH), y)
	SDK_APP_CFLAGS += -DMEMWATCH
	SDK_APP_CFLAGS += -include mem_watch.h
endif	
SDK_APP_CFLAGS += $(call cc-option, -mno-unaligned-access,)
SDK_APP_CFLAGS += -I$(SDK_TOPDIR)/include/apps -I$(SDK_TOPDIR)/include/bsp/private
SDK_APP_LDFLAGS :=

############################ DRIVER BUILD FLAGS #####################
export SDK_DRV_AFLAGS := $(SDK_AFLAGS)
export SDK_DRV_CFLAGS := $(SDK_CFLAGS)
export SDK_DRV_LDFLAGS := $(SDK_LDFLAGS)		

KERNEL_DEFCONFIG	:= $(shell echo $(CONFIG_KERNEL_DEFCONFIG_STRING))
KERNEL_INSTALL_PATH	:= $(SDK_BOARD_OUT)/rootfs/target
FIRMWARE_INSTALL_PATH	:= $(KERNEL_INSTALL_PATH)/lib/firmware
LINUX_INSTALL_FLAG := INSTALL_MOD_PATH=$(SDK_BOARD_OUT)/rootfs/target 
