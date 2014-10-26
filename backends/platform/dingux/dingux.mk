DINGUX_EXE_STRIPPED := scummvm_stripped$(EXEEXT)

bundle_name = dingux-dist/scummvm
gcw0_bundle = gcw0-opk

all: $(DINGUX_EXE_STRIPPED)

$(DINGUX_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

dingux-distclean:
	rm -rf $(bundle_name)
	rm $(DINGUX_EXE_STRIPPED)

dingux-dist: all
	$(MKDIR) $(bundle_name)
	$(MKDIR) $(bundle_name)/saves
	$(STRIP) $(EXECUTABLE) -o $(bundle_name)/scummvm.elf
	$(CP) $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_ENGINEDATA
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(CP) $(DIST_FILES_DOCS) $(bundle_name)/
ifdef DYNAMIC_MODULES
		$(MKDIR) $(bundle_name)/plugins
		$(CP) $(PLUGINS) $(bundle_name)/plugins
		$(STRIP) $(bundle_name)/plugins/*
endif
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(bundle_name)/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_small.zip $(bundle_name)/
	$(CP) $(srcdir)/backends/platform/dingux/scummvm.gpe $(bundle_name)/
	$(CP) $(srcdir)/backends/platform/dingux/README.DINGUX $(bundle_name)/
	$(CP) $(srcdir)/backends/platform/dingux/scummvm.png $(bundle_name)/

# Special target for generationg GCW-Zero OPK bundle
$(gcw0_bundle): all
	$(MKDIR) $(gcw0_bundle)
	$(CP) $(DIST_FILES_DOCS) $(gcw0_bundle)/
	$(MKDIR) $(gcw0_bundle)/themes
	$(CP) $(DIST_FILES_THEMES) $(gcw0_bundle)/themes/
ifdef DIST_FILES_ENGINEDATA
	$(MKDIR) $(gcw0_bundle)/engine-data
	$(CP) $(DIST_FILES_ENGINEDATA) $(gcw0_bundle)/engine-data/
endif
ifdef DYNAMIC_MODULES
	$(MKDIR) $(gcw0_bundle)/plugins
	$(CP) $(PLUGINS) $(gcw0_bundle)/plugins/
endif
	$(CP) $(EXECUTABLE) $(gcw0_bundle)/scummvm

	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(gcw0_bundle)/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_small.zip $(gcw0_bundle)/

	$(CP) $(srcdir)/dists/gcw0/scummvm.png $(gcw0_bundle)/
	$(CP) $(srcdir)/dists/gcw0/default.gcw0.desktop $(gcw0_bundle)/
	$(CP) $(srcdir)/dists/gcw0/scummvmrc $(gcw0_bundle)/
	$(CP) $(srcdir)/dists/gcw0/scummvm.sh $(gcw0_bundle)/

gcw0-opk-unstripped: $(gcw0_bundle)
	$(CP) $(PLUGINS) $(gcw0_bundle)/plugins/
	$(CP) $(EXECUTABLE) $(gcw0_bundle)/scummvm
	./dists/gcw0/opk_make.sh -d $(gcw0_bundle) -o scummvm

gcw-opk: $(gcw0_bundle)
	$(STRIP) $(gcw0_bundle)/plugins/*
	$(STRIP) $(gcw0_bundle)/scummvm
	./dists/gcw0/opk_make.sh -d $(gcw0_bundle) -o scummvm
