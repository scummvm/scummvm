all: pack_pbp

PSP_EXE := scummvm$(EXEEXT)
PSP_EXE_STRIPPED := scummvm_stripped$(EXEEXT)
PACK_PBP = pack-pbp

$(PSP_EXE_STRIPPED): $(PSP_EXE)
	$(STRIP) $< -o $@

psp_fixup_elf: $(PSP_EXE_STRIPPED)
	$(PSPDEV)/bin/psp-fixup-imports $<

pack_pbp: psp_fixup_elf
	$(PACK_PBP) EBOOT.PBP \
	$(srcdir)/backends/platform/psp/param.sfo \
	$(srcdir)/backends/platform/psp/icon0.png \
	NULL \
	$(srcdir)/backends/platform/psp/pic0.png \
	$(srcdir)/backends/platform/psp/pic1.png \
	NULL \
	$(PSP_EXE_STRIPPED) \
	NULL

.PHONY: psp_fixup_elf pack_pbp
