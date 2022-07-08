ifeq ($(shell echo a | iconv --to-code=RISCOS-LATIN1//IGNORE//TRANSLIT >/dev/null 2>&1; echo $$?),0)
ENCODING=RISCOS-LATIN1//IGNORE//TRANSLIT
else
ENCODING=ISO-8859-1//IGNORE//TRANSLIT
endif

BASE_APP_NAME ?= !ScummVM
APP_NAME ?= $(BASE_APP_NAME)

DIST_FILES_DOCS_plaintext := $(filter-out $(DIST_FILES_MANUAL),$(DIST_FILES_DOCS))

# Special target to create an RISC OS snapshot installation
riscosdist: all
	mkdir -p $(APP_NAME)
	elf2aif $(EXECUTABLE) $(APP_NAME)/scummvm,ff8
	cp ${srcdir}/dists/riscos/!Help,feb $(APP_NAME)/!Help,feb
ifdef MAKERUN
	$(MAKERUN) $(APP_NAME)/scummvm,ff8 ${srcdir}/dists/riscos/!Run,feb $(APP_NAME)/!Run,feb
else
	cp ${srcdir}/dists/riscos/!Run,feb $(APP_NAME)/!Run,feb
	sed -i -e "s/WIMPSLOT/WimpSlot -min `du -k $(APP_NAME)/scummvm,ff8 | cut -f1`K/g" $(APP_NAME)/!Run,feb
endif
ifeq ($(APP_NAME),$(BASE_APP_NAME))
	cp ${srcdir}/dists/riscos/!Boot,feb $(APP_NAME)/!Boot,feb
	cp ${srcdir}/dists/riscos/!Sprites,ff9 $(APP_NAME)/!Sprites,ff9
	cp ${srcdir}/dists/riscos/!Sprites11,ff9 $(APP_NAME)/!Sprites11,ff9
else
	cp ${srcdir}/dists/riscos/$(APP_NAME)/!Boot,feb $(APP_NAME)/!Boot,feb
	cp ${srcdir}/dists/riscos/$(APP_NAME)/!Sprites,ff9 $(APP_NAME)/!Sprites,ff9
	cp ${srcdir}/dists/riscos/$(APP_NAME)/!Sprites11,ff9 $(APP_NAME)/!Sprites11,ff9
endif
	mkdir -p $(BASE_APP_NAME)/data
	cp $(DIST_FILES_THEMES) $(BASE_APP_NAME)/data/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(BASE_APP_NAME)/data/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(BASE_APP_NAME)/data/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(BASE_APP_NAME)/data/
endif
ifdef DYNAMIC_MODULES
	mkdir -p $(APP_NAME)/plugins
	cp $(PLUGINS) $(APP_NAME)/plugins/
endif
ifeq ($(APP_NAME),$(BASE_APP_NAME))
	mkdir -p $(APP_NAME)/docs
ifdef TOKENIZE
	$(TOKENIZE) ${srcdir}/dists/riscos/FindHelp,fd1 -out $(APP_NAME)/FindHelp,ffb
endif
ifdef DIST_FILES_MANUAL
ifneq ("$(wildcard $(DIST_FILES_MANUAL))","")
	cp $(DIST_FILES_MANUAL) $(APP_NAME)/docs/ScummVM,adf
endif
endif
	@$(foreach file, $(DIST_FILES_DOCS_plaintext) $(srcdir)/doc/QuickStart, echo '   ' ICONV '  ' $(APP_NAME)/docs/$(notdir $(file)),fff;iconv --to-code=$(ENCODING) $(file) > $(APP_NAME)/docs/$(notdir $(file)),fff;)
	@$(foreach lang, $(DIST_FILES_DOCS_languages), mkdir -p $(APP_NAME)/docs/$(lang); $(foreach file, $(DIST_FILES_DOCS_$(lang)), echo '   ' ICONV '  ' $(APP_NAME)/docs/$(lang)/$(notdir $(file)),fff;iconv --from-code=UTF-8 --to-code=$(ENCODING) $(file) > $(APP_NAME)/docs/$(lang)/$(notdir $(file)),fff;))
endif

clean: riscosclean

riscosclean:
	$(RM_REC) $(APP_NAME)
	$(RM_REC) $(BASE_APP_NAME)

.PHONY: riscosdist riscosclean
