#
# create by liwei
#

ALL_TARGETS :=

###########################################################
## Retrieve the directory of the current makefile
###########################################################

# Figure out where we are.
define my-dir
$(strip \
	$(eval md_file_ := $$(lastword $$(MAKEFILE_LIST))) \
	$(patsubst %/,%,$(dir $(md_file_))) \
	$(eval MAKEFILE_LIST := $$(lastword $$(MAKEFILE_LIST))) \
 )
endef


###########################################################
## Retrieve a list of all makefiles immediately below some directory
###########################################################

define all-makefiles-under
$(wildcard $(1)/*/Makefile.sdk)
endef


###########################################################
## Retrieve a list of all makefiles immediately below your directory
###########################################################

define all-subdir-makefiles
$(call all-makefiles-under,$(call my-dir))
endef


###########################################################
## Add target into ALL_$(CLASS)
###########################################################

define add-target-into-build
$(eval ALL_TARGETS += $(strip $(1)))
endef

#define add-target-into-build
#	$(eval $(1) += $(strip $(2))) \
#	$(eval ALL_TARGETS += $(strip $(1)).$(strip $(2)).$(strip $(call my-dir)))
#endef



###########################################################
## Commands for post-processing the dependency files
###########################################################
define transform-d-to-p
@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	rm -f $(@:%.o=%.d)
endef


###########################################################
## Build static library, if there are libraries in prerequisites, then merge them
###########################################################
# 一定要foreach 否则 $($(LIB).PATH)无效
define add_static_path
$(foreach LIB, $(1), 
	$(if $(wildcard $(LIB)), @echo ADDLIB $(LIB) >> $(dir $@)/ar.mac, 
			@echo ADDLIB $(subst $(SDK_TOPDIR)/,$(SDK_BOARD_OUT)/,$($(LIB).PATH))/$(LIB) >> $(dir $@)/ar.mac))
endef

define build-static-library
	$(if $(wildcard $@),@rm -rf $@)
	$(if $(wildcard $(dir $@)/ar.mac),@rm -rf $(dir $@)/ar.mac)
	$(if $(filter %.a, $^),
		@echo CREATE $@ > $(dir $@)/ar.mac
		@echo SAVE >> $(dir $@)/ar.mac
		@echo END >> $(dir $@)/ar.mac
		@ar -M < $(dir $@)/ar.mac
	)

	$(if $(filter %.o,$^),@ar -c -q $@ $(filter %.o, $^))
	$(if $(filter %.a, $^),
		@echo OPEN $@ > $(dir $@)/ar.mac
		$(foreach LIB, $(filter %.a, $^),
				$(call add_static_path, $(LIB))
		)
		@echo SAVE >> $(dir $@)/ar.mac
		@echo END >> $(dir $@)/ar.mac
		@ar -M < $(dir $@)/ar.mac
		@rm -rf $(dir $@)/ar.mac
	)
endef


###############################
# Usefull functions ported from Kbuild.include
try-run = $(shell set -e; ($(1)) >/dev/null 2>&1 && echo "$(2)" || echo "$(3)")
cc-option = $(call try-run, $(TOOLCHAIN_PATH)/$(CROSS_COMPILE)gcc $(1) -c -xc /dev/null -o /dev/null,$(1),$(2))



