MODULE := graphics

MODULE_OBJS := \
	graphics/animation.o \
	graphics/consolefont.o \
	graphics/font.o \
	graphics/fontman.o \
	graphics/ilbm.o \
	graphics/newfont.o \
	graphics/newfont_big.o \
	graphics/primitives.o \
	graphics/scummfont.o \
	graphics/surface.o

MODULE_DIRS += \
	graphics

# Include common rules 
include $(srcdir)/common.rules
