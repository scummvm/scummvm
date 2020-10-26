scummvm.nro: $(EXECUTABLE)
	mkdir -p ./switch_release/scummvm/data
	mkdir -p ./switch_release/scummvm/doc
	nacptool --create "ScummVM" "Cpasjuste" "$(VERSION)" ./switch_release/scummvm.nacp
	elf2nro $(EXECUTABLE) ./switch_release/scummvm/scummvm.nro --icon=$(srcdir)/dists/switch/icon.jpg --nacp=./switch_release/scummvm.nacp

switch_release: scummvm.nro
	rm -f ./switch_release/scummvm.nacp
	cp $(DIST_FILES_THEMES) ./switch_release/scummvm/data
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./switch_release/scummvm/data
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) ./switch_release/scummvm/data
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) ./switch_release/scummvm/data
endif
ifdef DIST_FILES_SHADERS
	mkdir -p ./switch_release/scummvm/data/shaders
	cp $(DIST_FILES_SHADERS) ./switch_release/scummvm/data/shaders
endif
	cp $(DIST_FILES_DOCS) ./switch_release/scummvm/doc/
	cp $(srcdir)/backends/platform/sdl/switch/README.SWITCH ./switch_release/scummvm/doc/

scummvm_switch.zip: switch_release
	cd ./switch_release && zip -r ../scummvm_switch.zip . && cd ..

.PHONY: scummvm.nro switch_release scummvm_switch.zip

