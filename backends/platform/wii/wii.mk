WII_EXE := scummvm$(EXEEXT)
WII_EXE_STRIPPED := scummvm_stripped$(EXEEXT)

$(WII_EXE_STRIPPED): $(WII_EXE)
	$(STRIP) $< -o $@

clean: wiiclean

wiiclean:
	$(RM) $(WII_EXE_STRIPPED)

wiiload: $(WII_EXE_STRIPPED)
	$(DEVKITPPC)/bin/wiiload $<

geckoupload: $(WII_EXE_STRIPPED)
	$(DEVKITPPC)/bin/geckoupload $<

wiigdb:
	$(DEVKITPPC)/bin/powerpc-gekko-gdb -n $(WII_EXE)

wiidebug:
	$(DEVKITPPC)/bin/powerpc-gekko-gdb -n $(WII_EXE) -x $(srcdir)/backends/platform/wii/gdb.txt

.PHONY: wiiclean wiiload geckoupload wiigdb wiidebug

