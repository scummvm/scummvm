PSP_EXE_STRIPPED := scummvm_stripped$(EXEEXT)
PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_SFO = param.sfo
PSP_EBOOT_TITLE = ScummVM-PSP
DATE = $(shell date +%Y%m%d)
DIST_EXECUTABLES=$(PSP_EBOOT) $(PLUGINS)
DIST_FILES_PLATFORM=$(srcdir)/backends/platform/psp/README.PSP

MKSFO = mksfoex -d MEMSIZE=1
PACK_PBP = pack-pbp

all: pack_pbp

clean: psp_clean

$(PSP_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

$(PSP_EBOOT_SFO): $(EXECUTABLE)
	$(MKSFO) '$(PSP_EBOOT_TITLE) r$(VER_REV) ($(DATE))' $@

psp_clean:
	$(RM) $(PSP_EXE_STRIPPED) $(PSP_EBOOT) $(PSP_EBOOT_SFO)

psp_fixup_elf: $(PSP_EXE_STRIPPED)
	psp-fixup-imports $<

pack_pbp: psp_fixup_elf $(PSP_EBOOT_SFO)
	$(PACK_PBP) $(PSP_EBOOT) \
	$(PSP_EBOOT_SFO) \
	$(srcdir)/backends/platform/psp/icon0.png \
	NULL \
	$(srcdir)/backends/platform/psp/pic0.png \
	$(srcdir)/backends/platform/psp/pic1.png \
	NULL \
	$(PSP_EXE_STRIPPED) \
	NULL

psp_release: pack_pbp $(PLUGINS)
	rm -rf ./psp_release
	mkdir -p ./psp_release/scummvm/doc
	cp $(PSP_EBOOT) ./psp_release/scummvm/
	cp $(DIST_FILES_THEMES) ./psp_release/scummvm/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./psp_release/scummvm/
endif
ifdef DIST_FILES_SOUNDFONTS
	cp $(DIST_FILES_SOUNDFONTS) ./psp_release/scummvm/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) ./psp_release/scummvm/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) ./psp_release/scummvm/
else
	cp $(srcdir)/backends/platform/psp/kbd.zip ./psp_release/scummvm/
endif
ifdef DYNAMIC_MODULES
	mkdir -p ./psp_release/scummvm/plugins	
	cp $(PLUGINS) ./psp_release/scummvm/plugins/
endif
	cp $(DIST_FILES_DOCS) ./psp_release/scummvm/doc/
	cp $(srcdir)/backends/platform/psp/README.PSP ./psp_release/scummvm/doc/

scummvm_psp.zip: psp_release
	cd ./psp_release && zip -r ../scummvm_psp.zip . && cd ..

.PHONY: psp_fixup_elf pack_pbp psp_release scummvm_psp.zip
