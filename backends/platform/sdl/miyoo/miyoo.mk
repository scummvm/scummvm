OD_EXE_STRIPPED := scummvm_stripped$(EXEEXT)
bundle = sd-root

G2X_CATEGORY = games

all: $(OD_EXE_STRIPPED)

$(OD_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

$(bundle): all
	$(RM) -rf $(bundle)
	$(MKDIR) -p $(bundle)/$(G2X_CATEGORY)/scummvm
	$(CP) $(DIST_FILES_DOCS) $(bundle)/$(G2X_CATEGORY)/scummvm
	$(MKDIR) $(bundle)/$(G2X_CATEGORY)/scummvm/themes
	$(CP) $(DIST_FILES_THEMES) $(bundle)/$(G2X_CATEGORY)/scummvm/themes/

ifdef DIST_FILES_ENGINEDATA
	$(MKDIR) $(bundle)/$(G2X_CATEGORY)/scummvm/engine-data
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle)/$(G2X_CATEGORY)/scummvm/engine-data/
endif
ifdef DIST_FILES_NETWORKING
	$(CP) $(DIST_FILES_NETWORKING) $(bundle)/$(G2X_CATEGORY)/scummvm
endif
ifdef DIST_FILES_VKEYBD
	$(CP) $(DIST_FILES_VKEYBD) $(bundle)/$(G2X_CATEGORY)/scummvm
endif
ifdef DYNAMIC_MODULES
	$(MKDIR) $(bundle)/$(G2X_CATEGORY)/scummvm/plugins/
	$(CP) $(PLUGINS) $(bundle)/$(G2X_CATEGORY)/scummvm/plugins/
endif
	$(CP) $(EXECUTABLE) $(bundle)/$(G2X_CATEGORY)/scummvm/scummvm
	$(CP) $(srcdir)/backends/platform/sdl/miyoo/README.MIYOO $(bundle)/$(G2X_CATEGORY)/scummvm/README.man.txt
	echo >> $(bundle)/$(G2X_CATEGORY)/scummvm/README.man.txt
	echo '[General README]' >> $(bundle)/$(G2X_CATEGORY)/scummvm/README.man.txt
	echo >> $(bundle)/$(G2X_CATEGORY)/scummvm/README.man.txt
	cat $(srcdir)/README.md | sed -e 's/\[/⟦/g' -e 's/\]/⟧/g' -e '/^1\.1)/,$$ s/^[0-9][0-9]*\.[0-9][0-9]*.*/\[&\]/' >> $(bundle)/$(G2X_CATEGORY)/scummvm/README.man.txt
	echo '[General README]' >> $(bundle)/$(G2X_CATEGORY)/scummvm/README.man.txt
	$(MKDIR) -p $(bundle)/gmenu2x/sections/$(G2X_CATEGORY)
	$(CP) $(srcdir)/dists/miyoo/scummvm.miyoo $(bundle)/gmenu2x/sections/$(G2X_CATEGORY)/scummvm
	$(STRIP) $(bundle)/$(G2X_CATEGORY)/scummvm/scummvm

sd-zip: $(bundle)
	$(RM) scummvm_$(MIYOO_TARGET).zip
	cd $(bundle) && zip -r ../scummvm_$(MIYOO_TARGET).zip $(G2X_CATEGORY) gmenu2x
