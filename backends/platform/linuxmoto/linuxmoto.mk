# Special target to create a motoezx snapshot
motoezx: $(EXECUTABLE)
	$(MKDIR) motoezx/scummvm
	$(STRIP) $(EXECUTABLE) -o motoezx/scummvm/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) motoezx/scummvm/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip motoezx/scummvm/
	$(CP) $(srcdir)/dists/motoezx/* motoezx/scummvm/
	tar -C motoezx -cvzf motoezx/ScummVM.pkg scummvm
	$(RM) -r motoezx/scummvm

# Special target to create a motomagx snapshot
motomagx-mpkg: $(EXECUTABLE)
	$(MKDIR) motomagx/scummvm
	$(STRIP) $(EXECUTABLE) -o motomagx/scummvm/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) motomagx/scummvm/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip motomagx/scummvm/
	$(CP) $(srcdir)/dists/motomagx/mpkg/* motomagx/scummvm/
	tar -C motomagx -cvzf motomagx/ScummVM.mpkg scummvm
	$(RM) -r motomagx/scummvm

motomagx-mgx: $(EXECUTABLE)
	$(MKDIR) motomagx/scummvm
	$(STRIP) $(EXECUTABLE) -o motomagx/scummvm/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) motomagx/scummvm/
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip motomagx/scummvm/
	$(CP) $(srcdir)/dists/motomagx/mgx/* motomagx/scummvm/
	tar -C motomagx -cvf motomagx/ScummVM.mgx scummvm
	$(RM) -r motomagx/scummvm

motomagx-pep: $(EXECUTABLE)
	$(MKDIR) motomagx/pep
	$(CP) -r $(srcdir)/dists/motomagx/pep/* motomagx/pep
	$(RM) -r motomagx/pep/app/.svn
	$(STRIP) $(EXECUTABLE) -o motomagx/pep/app/$(EXECUTABLE)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) motomagx/pep/app
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip motomagx/pep/app
	tar -C motomagx/pep -czvf motomagx/ScummVM.pep app description.ini  scummvm_big_usr.png  scummvm_small_usr.png
	$(RM) -r motomagx/pep

.PHONY: motoezx motomagx-mpkg motomagx-mgx motomagx-pep
