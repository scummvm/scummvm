# Special target to create an AmigaOS snapshot installation
amigaos4dist: $(EXECUTABLE)
	mkdir -p $(AMIGAOS4PATH)
	mkdir -p $(AMIGAOS4PATH)/themes
	mkdir -p $(AMIGAOS4PATH)/extras
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOS4PATH)/$(EXECUTABLE)
	cp ${srcdir}/icons/residualvm.info $(AMIGAOS4PATH)/$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(AMIGAOS4PATH)/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AMIGAOS4PATH)/extras/
endif
	cp $(DIST_FILES_DOCS) $(AMIGAOS4PATH)

# Special target to cross create an AmigaOS snapshot installation
amigaos4cross: $(EXECUTABLE)
	mkdir -p ResidualVM
	mkdir -p ResidualVM/themes
	mkdir -p ResidualVM/extras
	cp $(EXECUTABLE) ResidualVM/$(EXECUTABLE)
	cp ${srcdir}/icons/residualvm.info ResidualVM/$(EXECUTABLE).info
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
	cp $(srcdir)/README ResidualVM/README.txt
	lha a residualvm-amigaos4.lha ResidualVM
