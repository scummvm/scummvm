MODULE := engines/scumm/detection

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_SCUMM), STATIC_PLUGIN)
DETECT_OBJS += $(MODULE)/../file.o
DETECT_OBJS += $(MODULE)/../file_nes.o
endif
