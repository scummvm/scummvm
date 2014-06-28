aos4dist: $(EXECUTABLE)
	mkdir -p $(AOS4PATH)
	mkdir -p $(AOS4PATH)/themes
	mkdir -p $(AOS4PATH)/extras
	$(STRIP) $(EXECUTABLE) -o $(AOS4PATH)/$(EXECUTABLE)
	cp ${srcdir}/icons/scummvm.info $(AOS4PATH)/$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(AOS4PATH)/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AOS4PATH)/extras/
endif
	cp $(DIST_FILES_DOCS) $(AOS4PATH)
