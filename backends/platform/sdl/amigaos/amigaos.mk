# Special target to create an AmigaOS snapshot installation
amigaosdist: $(EXECUTABLE)
	mkdir -p $(AMIGAOSPATH)
	mkdir -p $(AMIGAOSPATH)/themes
	mkdir -p $(AMIGAOSPATH)/extras
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOSPATH)/$(EXECUTABLE)
	cp ${srcdir}/icons/residualvm_drawer.info $(AMIGAOSPATH).info
	cp ${srcdir}/icons/residualvm.info $(AMIGAOSPATH)/$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(AMIGAOSPATH)/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AMIGAOSPATH)/extras/
endif
	cp $(DIST_FILES_DOCS) $(AMIGAOSPATH)

# Special target to cross create an AmigaOS snapshot installation
amigaoscross: $(EXECUTABLE)
	mkdir -p ResidualVM
	mkdir -p ResidualVM/themes
	mkdir -p ResidualVM/extras
	cp $(EXECUTABLE) ResidualVM/ResidualVM
	cp ${srcdir}/icons/residualvm_drawer.info ResidualVM.info
	cp ${srcdir}/icons/residualvm.info ResidualVM/ResidualVM.info
	cp $(DIST_FILES_THEMES) ResidualVM/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ResidualVM/extras/
endif
	cp $(srcdir)/AUTHORS ResidualVM/AUTHORS.txt
	cp $(srcdir)/COPYING ResidualVM/COPYING.txt
	cp $(srcdir)/COPYING.BSD ResidualVM/COPYING.BSD.txt
	cp $(srcdir)/COPYING.LGPL ResidualVM/COPYING.LGPL.txt
	cp $(srcdir)/COPYING.FREEFONT ResidualVM/COPYING.FREEFONT.txt
	cp $(srcdir)/COPYING.ISC ResidualVM/COPYING.ISC.txt
	cp $(srcdir)/COPYING.LUA ResidualVM/COPYING.LUA.txt
	cp $(srcdir)/COPYING.MIT ResidualVM/COPYING.MIT.txt
	cp $(srcdir)/COPYING.TINYGL ResidualVM/COPYING.TINYGL.txt
	cp $(srcdir)/COPYRIGHT ResidualVM/COPYRIGHT.txt
	cp $(srcdir)/KNOWN_BUGS ResidualVM/KNOWN_BUGS.txt
	cp $(srcdir)/NEWS ResidualVM/NEWS.txt
	cp $(srcdir)/doc/QuickStart ResidualVM/QuickStart.txt
	cp $(srcdir)/README.md ResidualVM/README.txt
	zip -r residualvm-amigaos4.zip ResidualVM ResidualVM.info
