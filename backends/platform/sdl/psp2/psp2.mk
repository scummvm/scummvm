PSP2_EXE_STRIPPED := scummvm_stripped$(EXEEXT)

$(PSP2_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) --strip-debug $< -o $@

psp2vpk: $(PSP2_EXE_STRIPPED)
	rm -rf psp2pkg
	rm -f $(EXECUTABLE).vpk
	mkdir -p psp2pkg/sce_sys/livearea/contents
	mkdir -p psp2pkg/data/
	mkdir -p psp2pkg/doc/
	vita-elf-create $(PSP2_EXE_STRIPPED) $(EXECUTABLE).velf
	vita-make-fself -s -c $(EXECUTABLE).velf psp2pkg/eboot.bin
	vita-mksfoex -s TITLE_ID=VSCU00001 -d ATTRIBUTE2=12 "$(EXECUTABLE)" psp2pkg/sce_sys/param.sfo
	cp $(srcdir)/dists/psp2/icon0.png psp2pkg/sce_sys/
	cp $(srcdir)/dists/psp2/template.xml psp2pkg/sce_sys/livearea/contents/
	cp $(srcdir)/dists/psp2/bg.png psp2pkg/sce_sys/livearea/contents/
	cp $(srcdir)/dists/psp2/startup.png psp2pkg/sce_sys/livearea/contents/
	cp $(DIST_FILES_THEMES) psp2pkg/data/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) psp2pkg/data/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) psp2pkg/data/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) psp2pkg/data/
endif
	cp $(DIST_FILES_DOCS) psp2pkg/doc/
	cp $(srcdir)/dists/psp2/readme-psp2.md psp2pkg/doc/
	cd psp2pkg && zip -r ../$(EXECUTABLE).vpk . && cd ..

.PHONY: psp2vpk
