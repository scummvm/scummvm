OD_EXE_STRIPPED := scummvm_stripped$(EXEEXT)
bundle = od-opk

all: $(OD_EXE_STRIPPED)

$(OD_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

$(bundle): all
	$(MKDIR) $(bundle)
	$(CP) $(DIST_FILES_DOCS) $(bundle)/

ifneq ($(OPENDINGUX_TARGET), rg99)
	$(MKDIR) $(bundle)/themes
	$(CP) $(DIST_FILES_THEMES) $(bundle)/themes/
endif

ifdef DIST_FILES_ENGINEDATA
	$(MKDIR) $(bundle)/engine-data
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle)/engine-data/
ifeq ($(OPENDINGUX_TARGET), rg99)
	$(CP) $(srcdir)/dists/opendingux/fonts_mini.dat $(bundle)/engine-data/fonts.dat
endif
endif
ifdef DIST_FILES_NETWORKING
	$(CP) $(DIST_FILES_NETWORKING) $(bundle)/
endif
ifdef DIST_FILES_VKEYBD
	$(CP) $(DIST_FILES_VKEYBD) $(bundle)/
endif
ifdef DYNAMIC_MODULES
	$(MKDIR) $(bundle)/plugins
	$(CP) $(PLUGINS) $(bundle)/plugins/
endif
	$(CP) $(EXECUTABLE) $(bundle)/scummvm

	$(CP) $(srcdir)/dists/opendingux/scummvm.png $(bundle)/
	$(CP) $(srcdir)/dists/opendingux/startUI.$(OPENDINGUX_TARGET).desktop $(bundle)/
ifdef dualopk
	$(CP) $(srcdir)/dists/opendingux/startGame.$(OPENDINGUX_TARGET).desktop $(bundle)/
	$(CP) $(srcdir)/dists/opendingux/scummvm.sh $(bundle)/
endif
	$(CP) $(srcdir)/backends/platform/sdl/opendingux/README.OPENDINGUX $(bundle)/README.man.txt
	echo >> $(bundle)/README.man.txt
	echo '[General README]' >> $(bundle)/README.man.txt
	echo >> $(bundle)/README.man.txt
	cat $(srcdir)/README.md | sed -e 's/\[/⟦/g' -e 's/\]/⟧/g' -e '/^1\.1)/,$$ s/^[0-9][0-9]*\.[0-9][0-9]*.*/\[&\]/' >> $(bundle)/README.man.txt

od-make-opk: $(bundle)
	$(STRIP) $(bundle)/scummvm

ifdef dualopk
	$(srcdir)/dists/opendingux/make-opk.sh -d $(bundle) -o scummvm_$(OPENDINGUX_TARGET)_dual
else
	$(srcdir)/dists/opendingux/make-opk.sh -d $(bundle) -o scummvm_$(OPENDINGUX_TARGET)
endif
