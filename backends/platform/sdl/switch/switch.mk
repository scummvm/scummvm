scummvm.nro: $(EXECUTABLE)
	mkdir -p $(srcdir)/switch_release/scummvm/data
	mkdir -p $(srcdir)/switch_release/scummvm/doc
	nacptool --create "ScummVM" "Cpasjuste" "1.0" $(srcdir)/switch_release/scummvm.nacp
	elf2nro $(EXECUTABLE) $(srcdir)/switch_release/scummvm/scummvm.nro --icon=$(srcdir)/dists/switch/icon.jpg --nacp=$(srcdir)/switch_release/scummvm.nacp

scummvm_switch.zip: scummvm.nro
	rm -f $(srcdir)/switch_release/scummvm.nacp
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip $(srcdir)/switch_release/scummvm/data
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_small.zip $(srcdir)/switch_release/scummvm/data
	cp $(DIST_FILES_THEMES) $(srcdir)/switch_release/scummvm/data
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(srcdir)/switch_release/scummvm/data
endif
	cp $(DIST_FILES_DOCS) $(srcdir)/switch_release/scummvm/doc/
	cd $(srcdir)/switch_release && zip -r ../scummvm_switch.zip . && cd ..

.PHONY: scummvm.nro scummvm_switch.zip

