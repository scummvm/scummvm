WII_ELF=scummvm.elf
WII_DOL=scummvm.dol
WII_DISTPATH=$(srcdir)/dists/wii
all: $(WII_DOL)

$(WII_DOL): $(WII_ELF)
	$(DEVKITPPC)/bin/elf2dol $< $@

clean: wiiclean

wiiclean:
	$(RM) $(WII_DOL)

wiiload: $(WII_DOL)
	$(DEVKITPPC)/bin/wiiload $(WII_DOL) -d3

geckoupload: $(WII_DOL)
	$(DEVKITPPC)/bin/geckoupload $(WII_DOL)

wiigdb:
	$(DEVKITPPC)/bin/powerpc-gekko-gdb -n $(WII_ELF)

wiidebug:
	$(DEVKITPPC)/bin/powerpc-gekko-gdb -n $(WII_ELF) -x $(srcdir)/backends/platform/wii/gdb.txt

wiidist:
	$(MKDIR) dist/scummvm
	$(CP) $(WII_DOL) dist/scummvm/boot.dol
	$(CP) $(WII_DISTPATH)/meta.xml dist/scummvm/
	$(CP) $(WII_DISTPATH)/icon.png dist/scummvm/
	$(CP) $(WII_DISTPATH)/READMII dist/scummvm/
	$(CP) $(srcdir)/AUTHORS dist/scummvm/
	$(CP) $(srcdir)/COPYING dist/scummvm/
	$(CP) $(srcdir)/COPYING.LGPL dist/scummvm/
	$(CP) $(srcdir)/COPYRIGHT dist/scummvm/
	$(CP) $(srcdir)/NEWS dist/scummvm/
	$(CP) $(srcdir)/README dist/scummvm/
	$(CP) $(srcdir)/dists/pred.dic dist/scummvm/
	$(CP) $(DIST_FILES_THEMES) dist/scummvm/
ifneq ($(DIST_FILES_ENGINEDATA),)
	$(CP) $(DIST_FILES_ENGINEDATA) dist/scummvm/
endif

gcdist:
	$(MKDIR) dist/scummvm
	$(CP) $(WII_DOL) dist/scummvm/
	$(CP) $(WII_DISTPATH)/READMII dist/scummvm/
	$(CP) $(srcdir)/AUTHORS dist/scummvm/
	$(CP) $(srcdir)/COPYING dist/scummvm/
	$(CP) $(srcdir)/COPYING.LGPL dist/scummvm/
	$(CP) $(srcdir)/COPYRIGHT dist/scummvm/
	$(CP) $(srcdir)/NEWS dist/scummvm/
	$(CP) $(srcdir)/README dist/scummvm/
	$(CP) $(srcdir)/dists/pred.dic dist/scummvm/
	$(CP) $(DIST_FILES_THEMES) dist/scummvm/
ifneq ($(DIST_FILES_ENGINEDATA),)
	$(CP) $(DIST_FILES_ENGINEDATA) dist/scummvm/
endif

.PHONY: wiiclean wiiload geckoupload wiigdb wiidebug wiidist gcdist

