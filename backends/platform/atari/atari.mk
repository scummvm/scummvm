.PHONY: atarilitedist atarifulldist fbdist

DIST_FILES_PLATFORM := $(srcdir)/backends/platform/atari/readme.txt
ifneq (${BACKEND},sdl)
DIST_FILES_PLATFORM += $(srcdir)/backends/platform/atari/patches
endif

LITE_DIR	:= scummvm-${VERSION}-atari-lite
LITE_DATA	:= ${LITE_DIR}/data
LITE_DOCS	:= ${LITE_DIR}/doc
LITE_THEMES	:=

FULL_DIR	:= scummvm-${VERSION}-atari-full
FULL_DATA	:= ${FULL_DIR}/data
FULL_DOCS	:= ${FULL_DIR}/doc
FULL_THEMES	:= ${FULL_DIR}/themes

FB_DIR		:= scummvm-${VERSION}-firebee
FB_DATA		:= ${FB_DIR}
FB_DOCS		:= ${FB_DIR}/doc
FB_THEMES	:= ${FB_DIR}

atarilitedist: $(EXECUTABLE)
	$(RM_REC) ${LITE_DIR}
	$(MKDIR) ${LITE_DIR}

	$(CP) $(EXECUTABLE) ${LITE_DIR}
	$(NM) -C ${LITE_DIR}/$(EXECUTABLE) | grep -vF ' .L' | grep ' [TtWV] ' | $(CXXFILT) | sort -u > ${LITE_DIR}/scummvm.sym
	$(STRIP) -s ${LITE_DIR}/$(EXECUTABLE)

	$(MKDIR) ${LITE_DOCS}
	$(CP) $(DIST_FILES_DOCS) ${LITE_DOCS}

	$(MKDIR) ${LITE_DATA}
	$(CP) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) ${LITE_DATA}

	# remove unused files
	$(RM) ${LITE_DATA}/helpdialog.zip
	$(RM) $(addsuffix .dat, $(addprefix ${LITE_DATA}/, achievements classicmacfonts encoding macgui))

	# rename remaining files still not fitting into the 8+3 limit (this has to be supported by the backend, too)
	! [ -f ${LITE_DATA}/supernova.dat ] || mv ${LITE_DATA}/supernova.dat ${LITE_DATA}/supernov.dat
	! [ -f ${LITE_DATA}/teenagent.dat ] || mv ${LITE_DATA}/teenagent.dat ${LITE_DATA}/teenagen.dat

	# readme.txt
	$(CP) -r $(DIST_FILES_PLATFORM) ${LITE_DIR}
	unix2dos ${LITE_DIR}/readme.txt

ifeq ($(CREATE_ZIP),y)
	$(RM) ../${LITE_DIR}.zip
	$(ZIP) -r -9 ../${LITE_DIR}.zip ${LITE_DIR}
endif

atarifulldist: $(EXECUTABLE)
	$(RM_REC) ${FULL_DIR}
	$(MKDIR) ${FULL_DIR}

	$(CP) $(EXECUTABLE) ${FULL_DIR}
	$(NM) -C ${FULL_DIR}/$(EXECUTABLE) | grep -vF ' .L' | grep ' [TtWV] ' | $(CXXFILT) | sort -u > ${FULL_DIR}/scummvm.sym
	$(STRIP) -s ${FULL_DIR}/$(EXECUTABLE)

	$(MKDIR) ${FULL_DOCS}
	$(CP) $(DIST_FILES_DOCS) ${FULL_DOCS}

	$(MKDIR) ${FULL_DATA}
	$(CP) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) ${FULL_DATA}

	# remove unused files
	$(RM) ${FULL_DATA}/helpdialog.zip
	$(RM) $(addsuffix .dat, $(addprefix ${FULL_DATA}/, achievements classicmacfonts encoding hadesch_translations macgui prince_translation))

	# rename remaining files still not fitting into the 8+3 limit (this has to be supported by the backend, too)
	! [ -f ${FULL_DATA}/cryomni3d.dat ] || mv ${FULL_DATA}/cryomni3d.dat ${FULL_DATA}/cryomni3.dat
	! [ -f ${FULL_DATA}/neverhood.dat ] || mv ${FULL_DATA}/neverhood.dat ${FULL_DATA}/neverhoo.dat
	! [ -f ${FULL_DATA}/supernova.dat ] || mv ${FULL_DATA}/supernova.dat ${FULL_DATA}/supernov.dat
	! [ -f ${FULL_DATA}/teenagent.dat ] || mv ${FULL_DATA}/teenagent.dat ${FULL_DATA}/teenagen.dat

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

	# readme.txt
	$(CP) -r $(DIST_FILES_PLATFORM) ${FULL_DIR}
	unix2dos ${FULL_DIR}/readme.txt

ifeq ($(CREATE_ZIP),y)
	$(RM) ../${FULL_DIR}.zip
	$(ZIP) -r -9 ../${FULL_DIR}.zip ${FULL_DIR}
endif

fbdist: $(EXECUTABLE)
	$(RM_REC) ${FB_DIR}
	$(MKDIR) ${FB_DIR}

	$(CP) $(EXECUTABLE) ${FB_DIR}
	$(STRIP) -s ${FB_DIR}/$(EXECUTABLE)

	$(MKDIR) ${FB_DOCS}
	$(CP) $(DIST_FILES_DOCS) ${FB_DOCS}

	$(MKDIR) ${FB_DATA}
	$(CP) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_ENGINEDATA_BIG) ${FB_DATA}

	# remove unused files
	$(RM) ${FB_DATA}/helpdialog.zip
	$(RM) $(addsuffix .dat, $(addprefix ${FB_DATA}/, achievements classicmacfonts encoding hadesch_translations macgui prince_translation))

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

	# readme.txt
	$(CP) -r $(DIST_FILES_PLATFORM) ${FB_DIR}
	unix2dos ${FB_DIR}/readme.txt

ifeq ($(CREATE_ZIP),y)
	$(RM) ../${FB_DIR}.zip
	$(ZIP) -r -9 ../${FB_DIR}.zip ${FB_DIR}
endif
