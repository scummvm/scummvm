MODULE := graphics

MODULE_OBJS := \
	graphics/animation.o \
	graphics/font.o \
	graphics/scummfont.o \
	graphics/newfont.o

MODULE_DIRS += \
	graphics

# Include common rules 
include $(srcdir)/common.rules
