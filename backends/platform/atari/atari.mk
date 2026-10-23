.PHONY: atarilitedist atarifulldist fbdist

DIST_FILES_PLATFORM := $(srcdir)/backends/platform/atari/readme.txt

# renames files in the current directory not fitting into the 8+3 limit the same way GEMDOS shortens them
RENAME_TO_8_3 := \
	for f in *.*; \
	do \
		n=$$(echo "$${f%.*}" | cut -c1-8).$$(echo "$${f\#\#*.}" | cut -c1-3); \
		[ "$$f" = "$$n" ] || mv "$$f" "$$n"; \
	done

LITE_DIR	:= scummvm-${VERSION}-atari-lite
LITE_DATA	:= ${LITE_DIR}/data
LITE_DOCS	:= ${LITE_DIR}/doc
LITE_THEMES	:=

FULL_DIR	:= scummvm-${VERSION}-atari-full
FULL_DATA	:= ${FULL_DIR}/data
FULL_DOCS	:= ${FULL_DIR}/doc
FULL_THEMES	:= ${FULL_DIR}/themes

FB_DIR		:= scummvm-${VERSION}-atari-m5475
FB_DATA		:= ${FB_DIR}/data
FB_DOCS		:= ${FB_DIR}/doc
FB_THEMES	:= ${FB_DIR}/themes

atarilitedist: $(EXECUTABLE) plugins
	$(RM_REC) ${LITE_DIR}
	$(MKDIR) ${LITE_DIR}

	$(CP) $(EXECUTABLE) ${LITE_DIR}
	$(NM) -C ${LITE_DIR}/$(EXECUTABLE) | grep -vF ' .L' | grep ' [TtWV] ' | $(CXXFILT) | sort -u > ${LITE_DIR}/scummvm.sym
	$(STRIP) -s ${LITE_DIR}/$(EXECUTABLE)

