##
## create by liwei
## MODIFY BY ZHANG

-include .config

export ARCH=arm

export APP_CROSS_COMPILE:=arm-hisiv100nptl-linux
export CROSS_COMPILE:=$(APP_CROSS_COMPILE)-

export PLATFORM:=              #hisi
export MODEL:=$(shell echo $(CAMERA_MODEL))
export OEM:=$(shell echo $(OEM_INFO))
export BOARD_DIR:=$(shell pwd)

export BOARD:=$(MODEL)_$(OEM)
export PLATFORM_DIR			 :=$(SDK_TOPDIR)/bsp/$(PLATFORM)
export APP_INC_DIR			 :=$(SDK_TOPDIR)/include
export LINUX_SRC_DIR		 :=$(SDK_TOPDIR)/kernel
export PREBUILD_3RD_PARTY_DIR:=$(SDK_TOPDIR)/prebuild
export TOOLS_DIR			 :=$(SDK_TOPDIR)/tools

##########  for sdk out  ##########
export SDK_BOARD_OUT	:=$(SDK_TOPDIR)/out/$(BOARD)
export IMAGES_DIR		:=$(SDK_BOARD_OUT)/images
export LINUX_OUT_DIR	:=$(SDK_BOARD_OUT)/linux
export ROOTFS_OUT_DIR	:=$(SDK_BOARD_OUT)/rootfs/target

##########  for target rootfs  #########
export ROOTFS_OUT_LIB:=$(ROOTFS_OUT_DIR)/lib
export ROOTFS_OUT_APP:=$(ROOTFS_OUT_DIR)/usr/sbin
export ROOTFS_OUT_CGI:=$(ROOTFS_OUT_DIR)/www/cgi-bin
export ROOTFS_OUT_DRV:=$(ROOTFS_OUT_DIR)/lib/modules
export APP_DIR		 :=$(ROOTFS_OUT_DIR)/local/bin

##########  for backup rootfs #########
export BACKUP_ROOTFS_OUT_DIR:=$(SDK_BOARD_OUT)/rootfs/backup
export BACKUP_ROOTFS_OUT_LIB:=$(BACKUP_ROOTFS_OUT_DIR)/lib
export BACKUP_ROOTFS_OUT_APP:=$(BACKUP_ROOTFS_OUT_DIR)/usr/sbin
export BACKUP_ROOTFS_OUT_CGI:=$(BACKUP_ROOTFS_OUT_DIR)/www/cgi-bin
export BACKUP_ROOTFS_OUT_DRV:=$(BACKUP_ROOTFS_OUT_DIR)/lib/modules
export BACKUP_APP_DIR:=$(BACKUP_ROOTFS_OUT_DIR)/local/bin

##########  for factory test  ##########
export TEST_APP_DIR:=$(SDK_BOARD_OUT)/rootfs/test
