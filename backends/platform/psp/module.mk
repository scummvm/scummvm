MODULE := backends/platform/psp

MODULE_OBJS := powerman.o \
	psp_main.o \
	osys_psp.o \
	psppixelformat.o \
	memory.o \
	display_manager.o \
	display_client.o \
	default_display_client.o \
	input.o \
	cursor.o \
	trace.o \
	psploader.o \
	pspkeyboard.o \
	audio.o \
	timer.o

MODULE_DIRS += \
	backends/platform/psp/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
