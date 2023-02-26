bundle = zip-root

all: scummvm.kos $(EXECUTABLE)

scummvm.kos: $(srcdir)/backends/platform/sdl/kolibrios/wrapper-main.c
	+$(QUIET_CC)$(CXX) -I$(KOS32_SDK_DIR)/sources/newlib/libc/include -specs=$(srcdir)/backends/platform/sdl/kolibrios/kolibrios.spec -x c -o $@.coff $<
	+$(QUIET)$(KOS32_AUTOBUILD)/tools/win32/bin/kos32-objcopy $@.coff -O binary $@

$(bundle): all
	$(RM) -rf $(bundle)
	$(MKDIR) -p $(bundle)/scummvm
	$(CP) $(DIST_FILES_DOCS) $(bundle)/scummvm
	$(MKDIR) $(bundle)/scummvm/themes
	$(CP) $(DIST_FILES_THEMES) $(bundle)/scummvm/themes/

ifdef DIST_FILES_ENGINEDATA
	$(MKDIR) $(bundle)/scummvm/engine-data
	$(CP) $(DIST_FILES_ENGINEDATA) $(bundle)/scummvm/engine-data/
endif
ifdef DIST_FILES_NETWORKING
	$(CP) $(DIST_FILES_NETWORKING) $(bundle)/scummvm
endif
ifdef DIST_FILES_VKEYBD
	$(CP) $(DIST_FILES_VKEYBD) $(bundle)/scummvm
endif
ifdef DYNAMIC_MODULES
	$(MKDIR) $(bundle)/scummvm/plugins/
	$(CP) $(PLUGINS) $(bundle)/scummvm/plugins/
endif
	$(CP) scummvm.kos $(bundle)/scummvm/scummvm
	$(CP) scummvm.dll $(bundle)/scummvm/scummvm.dll

scummvm-zip: $(bundle)
	$(RM) scummvm_kolibrios.zip
	cd $(bundle) && zip -r ../scummvm_kolibri.zip scummvm