ifneq ($(PLUGINS),)
	$(MKDIR) ${LITE_DIR}/plugins
	$(CP) $(PLUGINS) ${LITE_DIR}/plugins
	$(STRIP) --strip-debug ${LITE_DIR}/plugins/*$(PLUGIN_SUFFIX)
	cd ${LITE_DIR}/plugins && $(RENAME_TO_8_3)
endif

	$(MKDIR) ${LITE_DOCS}
	$(CP) $(DIST_FILES_DOCS) ${LITE_DOCS}

	$(MKDIR) ${LITE_DATA}
	$(CP) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) ${LITE_DATA}

	# remove unused files
	$(RM) ${LITE_DATA}/helpdialog.zip
	$(RM) $(addsuffix .dat, $(addprefix ${LITE_DATA}/, achievements classicmacfonts encoding macgui))

	# rename remaining files still not fitting into the 8+3 limit (this has to be supported by the backend, too)
	cd ${LITE_DATA} && $(RENAME_TO_8_3)

	# readme.txt
	$(CP) $(DIST_FILES_PLATFORM) ${LITE_DIR}
	unix2dos ${LITE_DIR}/readme.txt

ifeq ($(CREATE_ZIP),y)
	$(RM) ../${LITE_DIR}.zip
	$(ZIP) -r -9 ../${LITE_DIR}.zip ${LITE_DIR}
endif

atarifulldist: $(EXECUTABLE) plugins
	$(RM_REC) ${FULL_DIR}
	$(MKDIR) ${FULL_DIR}

	$(CP) $(EXECUTABLE) ${FULL_DIR}
	$(NM) -C ${FULL_DIR}/$(EXECUTABLE) | grep -vF ' .L' | grep ' [TtWV] ' | $(CXXFILT) | sort -u > ${FULL_DIR}/scummvm.sym
	$(STRIP) -s ${FULL_DIR}/$(EXECUTABLE)

ifneq ($(PLUGINS),)
	$(MKDIR) ${FULL_DIR}/plugins
	$(CP) $(PLUGINS) ${FULL_DIR}/plugins
	$(STRIP) --strip-debug ${FULL_DIR}/plugins/*$(PLUGIN_SUFFIX)
	cd ${FULL_DIR}/plugins && $(RENAME_TO_8_3)
endif

	$(MKDIR) ${FULL_DOCS}
	$(CP) $(DIST_FILES_DOCS) ${FULL_DOCS}

	$(MKDIR) ${FULL_DATA}
	$(CP) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) ${FULL_DATA}

	# remove unused files
	$(RM) ${FULL_DATA}/helpdialog.zip
	$(RM) $(addsuffix .dat, $(addprefix ${FULL_DATA}/, achievements classicmacfonts encoding hadesch_translations macgui prince_translation))

	# rename remaining files still not fitting into the 8+3 limit (this has to be supported by the backend, too)
	cd ${FULL_DATA} && $(RENAME_TO_8_3)

	$(MKDIR) ${FULL_THEMES}
	$(CP) $(DIST_FILES_THEMES) ${FULL_THEMES}

	# remove unused files; absent gui-icons.dat massively speeds up startup time (it is used for the grid mode only)
	$(RM) ${FULL_THEMES}/gui-icons.dat ${FULL_THEMES}/shaders.dat

	# adjust to compression level zero for faster depacking
	cd ${FULL_THEMES} && \
		for f in *.zip; \
		do \
			unzip -q -d tmp "$$f" && $(RM) "$$f" && cd tmp && $(ZIP) -0 "../$$f" * && cd .. && $(RM_REC) tmp; \
		done
	cd ${FULL_THEMES} && $(RENAME_TO_8_3)

	# readme.txt
	$(CP) $(DIST_FILES_PLATFORM) ${FULL_DIR}
	unix2dos ${FULL_DIR}/readme.txt

ifeq ($(CREATE_ZIP),y)
	$(RM) ../${FULL_DIR}.zip
	$(ZIP) -r -9 ../${FULL_DIR}.zip ${FULL_DIR}
endif

fbdist: $(EXECUTABLE) plugins
	$(RM_REC) ${FB_DIR}
	$(MKDIR) ${FB_DIR}

	$(CP) $(EXECUTABLE) ${FB_DIR}
	$(STRIP) -s ${FB_DIR}/$(EXECUTABLE)

ifneq ($(PLUGINS),)
	$(MKDIR) ${FB_DIR}/plugins
	$(CP) $(PLUGINS) ${FB_DIR}/plugins
	$(STRIP) --strip-debug ${FB_DIR}/plugins/*$(PLUGIN_SUFFIX)
	cd ${FB_DIR}/plugins && $(RENAME_TO_8_3)
endif

	$(MKDIR) ${FB_DOCS}
	$(CP) $(DIST_FILES_DOCS) ${FB_DOCS}

	$(MKDIR) ${FB_DATA}
	$(CP) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) ${FB_DATA}

	# remove unused files
	$(RM) ${FB_DATA}/helpdialog.zip
	$(RM) $(addsuffix .dat, $(addprefix ${FB_DATA}/, achievements classicmacfonts encoding hadesch_translations macgui prince_translation))

	# rename remaining files still not fitting into the 8+3 limit (this has to be supported by the backend, too)
	cd ${FB_DATA} && $(RENAME_TO_8_3)

	$(MKDIR) ${FB_THEMES}
	$(CP) $(DIST_FILES_THEMES) ${FB_THEMES}

	# remove unused files
	$(RM) ${FB_THEMES}/shaders.dat

	# adjust to compression level zero for faster depacking
	cd ${FB_THEMES} && \
		for f in *.zip; \
		do \
			unzip -q -d tmp "$$f" && $(RM) "$$f" && cd tmp && $(ZIP) -0 "../$$f" * && cd .. && $(RM_REC) tmp; \
		done
	cd ${FB_THEMES} && $(RENAME_TO_8_3)

	# readme.txt
	$(CP) $(DIST_FILES_PLATFORM) ${FB_DIR}
	unix2dos ${FB_DIR}/readme.txt

ifeq ($(CREATE_ZIP),y)
	$(RM) ../${FB_DIR}.zip
	$(ZIP) -r -9 ../${FB_DIR}.zip ${FB_DIR}
endif
