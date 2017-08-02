#
# create by liwei
#

$(if $(filter %Kbuild, $(LOCAL_SRCS)),,$(error $(LOCAL_PATH): No Kbuild))

#KBUILD_DIR	:= $(shell cd $(dir $(filter %Kbuild, $(LOCAL_SRCS))) && pwd)			modify by liwei(Â·¾¶×ª»»)
KBUILD_DIR	:= $(dir $(filter %Kbuild, $(LOCAL_SRCS)))
LOCAL_TARGET	:= $(KBUILD_DIR:$(PLATFORM_SRC_DRIVER_DIR)/%=%)

LOCAL_OBJS	:= $(patsubst $(PLATFORM_SRC_DRIVER_DIR)/%, $(PLATFORM_OUT_DRIVER_DIR)/%,$(LOCAL_SRCS))

$(PLATFORM_OUT_DRIVER_DIR)/%: $(PLATFORM_SRC_DRIVER_DIR)/%
	@mkdir -p $(dir $@)
	@cp -dpRf $< $@

prepare_platform_drivers: $(LOCAL_OBJS)

PLATFORM_DRIVER_MODULES += $(LOCAL_TARGET)

