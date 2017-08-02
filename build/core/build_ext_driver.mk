##
## create by liwei
##

$(if $(filter %Kbuild, $(LOCAL_SRCS)),,$(error $(LOCAL_PATH): No Kbuild))

KBUILD_DIR	:= $(shell cd $(dir $(filter %Kbuild, $(LOCAL_SRCS))) && pwd)
LOCAL_TARGET	:= $(KBUILD_DIR:$(EXT_DRIVERS_SRCS_DIR)/%=%)

LOCAL_OBJS	:= $(patsubst $(EXT_DRIVERS_SRCS_DIR)/%, $(EXT_DRIVERS_OUT_DIR)/%,$(LOCAL_SRCS))

$(EXT_DRIVERS_OUT_DIR)/%: $(EXT_DRIVERS_SRCS_DIR)/%
	@mkdir -p $(dir $@)
	@cp -dpRf $< $@

prepare_external_drivers: $(LOCAL_OBJS)

EXT_DRIVER_MODULES += $(LOCAL_TARGET)/

