MODULE := backends/platform/psp

MODULE_OBJS := \
	powerman.o \
	psp_main.o \
	osys_psp.o \
	trace.o \
	psploader.o \
	pspkeyboard.o

MODULE_DIRS += \
	backends/platform/psp/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
