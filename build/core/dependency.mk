#
# create by liwei
#

$(ALL_TARGETS): $(DOT_CONFIG)

# Add clean target
.PHONY: clean_rootfs clean_app clean_linux

clean_rootfs:
	@rm -rf $(ROOTFS_OUT_DIR)

clean_app:
	@rm -rf $(SDK_BOARD_OUT)/apps

clean_linux:
	@rm -rf $(SDK_BOARD_OUT)/linux

