MODULE := backends/platform/psp

MODULE_OBJS := \
	powerman.o \
	psp_main.o \
	osys_psp.o \
	kbd_ss_c.o \
	kbd_s_c.o \
	kbd_ls_c.o \
	kbd_l_c.o \
	trace.o \
	psploader.o

MODULE_DIRS += \
	backends/platform/psp/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
