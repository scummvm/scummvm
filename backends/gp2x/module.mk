MODULE := backends/gp2x

MODULE_OBJS := \
	gp2x-hw.o  \
	gp2x-mem.o  \
	events.o \
	graphics.o \
	gp2x.o 
	# overload_help.o 

MODULE_DIRS += \
	backends/gp2x/

# We don't use the common.rules here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
