MODULE := graphics

MODULE_OBJS := \
	graphics/animation.o \
	graphics/font.o \
	graphics/newfont.o \
	graphics/scummfont.o \
	graphics/surface.o

MODULE_DIRS += \
	graphics

# Include common rules 
include $(srcdir)/common.rules
