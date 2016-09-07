DATE := $(shell date +%y-%m-%d)

psp2pkg: $(EXECUTABLE)
	$(STRIP) -g $(EXECUTABLE)
	mkdir -p psp2pkg/scummvm/
	vita-elf-create $(EXECUTABLE) $(EXECUTABLE).velf
	vita-make-fself -s $(EXECUTABLE).velf eboot.bin
	vita-mksfoex -s TITLE_ID=VSCU00001 "$(EXECUTABLE)" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin \
		--add $(srcdir)/dists/psp2/icon0.png=sce_sys/icon0.png \
		--add $(srcdir)/dists/psp2/pic0.png=sce_sys/pic0.png \
		psp2pkg/$(EXECUTABLE).vpk
	mkdir -p psp2pkg/scummvm/data/
	mkdir -p psp2pkg/scummvm/doc/
	mkdir -p psp2pkg/scummvm/saves/
	cp $(DIST_FILES_THEMES) psp2pkg/scummvm/data/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) psp2pkg/scummvm/data/
endif
	cp $(DIST_FILES_DOCS) psp2pkg/scummvm/doc/
	cp $(srcdir)/dists/psp2/readme-psp2.md psp2pkg/scummvm/doc/
	cp $(srcdir)/dists/psp2/readme-psp2.md psp2pkg/
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip psp2pkg/scummvm/data/
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_small.zip psp2pkg/scummvm/data/
	cd psp2pkg && 7z a -tzip ../$(EXECUTABLE)-psp2_$(DATE).zip -r . && cd ..

.PHONY: psp2pkg
