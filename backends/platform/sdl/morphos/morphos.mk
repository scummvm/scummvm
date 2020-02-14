# Special target to create an MorphOS snapshot installation.
# AmigaOS shell doesn't like indented comments.
morphosdist: $(EXECUTABLE) $(PLUGINS)
	mkdir -p $(MORPHOSPATH)
	mkdir -p $(MORPHOSPATH)/extras
	cp ${srcdir}/dists/amiga/scummvm_drawer.info $(MORPHOSPATH).info
	cp ${srcdir}/dists/amiga/scummvm.info $(MORPHOSPATH)/$(EXECUTABLE).info
# Copy mandatory installation files.
# Prepare README.md for AmigaGuide conversion.
	cat ${srcdir}/README.md | sed -f ${srcdir}/dists/amiga/convertRM.sed > README.conv
# AmigaOS AREXX has a problem when ${srcdir} is '.'.
# It will break with a "Program not found" error.
# Copy the script to cwd and, once it has finished, remove it.
	cp ${srcdir}/dists/amiga/RM2AG.rexx .
	rx RM2AG.rexx README.conv $(MORPHOSPATH)
	rm README.conv
	rm RM2AG.rexx
ifdef DIST_FILES_DOCS
	mkdir -p $(MORPHOSPATH)/doc
	cp -r $(srcdir)/doc/ $(MORPHOSPATH)
	cp $(DIST_FILES_DOCS) $(MORPHOSPATH)/doc/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(MORPHOSPATH)/extras/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(MORPHOSPATH)/extras/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(MORPHOSPATH)/extras/
endif
ifdef DIST_FILES_THEMES
	mkdir -p $(MORPHOSPATH)/themes
	cp $(DIST_FILES_THEMES) $(MORPHOSPATH)/themes/
endif
# Strip and copy engine plugins.
ifdef DYNAMIC_MODULES
	mkdir -p $(MORPHOSPATH)/plugins
	$(foreach plugin, $(PLUGINS), $(STRIP) $(plugin) -o $(MORPHOSPATH)/$(plugin);)
# Extract and install compiled-in shared libraries.
# Not every AmigaOS installation, especially vanilla ones,
# come with every mandatory shared library.
	mkdir -p $(MORPHOSPATH)/sobjs
	cp ${srcdir}/dists/amiga/Ext_Inst_so.rexx .
	rx Ext_Inst_so.rexx $(EXECUTABLE) $(MORPHOSPATH)
	rm Ext_Inst_so.rexx
endif
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOSPATH)/$(EXECUTABLE)
