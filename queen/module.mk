MODULE := queen

MODULE_OBJS = \
	queen/logic.o \
	queen/resource.o \
	queen/restables.o \
	queen/queen.o \
	queen/version.o \

# Include common rules 
include common.rules
