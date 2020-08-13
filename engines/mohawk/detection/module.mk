MODULE := engines/mohawk/detection

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_MOHAWK), STATIC_PLUGIN)
#ifndef ENABLE_MYST
DETECT_OBJS += $(MODULE)/../myst_metaengine/metaengine.o
#endif
#ifndef ENABLE_RIVEN
DETECT_OBJS += $(MODULE)/../riven_metaengine/metaengine.o
#endif
endif
