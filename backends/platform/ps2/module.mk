MODULE := backends/platform/ps2

MODULE_OBJS := \
	DmaPipe.o \
    Gs2dScreen.o \
    irxboot.o \
	ps2input.o \
	ps2pad.o \
	savefilemgr.o \
    fileio.o \
    asyncfio.o \
	icon.o \
    cd.o \
    eecodyvdfs.o \
    rpckbd.o \
    systemps2.o \
    ps2mutex.o \
    ps2time.o \
	ps2debug.o

MODULE_DIRS += \
	backends/platform/ps2/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
