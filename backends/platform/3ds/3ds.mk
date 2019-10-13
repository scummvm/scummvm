TARGET := scummvm

APP_TITLE       := ScummVM
APP_DESCRIPTION := Point-and-click adventure game engines
APP_AUTHOR      := ScummVM Team
APP_ICON        := $(srcdir)/backends/platform/3ds/app/icon.png

APP_RSF         := $(srcdir)/backends/platform/3ds/app/scummvm.rsf
APP_BANNER_IMAGE:= $(srcdir)/backends/platform/3ds/app/banner.png
APP_BANNER_AUDIO:= $(srcdir)/backends/platform/3ds/app/banner.wav

ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CXXFLAGS += -std=gnu++11
ASFLAGS  += -mfloat-abi=hard
LDFLAGS  += -specs=3dsx.specs $(ARCH) -L$(DEVKITPRO)/libctru/lib -L$(DEVKITPRO)/portlibs/3ds/lib

.PHONY: clean_3ds

clean: clean_3ds

clean_3ds:
	$(RM) $(TARGET).3dsx
	$(RM) $(TARGET).cia
	$(RM) -rf romfs

romfs: $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD)
	@rm -rf romfs
	@mkdir -p romfs
	@cp $(DIST_FILES_THEMES) romfs/
ifdef DIST_FILES_ENGINEDATA
	@cp $(DIST_FILES_ENGINEDATA) romfs/
endif
ifdef DIST_FILES_NETWORKING
	@cp $(DIST_FILES_NETWORKING) romfs/
endif
ifdef DIST_FILES_VKEYBD
	@cp $(DIST_FILES_VKEYBD) romfs/
endif

$(TARGET).smdh: $(APP_ICON)
	@smdhtool --create "$(APP_TITLE)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $(APP_ICON) $@
	@echo built ... $(notdir $@)

$(TARGET).3dsx: $(EXECUTABLE) $(TARGET).smdh romfs
	@3dsxtool $< $@ --smdh=$(TARGET).smdh --romfs=romfs
	@echo built ... $(notdir $@)
	
$(TARGET).bnr: $(APP_BANNER_IMAGE) $(APP_BANNER_AUDIO)
	@bannertool makebanner -o $@ -i $(APP_BANNER_IMAGE) -a $(APP_BANNER_AUDIO)
	@echo built ... $(notdir $@)

$(TARGET).romfs: romfs
	@3dstool -cvtf romfs $(TARGET).romfs --romfs-dir romfs
	@echo built ... $(notdir $@)

$(TARGET).cia: $(EXECUTABLE) $(APP_RSF) $(TARGET).smdh $(TARGET).bnr $(TARGET).romfs
	@makerom -f cia -target t -exefslogo -o $@ -elf $(EXECUTABLE) -rsf $(APP_RSF) -banner $(TARGET).bnr -icon $(TARGET).smdh -romfs $(TARGET).romfs
	@echo built ... $(notdir $@)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval FILEPATH := $(patsubst %.shbin.o,%.shbin,$@))
	$(eval FILE := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	picasso -o $(FILEPATH) $1
	bin2s $(FILEPATH) | $(AS) -o $@
	echo "extern const u8" `(echo $(FILE) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(FILEPATH) | tr . _)`.h
	echo "extern const u8" `(echo $(FILE) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(FILEPATH) | tr . _)`.h
	echo "extern const u32" `(echo $(FILE) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(FILEPATH) | tr . _)`.h
endef

vpath %.v.pica $(srcdir)
vpath %.g.pica $(srcdir)
vpath %.shlist $(srcdir)

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))
