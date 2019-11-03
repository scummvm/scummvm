ps3pkg: $(EXECUTABLE)
	$(STRIP) $(EXECUTABLE)
	sprxlinker $(EXECUTABLE)
	mkdir -p ps3pkg/USRDIR/data/
	mkdir -p ps3pkg/USRDIR/doc/
	mkdir -p ps3pkg/USRDIR/saves/
	make_self_npdrm "$(EXECUTABLE)" ps3pkg/USRDIR/EBOOT.BIN UP0001-SCUM12000_00-0000000000000000
	cp $(DIST_FILES_THEMES) ps3pkg/USRDIR/data/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ps3pkg/USRDIR/data/
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) ps3pkg/USRDIR/data/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) ps3pkg/USRDIR/data/
endif
ifdef DIST_PS3_EXTRA_FILES
	@cp -a $(DIST_PS3_EXTRA_FILES) ps3pkg/USRDIR/data/
endif
	cp $(DIST_FILES_DOCS) ps3pkg/USRDIR/doc/
	cp $(srcdir)/dists/ps3/readme-ps3.md ps3pkg/USRDIR/doc/
	cp $(srcdir)/dists/ps3/ICON0.PNG ps3pkg/
	cp $(srcdir)/dists/ps3/PIC1.PNG ps3pkg/
	sfo.py -f $(srcdir)/dists/ps3/sfo.xml ps3pkg/PARAM.SFO
	pkg.py --contentid UP0001-SCUM12000_00-0000000000000000 ps3pkg/ scummvm-ps3.pkg

ps3run: $(EXECUTABLE)
	$(STRIP) $(EXECUTABLE)
	sprxlinker $(EXECUTABLE)
	make_self $(EXECUTABLE) $(EXECUTABLE).self
	ps3load $(EXECUTABLE).self

.PHONY: ps3pkg ps3run
