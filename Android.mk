#
# Copyright (C) 2009-2011 The Android-x86 Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
ifneq ($(strip $(MTK_PROJECT_NAME)),)
PRELOADER_ROOT_DIR := $(PWD)
ifneq ($(filter /% ~%,$(TARGET_OUT_INTERMEDIATES)),)
PRELOADER_OUT := $(TARGET_OUT_INTERMEDIATES)/PRELOADER_OBJ
else
PRELOADER_OUT := $(PWD)/$(TARGET_OUT_INTERMEDIATES)/PRELOADER_OBJ
endif
TARGET_PRELOADER = $(PRELOADER_OUT)/bin/preloader_$(PRELOADER_TARGET_PRODUCT).bin

.PHONY: clean-pl pl
clean-pl:
	$(hide) rm -rf $(PRELOADER_OUT)

#$(PRELOADER_OUT):
#	@mkdir -p $(PRELOADER_OUT)

# Top level for eMMC variant targets
$(TARGET_PRELOADER): clean-pl
	@cd bootable/bootloader/preloader && TARGET_PRODUCT=$(PRELOADER_TARGET_PRODUCT) MTK_TARGET_PROJECT=$(MTK_TARGET_PROJECT) PRELOADER_OUT=$(PRELOADER_OUT) ROOTDIR=$(PRELOADER_ROOT_DIR) ./build.sh
	@cp $(TARGET_PRELOADER) $(PRODUCT_OUT)/preloader_$(PRELOADER_TARGET_PRODUCT).bin

pl: $(TARGET_PRELOADER)

droidcore: $(TARGET_PRELOADER)

droid: check-pl-config
check-mtk-config: check-pl-config
check-pl-config:
	python device/mediatek/build/build/tools/check_kernel_config.py -c $(MTK_TARGET_PROJECT_FOLDER)/ProjectConfig.mk -b bootable/bootloader/preloader/custom/$(PRELOADER_TARGET_PRODUCT)/$(PRELOADER_TARGET_PRODUCT).mk -p $(MTK_PROJECT_NAME)

endif
endif

