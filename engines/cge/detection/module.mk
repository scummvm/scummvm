MODULE := engines/cge/detection

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_CGE), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/../fileio.o
endif
