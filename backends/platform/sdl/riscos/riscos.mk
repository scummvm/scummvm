ifeq ($(shell echo a | iconv --to-code=RISCOS-LATIN1//TRANSLIT >/dev/null 2>&1; echo $$?),0)
ENCODING=RISCOS-LATIN1//TRANSLIT
else
ENCODING=ISO-8859-1//TRANSLIT
endif

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
	@$(foreach file, $(DIST_FILES_DOCS) $(srcdir)/doc/QuickStart, echo '   ' ICONV '  ' !ScummVM/docs/$(notdir $(file)),fff;iconv --to-code=$(ENCODING) $(file) > !ScummVM/docs/$(notdir $(file)),fff;)
	@$(foreach lang, $(DIST_FILES_DOCS_languages), mkdir -p !ScummVM/docs/$(lang); $(foreach file, $(DIST_FILES_DOCS_$(lang)), echo '   ' ICONV '  ' !ScummVM/docs/$(lang)/$(notdir $(file)),fff;iconv --to-code=$(ENCODING) $(file) > !ScummVM/docs/$(lang)/$(notdir $(file)),fff;))
