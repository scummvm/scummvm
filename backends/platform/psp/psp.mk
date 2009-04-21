all: psp_fixup_elf

psp_fixup_elf: scummvm.elf 
	$(PSPDEV)/bin/psp-fixup-imports $<

.PHONY: psp_fixup_elf
