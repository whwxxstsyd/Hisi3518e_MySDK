##
## create by liwei
##

$(if $(LOCAL_TARGET),,$(error $(LOCAL_PATH): LOCAL_TARGET is not defined))

$(LOCAL_TARGET).PATH := $(LOCAL_PATH)

# 如果存在CPP文件，使用G++编译
LOCAL_CXX	:= $(if $(filter %.cpp,$(LOCAL_SRCS)),$(CROSS_COMPILE)g++,$(CROSS_COMPILE)gcc)

# 去除文件
NONE_OBJS := $(patsubst $(SDK_TOPDIR)/%.c, $(SDK_BOARD_OUT)/%.o,$(NONE_SRCS))
NONE_OBJS := $(patsubst $(SDK_TOPDIR)/%.cpp, $(SDK_BOARD_OUT)/%.o,$(NONE_OBJS))
NONE_OBJS := $(patsubst $(SDK_TOPDIR)/%.S, $(SDK_BOARD_OUT)/%.o,$(NONE_OBJS))
NONE_OBJS := $(patsubst $(SDK_TOPDIR)/%.asm, $(SDK_BOARD_OUT)/%.o,$(NONE_OBJS))

# 把C文件对应的目标文件包含
LOCAL_OBJS	:= $(patsubst $(SDK_TOPDIR)/%.c, $(SDK_BOARD_OUT)/%.o,$(LOCAL_SRCS))
# 把CPP文件对应的目标文件包含
LOCAL_OBJS	:= $(patsubst $(SDK_TOPDIR)/%.cpp, $(SDK_BOARD_OUT)/%.o,$(LOCAL_OBJS))
# 把S文件对应的目标文件包含
LOCAL_OBJS	:= $(patsubst $(SDK_TOPDIR)/%.S, $(SDK_BOARD_OUT)/%.o,$(LOCAL_OBJS))
# 把ASM文件对应的目标文件包含
LOCAL_OBJS	:= $(patsubst $(SDK_TOPDIR)/%.asm, $(SDK_BOARD_OUT)/%.o,$(LOCAL_OBJS))
# 去掉静态库文件
LOCAL_OBJS	:= $(filter-out %.a, $(LOCAL_OBJS))
LOCAL_OBJS	:= $(filter-out $(NONE_OBJS), $(LOCAL_OBJS))

# 设置目标路径
LOCAL_MODULE	:= $(patsubst $(SDK_TOPDIR)/%, $(SDK_BOARD_OUT)/%, $(LOCAL_PATH))/$(LOCAL_TARGET)
# 定义动态库名字
LOCAL_SO_NAME ?= $(LOCAL_TARGET)

ifeq ($(MEMORY_WATCH), y)
LOCAL_LIBS		+= libMemWatch.a	
endif
LOCAL_LIBS	:= $(filter-out $(NONE_LIBS), $(LOCAL_LIBS))

# define variables owned by specific $(LOCAL_MODULE)
__OWN_LIBS__	:= $(patsubst lib%.so, -l%, $(filter %.so,$(LOCAL_LIBS)))
__OWN_LIBS__	+= $(patsubst lib%.a, -l%, $(filter %.a,$(LOCAL_LIBS)))
$(LOCAL_MODULE): PRIVATE_LIBS := $(LOCAL_LIBS)
$(LOCAL_MODULE): PRIVATE_CFLAGS := $(LOCAL_CFLAGS)
$(LOCAL_MODULE): PRIVATE_AFLAGS := $(LOCAL_AFLAGS)
$(LOCAL_MODULE): PRIVATE_LDFLAGS := $(__OWN_LIBS__) $(LOCAL_LDFLAGS)
$(LOCAL_MODULE): PRIVATE_SO_FLAGS := -Wl,-soname,$(LOCAL_SO_NAME)

# include dependency files	(包涵每个目标文件的依赖文件)
-include $(LOCAL_OBJS:%.o=%.P)

# compile
$(SDK_BOARD_OUT)/%.o: $(SDK_TOPDIR)/%.c $(LOCAL_PATH)/Makefile.sdk
	@mkdir -p $(dir $@)
	$(PRIVATE_CXX) $(SDK_APP_CFLAGS) $(PRIVATE_CFLAGS) -MMD -c $< -o $@
	$(call transform-d-to-p)

$(SDK_BOARD_OUT)/%.o: $(SDK_TOPDIR)/%.cpp $(LOCAL_PATH)/Makefile.sdk
	@mkdir -p $(dir $@)
	$(PRIVATE_CXX) $(SDK_APP_CFLAGS) $(PRIVATE_CFLAGS) -MMD -c $< -o $@
	$(call transform-d-to-p)

$(SDK_BOARD_OUT)/%.o: $(SDK_TOPDIR)/%.S $(LOCAL_PATH)/Makefile.sdk
	@mkdir -p $(dir $@)
	$(if $(findstring COMPILE_S_WITH_GCC, $(PRIVATE_CFLAGS)), \
		$(CROSS_COMPILE)gcc $(PRIVATE_CFLAGS) -c $< -o $@, \
		$(CROSS_COMPILE)as $(PRIVATE_AFLAGS) -o $@ $<)

$(SDK_BOARD_OUT)/%.o: $(SDK_TOPDIR)/%.asm $(LOCAL_PATH)/Makefile.sdk
	@mkdir -p $(dir $@)
	$(CROSS_COMPILE)as $(AMBARELLA_APP_AFLAGS) $(PRIVATE_AFLAGS) -o $@ $<

# link
LOCAL_MODULE_TYPE := $(if $(filter %.a, $(LOCAL_MODULE)), static_library, \
			$(if $(filter %.so, $(LOCAL_MODULE)), shared_library, exectutable))

$(LOCAL_MODULE): PRIVATE_CXX:=$(LOCAL_CXX)
$(LOCAL_MODULE): PRIVATE_MODULE_TYPE:=$(LOCAL_MODULE_TYPE)
$(LOCAL_MODULE): $(LOCAL_OBJS) $(filter %.a, $(LOCAL_SRCS)) $(LOCAL_LIBS) $(LOCAL_PATH)/Makefile.sdk
	$(if $(findstring static_library, $(PRIVATE_MODULE_TYPE)),	\
		$(build-static-library))
	$(if $(findstring shared_library, $(PRIVATE_MODULE_TYPE)), \
		mkdir -p $(APP_DIR)/../lib && \
		$(PRIVATE_CXX) $(SDK_APP_LDFLAGS) $(PRIVATE_SO_FLAGS) -shared -o $@ $(filter-out %.a %.so %Makefile.sdk, $^) \
		$(foreach p, $(PRIVATE_LIBS), -L$(subst $(SDK_TOPDIR)/,$(SDK_BOARD_OUT)/,$($(p).PATH))) $(PRIVATE_LDFLAGS) && \
		$(CROSS_COMPILE)strip --strip-unneeded $@ && cp $@ $(APP_DIR)/../lib) 
	$(if $(findstring exectutable, $(PRIVATE_MODULE_TYPE)), \
		mkdir -p $(APP_DIR) && $(PRIVATE_CXX) \
		$(foreach p, $(PRIVATE_LIBS), -L$(subst $(SDK_TOPDIR)/,$(SDK_BOARD_OUT)/,$($(p).PATH))) \
		-o $@ $(filter-out %.a %.so %Makefile.sdk, $^)  $(PRIVATE_LDFLAGS) $(SDK_APP_LDFLAGS) && \
		cp $@ $(APP_DIR) && $(CROSS_COMPILE)strip -s $(APP_DIR)/$(notdir $@))					

		
