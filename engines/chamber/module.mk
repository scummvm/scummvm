MODULE := engines/chamber

MODULE_OBJS := \
	anim.o \
	bkbuff.o \
	cga.o \
	chamber.o \
	cursor.o \
	decompr.o \
	dialog.o \
	input.o \
	invent.o \
	kult.o \
	menu.o \
	metaengine.o \
	portrait.o \
	print.o \
	r_other.o \
	r_pers.o \
	r_puzzl.o \
	r_sprit.o \
	r_templ.o \
	r_texts.o \
	resdata.o \
	room.o \
	savegame.o \
	script.o \
	sound.o \
	timer.o

MODULE_DIRS += \
	engines/chamber

# This module can be built as a plugin
ifeq ($(ENABLE_CHAMBER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
