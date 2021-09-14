DINGUX_EXE_STRIPPED := scummvm_stripped$(EXEEXT)
HUGEEDIT = hugeedit

bundle_name = dingux-dist/scummvm
bundle = od-opk

all: $(DINGUX_EXE_STRIPPED)

$(DINGUX_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

$(bundle): all #GeneralUser\ GS\ FluidSynth\ v1.44.sf2
	$(MKDIR) $(bundle)
	$(CP) $(DIST_FILES_DOCS) $(bundle)/
	$(MKDIR) $(bundle)/themes
	$(CP) $(DIST_FILES_THEMES) $(bundle)/themes/
ifdef DIST_FILES_ENGINEDATA
	$(MKDIR) $(bundle)/engine-data
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle)/engine-data/
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
	$(CP) $(srcdir)/dists/opendingux/default.$(VARIANT).desktop $(bundle)/
	$(CP) $(srcdir)/dists/opendingux/scummvmrc $(bundle)/
	$(CP) $(srcdir)/dists/opendingux/scummvm.sh $(bundle)/
	$(CP) $(srcdir)/backends/platform/opendingux/README.OPENDINGUX $(bundle)/README.man.txt
	echo >> $(bundle)/README.man.txt
	echo '[General README]' >> $(bundle)/README.man.txt
	echo >> $(bundle)/README.man.txt
	cat $(srcdir)/README.md | sed -e 's/\[/⟦/g' -e 's/\]/⟧/g' -e '/^1\.1)/,$$ s/^[0-9][0-9]*\.[0-9][0-9]*.*/\[&\]/' >> $(bundle)/README.man.txt

#$(CP) GeneralUser\ GS\ FluidSynth\ v1.44.sf2 $(bundle)/

od-make-opk-unstripped: $(bundle)
	$(CP) $(PLUGINS) $(bundle)/plugins/
	$(CP) $(EXECUTABLE) $(bundle)/scummvm
	$(HUGEEDIT) --text --data $(bundle)/scummvm
	$(srcdir)/dists/opendingux/make-opk.sh -d $(bundle) -o scummvm_$(VARIANT)

od-make-opk: $(bundle)
	$(STRIP) $(bundle)/plugins/*
	$(STRIP) $(bundle)/scummvm
	$(HUGEEDIT) --text --data $(bundle)/scummvm
	$(srcdir)/dists/opendingux/make-opk.sh -d $(bundle) -o scummvm_$(VARIANT)

GeneralUser_GS_1.44-FluidSynth.zip:
	curl -s https://spacehopper.org/mirrors/GeneralUser_GS_1.44-FluidSynth.zip -o GeneralUser_GS_1.44-FluidSynth.zip

GeneralUser\ GS\ FluidSynth\ v1.44.sf2: GeneralUser_GS_1.44-FluidSynth.zip
	unzip -n GeneralUser_GS_1.44-FluidSynth.zip
	mv "GeneralUser GS 1.44 FluidSynth/GeneralUser GS FluidSynth v1.44.sf2" .
	mv "GeneralUser GS 1.44 FluidSynth/README.txt" README.soundfont
	mv "GeneralUser GS 1.44 FluidSynth/LICENSE.txt" LICENSE.soundfont
