# Special target to create an AndroidSDL snapshot
androidsdl: 
	$(MKDIR) release
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) release
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS)  release
	$(CP) $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip release
	zip -j scummvm_1_10_0-git-appdata.zip release/*
	split -d -b 1000000 scummvm_1_10_0-git-appdata.zip scummvm_1_10_0-git-appdata.zip0
	$(RM) -r scummvm_1_10_0-git-appdata.zip

.PHONY: androidsdl
