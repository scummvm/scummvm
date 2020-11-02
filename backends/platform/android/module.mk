MODULE := backends/platform/android

MODULE_OBJS := \
	jni-android.o \
	asset-archive.o \
	android.o \
	graphics.o \
	events.o \
	options.o \
	snprintf.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
