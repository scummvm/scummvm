MODULE := base/detection

DETECT_OBJS_DYNAMIC=$(addprefix ../../,$(DETECT_OBJS))

MODULE_OBJS := \
	detection.o \
	$(DETECT_OBJS_DYNAMIC)

# Reset detect objects, so none of them build into the executable.
DETECT_OBJS :=

PLUGIN := 1

# Include common rules
include $(srcdir)/rules.mk
