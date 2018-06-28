# Special target to create an RISC OS snapshot installation
riscosdist: scummvm$(EXEEXT)
	mkdir -p !ScummVM
	elf2aif scummvm$(EXEEXT) !ScummVM/scummvm,ff8
	cp ${srcdir}/dists/riscos/!Boot,feb !ScummVM/!Boot,feb
	cp ${srcdir}/dists/riscos/!Run,feb !ScummVM/!Run,feb
	cp ${srcdir}/dists/riscos/!Sprites,ff9 !ScummVM/!Sprites,ff9
	cp ${srcdir}/dists/riscos/!Sprites11,ff9 !ScummVM/!Sprites11,ff9
	sed -i -e "s/|WimpSlot/WimpSlot -min `du -k !ScummVM/scummvm,ff8 | cut -f1`K/g" !ScummVM/!Run,feb
	mkdir -p !ScummVM/data
	cp $(DIST_FILES_THEMES) !ScummVM/data/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) !ScummVM/data/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) !ScummVM/data/
endif
ifdef DYNAMIC_MODULES
	mkdir -p !ScummVM/plugins
	cp $(PLUGINS) !ScummVM/plugins/
endif
	mkdir -p !ScummVM/docs
	cp ${srcdir}/dists/riscos/!Help,feb !ScummVM/!Help,feb
ifdef TOKENIZE
	$(TOKENIZE) dists/riscos/FindHelp,fd1 -out !ScummVM/FindHelp,ffb
endif
	cp $(DIST_FILES_DOCS) !ScummVM/docs
	cp -r ${srcdir}/doc/* !ScummVM/docs
