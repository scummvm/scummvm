MODULE := backends/platform/ds

PORT_OBJS := \
	arm9/source/blitters_arm.o \
	arm9/source/dsmain.o \
	arm9/source/scummhelp.o \
	arm9/source/osystem_ds.o \
	arm9/source/touchkeyboard.o \
	arm9/source/dsoptions.o \
	arm9/source/wordcompletion.o

DATA_OBJS := \
	arm9/data/icons.o \
	arm9/data/keyboard.o \
	arm9/data/keyboard_pal.o \
	arm9/data/default_font.o \
	arm9/data/8x8font_tga.o


MODULE_OBJS := $(DATA_OBJS) $(PORT_OBJS)


#---------------------------------------------------------------------------------
# canned command sequence for binary data
#---------------------------------------------------------------------------------
define bin2o
	$(MKDIR) $(*D)
	bin2s $< | $(AS) -mthumb -mthumb-interwork -o $(@)
endef

define bin2h
	$(MKDIR) $(*D)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > $@
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> $@
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> $@
endef

vpath %.raw $(srcdir)
vpath %.pal $(srcdir)
vpath %.bin $(srcdir)

%.o: %.raw
	$(bin2o)

%_raw.h: %.raw
	$(bin2h)

%.o: %.pal
	$(bin2o)

%_raw.h: %.pal
	$(bin2h)

%.o: %.bin
	$(bin2o)

%_raw.h: %.bin
	$(bin2h)


# Mark files which require the *_raw.h files manually (for now, at least)
$(MODULE)/arm9/source/dsmain.o: \
	$(MODULE)/arm9/data/icons_raw.h \
	$(MODULE)/arm9/data/keyboard_raw.h \
	$(MODULE)/arm9/data/keyboard_pal_raw.h

$(MODULE)/arm9/source/touchkeyboard.o: \
	$(MODULE)/arm9/data/keyboard_raw.h \
	$(MODULE)/arm9/data/keyboard_pal_raw.h \
	$(MODULE)/arm9/data/8x8font_tga_raw.h


# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
