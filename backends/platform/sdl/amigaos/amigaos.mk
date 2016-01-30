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
	cat ${srcdir}/README | sed -f ${srcdir}/dists/amiga/convertRM.sed > README.conv
	rx dists/amiga/RM2AG.rx README.conv
	cp ${srcdir}/README.guide $(AMIGAOSPATH)
	rm ${srcdir}/README.conv
	rm ${srcdir}/README.guide
	cp $(DIST_FILES_DOCS) $(AMIGAOSPATH)
