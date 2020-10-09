MODULE := backends/platform/android3d

MODULE_OBJS := \
	jni-android.o \
	texture.o \
	asset-archive.o \
	android.o \
	graphics.o \
	events.o \
	snprintf.o \
	touchcontrols.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
