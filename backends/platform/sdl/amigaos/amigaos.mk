# Special target to create an AmigaOS snapshot installation
amigaosdist: $(EXECUTABLE)
	mkdir -p $(AMIGAOSPATH)
	mkdir -p $(AMIGAOSPATH)/themes
	mkdir -p $(AMIGAOSPATH)/extras
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOSPATH)/$(EXECUTABLE)
	cp ${srcdir}/icons/scummvm_drawer.info $(AMIGAOSPATH).info
	cp ${srcdir}/icons/scummvm.info $(AMIGAOSPATH)/$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(AMIGAOSPATH)/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AMIGAOSPATH)/extras/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(AMIGAOSPATH)/extras/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(AMIGAOSPATH)/extras/
endif
	cat ${srcdir}/README.md | sed -f ${srcdir}/dists/amiga/convertRM.sed > README.conv
# AmigaOS's shell is not happy with indented comments, thus don't do it.
# AREXX seems to have problems when ${srcdir} is '.'. It will break with a
# "Program not found" error. Therefore we copy the script to the cwd and
# remove it again, once it has finished.
	cp ${srcdir}/dists/amiga/RM2AG.rexx .
	rx RM2AG.rexx README.conv
	cp README.guide $(AMIGAOSPATH)
	rm RM2AG.rexx
	rm README.conv
	rm README.guide
	cp $(DIST_FILES_DOCS) $(AMIGAOSPATH)
