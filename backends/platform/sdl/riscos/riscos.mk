# Special target to create an RISC OS snapshot installation
riscosdist: $(EXECUTABLE)
	mkdir -p !ScummVM
	elf2aif $(EXECUTABLE) !ScummVM/$(EXECUTABLE),ff8
	cp ${srcdir}/dists/riscos/!Boot,feb !ScummVM/!Boot,feb
	cp ${srcdir}/dists/riscos/!Run,feb !ScummVM/!Run,feb
	cp ${srcdir}/dists/riscos/!Sprites,ff9 !ScummVM/!Sprites,ff9
	cp ${srcdir}/dists/riscos/!Sprites11,ff9 !ScummVM/!Sprites11,ff9
	mkdir -p !ScummVM/data
	cp $(DIST_FILES_THEMES) !ScummVM/data/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) !ScummVM/data/
endif
ifdef DYNAMIC_MODULES
	mkdir -p !ScummVM/plugins
	cp $(PLUGINS) !ScummVM/plugins/
endif
	mkdir -p !ScummVM/docs
	cp ${srcdir}/dists/riscos/!Help,feb !ScummVM/!Help,feb
	cp $(DIST_FILES_DOCS) !ScummVM/docs