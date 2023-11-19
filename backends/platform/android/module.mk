MODULE := backends/platform/android

MODULE_OBJS := \
	jni-android.o \
	asset-archive.o \
	android.o \
	events.o \
	options.o \
	snprintf.o \
	touchcontrols.o

ifdef NEED_ANDROID_CPUFEATURES
MODULE_OBJS += \
	cpu-features.o
$(MODULE)/android.o: CXXFLAGS += "-I$(ANDROID_NDK_ROOT)/sources/android/cpufeatures"
# We don't configure a C compiler, use a C++ one in C mode
$(MODULE)/cpu-features.o: $(ANDROID_NDK_ROOT)/sources/android/cpufeatures/cpu-features.c
	$(QUIET)$(MKDIR) $(*D)
	$(QUIET_CXX)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -x c -std=c99 -c $(<) -o $@
endif

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
