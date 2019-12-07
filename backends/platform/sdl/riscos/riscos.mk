ifeq ($(shell echo a | iconv --to-code=RISCOS-LATIN1//IGNORE//TRANSLIT >/dev/null 2>&1; echo $$?),0)
ENCODING=RISCOS-LATIN1//IGNORE//TRANSLIT
else
ENCODING=ISO-8859-1//IGNORE//TRANSLIT
endif

APP_NAME=!ResidualVM

# Special target to create an RISC OS snapshot installation
riscosdist: all
	mkdir -p $(APP_NAME)
	elf2aif $(EXECUTABLE) $(APP_NAME)/residualvm,ff8
	cp ${srcdir}/dists/riscos/!Boot,feb $(APP_NAME)/!Boot,feb
	cp ${srcdir}/dists/riscos/!Run,feb $(APP_NAME)/!Run,feb
	cp ${srcdir}/dists/riscos/!Sprites,ff9 $(APP_NAME)/!Sprites,ff9
	sed -i -e "s/|WimpSlot/WimpSlot -min `du -k $(APP_NAME)/residualvm,ff8 | cut -f1`K/g" $(APP_NAME)/!Run,feb
	mkdir -p $(APP_NAME)/data
	cp $(DIST_FILES_THEMES) $(APP_NAME)/data/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(APP_NAME)/data/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(APP_NAME)/data/
endif
	# ResidualVM specific
ifdef USE_OPENGL_SHADERS
	mkdir -p $(APP_NAME)/data/shaders
	cp $(DIST_FILES_SHADERS) $(APP_NAME)/data/shaders/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(APP_NAME)/data/
endif
ifdef DYNAMIC_MODULES
	mkdir -p $(APP_NAME)/plugins
	cp $(PLUGINS) $(APP_NAME)/plugins/
endif
	mkdir -p $(APP_NAME)/docs
	cp ${srcdir}/dists/riscos/!Help,feb $(APP_NAME)/!Help,feb
	@$(foreach file, $(DIST_FILES_DOCS) $(srcdir)/doc/QuickStart, echo '   ' ICONV '  ' $(APP_NAME)/docs/$(notdir $(file)),fff;iconv --to-code=$(ENCODING) $(file) > $(APP_NAME)/docs/$(notdir $(file)),fff;)

clean: riscosclean

riscosclean:
	$(RM_REC) $(APP_NAME)

.PHONY: riscosdist riscosclean
