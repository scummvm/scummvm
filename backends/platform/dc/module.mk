MODULE := backends/platform/dc

MODULE_OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o softkbd.o dcloader.o cache.o dc-fs.o

MODULE_DIRS += \
        backends/platform/dc/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
