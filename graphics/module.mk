MODULE := graphics

MODULE_OBJS := \
	graphics/animation.o \
	graphics/scummfont.o \
	graphics/font.o \
	graphics/fontman.o \
	graphics/newfont.o \
	graphics/newfont_big.o \
	graphics/surface.o

MODULE_DIRS += \
	graphics

# Include common rules 
include $(srcdir)/common.rules
