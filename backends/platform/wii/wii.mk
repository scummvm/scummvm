WII_EXE_STRIPPED := scummvm_stripped$(EXEEXT)

all: $(WII_EXE_STRIPPED)

$(WII_EXE_STRIPPED): $(EXECUTABLE)
	$(STRIP) $< -o $@

clean: wiiclean

wiiclean:
	$(RM) $(WII_EXE_STRIPPED)

wiiload: $(WII_EXE_STRIPPED)
	$(DEVKITPPC)/bin/wiiload $<

geckoupload: $(WII_EXE_STRIPPED)
	$(DEVKITPPC)/bin/geckoupload $<

wiigdb:
	$(DEVKITPPC)/bin/powerpc-gekko-gdb -n $(EXECUTABLE)

wiidebug:
	$(DEVKITPPC)/bin/powerpc-gekko-gdb -n $(EXECUTABLE) -x $(srcdir)/backends/platform/wii/gdb.txt

# target to create a Wii snapshot
wiidist: all
	$(MKDIR) wiidist/scummvm
ifeq ($(GAMECUBE),1)
	$(DEVKITPPC)/bin/elf2dol $(EXECUTABLE) wiidist/scummvm/scummvm.dol
else
	$(STRIP) $(EXECUTABLE) -o wiidist/scummvm/boot.elf
	$(CP) $(srcdir)/dists/wii/icon.png wiidist/scummvm/
	sed "s/@REVISION@/$(VER_SVNREV)/;s/@TIMESTAMP@/`date +%Y%m%d%H%M%S`/" < $(srcdir)/dists/wii/meta.xml > wiidist/scummvm/meta.xml
endif
	sed 's/$$/\r/' < $(srcdir)/dists/wii/READMII > wiidist/scummvm/READMII.txt
	for i in $(DIST_FILES_DOCS); do sed 's/$$/\r/' < $$i > wiidist/scummvm/`basename $$i`.txt; done
	$(CP) $(DIST_FILES_THEMES) wiidist/scummvm/
ifneq ($(DIST_FILES_ENGINEDATA),)
	$(CP) $(DIST_FILES_ENGINEDATA) wiidist/scummvm/
endif
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip wiidist/scummvm/


.PHONY: wiiclean wiiload geckoupload wiigdb wiidebug wiidist

