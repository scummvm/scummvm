# Special target to create an MorphOS snapshot installation
morphosdist: $(EXECUTABLE)
	mkdir -p $(MORPHOSPATH)
	mkdir -p $(MORPHOSPATH)themes
	mkdir -p $(MORPHOSPATH)extras
	$(STRIP) $(EXECUTABLE) -o $(MORPHOSPATH)$(EXECUTABLE)
	cp ${srcdir}/icons/scummvm_drawer.info $(MORPHOSPATH).info
	cp ${srcdir}/icons/scummvm.info $(MORPHOSPATH)$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(MORPHOSPATH)themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(MORPHOSPATH)extras/
endif
	cat ${srcdir}/README | sed -f ${srcdir}/dists/amiga/convertRM.sed > README.conv
# MorphOS's shell is not happy with indented comments, thus don't do it.
# AREXX seems to have problems when ${srcdir} is '.'. It will break with a
# "Program not found" error. Therefore we copy the script to the cwd and
# remove it again, once it has finished.
	cp ${srcdir}/dists/amiga/RM2AG.rx .
	rx RM2AG.rx README.conv
	cp README.guide $(MORPHOSPATH)
	rm RM2AG.rx
	rm README.conv
	rm README.guide
	cp $(DIST_FILES_DOCS) $(MORPHOSPATH)
