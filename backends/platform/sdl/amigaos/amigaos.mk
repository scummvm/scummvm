# Special target to create an AmigaOS snapshot installation.
# AmigaOS shell doesn't like indented comments.
amigaosdist: $(EXECUTABLE)
	mkdir -p $(AMIGAOSPATH)
	mkdir -p $(AMIGAOSPATH)/extras
	cp ${srcdir}/dists/amiga/scummvm_drawer.info $(AMIGAOSPATH).info
	cp ${srcdir}/dists/amiga/scummvm.info $(AMIGAOSPATH)/$(EXECUTABLE).info
ifdef DIST_FILES_DOCS
	cp -r $(srcdir)/doc/ $(AMIGAOSPATH)
	cp $(DIST_FILES_DOCS) $(AMIGAOSPATH)/doc/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AMIGAOSPATH)/extras/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(AMIGAOSPATH)/extras/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(AMIGAOSPATH)/extras/
endif
# Copy shared library plugins, if available.
ifdef DYNAMIC_MODULES
	mkdir -p $(AMIGAOSPATH)/plugins
	cp $(PLUGINS) -o $(AMIGAOSPATH)/plugins/
endif
ifdef DIST_FILES_THEMES
	mkdir -p $(AMIGAOSPATH)/themes
	cp $(DIST_FILES_THEMES) $(AMIGAOSPATH)/themes/
endif
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOSPATH)/$(EXECUTABLE)
# Prepare README.md for AmigaGuide conversion.
	cat ${srcdir}/README.md | sed -f ${srcdir}/dists/amiga/convertRM.sed > README.conv
# AmigaOS AREXX has a problem when ${srcdir} is '.'.
# It will break with a "Program not found" error.
# We copy the script to cwd first and, once it has finished, remove it again.
	cp ${srcdir}/dists/amiga/RM2AG.rexx .
	rx RM2AG.rexx README.conv
	cp README.guide $(AMIGAOSPATH)
	rm RM2AG.rexx
	rm README.conv
	rm README.guide
