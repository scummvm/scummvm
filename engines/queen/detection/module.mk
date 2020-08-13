MODULE := engines/queen/detection

# Detection objecs
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_QUEEN), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/../resource.o
DETECT_OBJS += $(MODULE)/../restables.o
endif
