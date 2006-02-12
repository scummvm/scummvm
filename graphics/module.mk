MODULE := graphics

MODULE_OBJS := \
	animation.o \
	consolefont.o \
	font.o \
	fontman.o \
	ilbm.o \
	newfont.o \
	newfont_big.o \
	primitives.o \
	scummfont.o \
	surface.o \
	imageman.o \
	imagedec.o

MODULE_DIRS += \
	graphics

# Include common rules 
include $(srcdir)/common.rules
