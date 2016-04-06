TARGET := scummvm

APP_TITLE       := ScummVM
APP_DESCRIPTION := Point-and-click adventure game engines
APP_AUTHOR      := ScummVM Team
APP_ICON        := backends/platform/3ds/icon.png

ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CXXFLAGS += -std=gnu++11
ASFLAGS  += -mfloat-abi=hard
LDFLAGS  += -specs=3dsx.specs $(ARCH) -L$(DEVKITPRO)/libctru/lib -L$(DEVKITPRO)/portlibs/3ds/lib

.PHONY: clean_3ds

all: $(TARGET).3dsx

clean: clean_3ds

clean_3ds:
	$(RM) $(TARGET).3dsx

$(TARGET).smdh: $(APP_ICON) $(MAKEFILE_LIST)
	@smdhtool --create "$(APP_TITLE)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $(APP_ICON) $@
	@echo built ... $(notdir $@)

$(TARGET).3dsx: $(EXECUTABLE) $(TARGET).smdh
	@3dsxtool $< $@ --smdh=$(TARGET).smdh
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

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))
