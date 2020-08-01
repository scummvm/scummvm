# Special target to create an MorphOS snapshot installation.
# AmigaOS shell doesn't like indented comments.
morphosdist: $(EXECUTABLE) $(PLUGINS)
	mkdir -p $(MORPHOSPATH)extras
	cp ${srcdir}/dists/amiga/scummvm.info $(MORPHOSPATH)/$(EXECUTABLE).info
ifdef DIST_FILES_DOCS
	mkdir -p $(MORPHOSPATH)/doc
	cp -r $(srcdir)/doc/ $(MORPHOSPATH)
	cp $(DIST_FILES_DOCS) $(MORPHOSPATH)doc/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(MORPHOSPATH)extras/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(MORPHOSPATH)extras/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(MORPHOSPATH)extras/
endif
ifdef DIST_FILES_THEMES
	mkdir -p $(MORPHOSPATH)themes
	cp $(DIST_FILES_THEMES) $(MORPHOSPATH)themes/
endif
# Strip 
	$(STRIP) $(EXECUTABLE) -o $(MORPHOSPATH)$(EXECUTABLE)
