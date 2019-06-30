scummvm.nro: $(EXECUTABLE)
	mkdir -p ./switch_release/scummvm/data
	mkdir -p ./switch_release/scummvm/doc
	nacptool --create "ScummVM" "Cpasjuste" "$(VERSION)" ./switch_release/scummvm.nacp
	elf2nro $(EXECUTABLE) ./switch_release/scummvm/scummvm.nro --icon=$(srcdir)/dists/switch/icon.jpg --nacp=./switch_release/scummvm.nacp

switch_release: scummvm.nro
	rm -f ./switch_release/scummvm.nacp
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip ./switch_release/scummvm/data
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_small.zip ./switch_release/scummvm/data
	cp $(DIST_FILES_THEMES) ./switch_release/scummvm/data
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./switch_release/scummvm/data
endif
	cp $(DIST_FILES_DOCS) ./switch_release/scummvm/doc/

scummvm_switch.zip: scummvm.nro
	rm -f ./switch_release/scummvm.nacp
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip ./switch_release/scummvm/data
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_small.zip ./switch_release/scummvm/data
	cp $(DIST_FILES_THEMES) ./switch_release/scummvm/data
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./switch_release/scummvm/data
endif
	cp $(DIST_FILES_DOCS) ./switch_release/scummvm/doc/
	cd ./switch_release && zip -r ../scummvm_switch.zip . && cd ..

.PHONY: scummvm.nro switch_release scummvm_switch.zip

