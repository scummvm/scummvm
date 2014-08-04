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
