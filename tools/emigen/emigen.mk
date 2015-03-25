##############################################################
# Emigen Input Files
#
CUSTOM_MEMORY_HDR = $(PWD)/custom/$(TARGET)/inc/custom_MemoryDevice.h
ifeq ("$(MACH_TYPE)","mt6735m")
MEMORY_DEVICE_XLS = $(PWD)/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_$(MTK_PLATFORM)M.xls
else ifeq ("$(MACH_TYPE)","mt6753")
MEMORY_DEVICE_XLS = $(PWD)/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_MT6753.xls
else
MEMORY_DEVICE_XLS = $(PWD)/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_$(MTK_PLATFORM).xls
endif

##############################################################
# Emigen Building Tool
#
EMIGEN_PERL := $(PWD)/tools/emigen/$(MTK_PLATFORM)/emigen.pl 

##############################################################
# Emigen generate parameter for header and tag files
#
BUILD_EMI_H := 0
BUILD_LOADER_TAG := 1

##############################################################
# Emigen Output Path
#

# Customer Define
#EMIGEN_OUT := $(PWD)/custom/$(MTK_PROJECT)

# Default Output to out folder
ifndef EMIGEN_OUT
EMIGEN_OUT := $(PRELOADER_OUT)
endif
export EMIGEN_OUT

EMIGEN_TAG_OUT := $(EMIGEN_OUT)
EMIGEN_H_OUT := $(EMIGEN_OUT)/inc

##############################################################
# Emigen Generate API
#

$(EMIGEN_H_OUT)/custom_emi.h: $(EMIGEN_PERL) $(CUSTOM_MEMORY_HDR) $(MEMORY_DEVICE_XLS)
	@echo [current path]=$(PWD)
	@echo [emigen_out]=$(EMIGEN_OUT)
	@perl $(EMIGEN_PERL) $(CUSTOM_MEMORY_HDR) \
                     $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(TARGET) $(EMIGEN_H_OUT) $(BUILD_EMI_H)

$(EMIGEN_TAG_OUT)/MTK_Loader_Info.tag: $(EMIGEN_PERL) $(CUSTOM_MEMORY_HDR) $(MEMORY_DEVICE_XLS)
	@perl $(EMIGEN_PERL) $(CUSTOM_MEMORY_HDR) \
                     $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(TARGET) $(EMIGEN_TAG_OUT) $(BUILD_LOADER_TAG)
