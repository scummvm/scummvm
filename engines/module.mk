MODULE := engines

MODULE_OBJS := \
	engine.o \
	dialogs.o
# Include common rules
include $(srcdir)/rules.mk
