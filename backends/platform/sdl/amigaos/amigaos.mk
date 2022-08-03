# Special target to create an AmigaOS snapshot installation.
#
# WORKAROUNDS:
#
# 'mkdir' seems to incorrectly set permissions to path/dirs on AmigaOS.
# Once a vanilla installation is created, none of the corresponding subdirs
# are found or accessible (extras, themes, plugins), instead ScummVM will
# report missing theme files and a missing valid translation.dat. Same with
# cross-partition access (which make we wonder if it's a FS bug afterall).
# Switching to AmigaOS' own "makedir" until there is a fix or other solution.
#
amigaosdist: $(EXECUTABLE) $(PLUGINS)
	# Releases should always be completely fresh installs.
	rm -rf $(AMIGAOSPATH)
	makedir all $(AMIGAOSPATH)
	$(CP) ${srcdir}/dists/amigaos/scummvm_drawer.info $(patsubst %/,%,$(AMIGAOSPATH)).info
	$(CP) ${srcdir}/dists/amigaos/scummvm.info $(AMIGAOSPATH)/$(EXECUTABLE).info
ifdef DIST_FILES_DOCS
	makedir all $(AMIGAOSPATH)/doc
	$(CP) $(DIST_FILES_DOCS) $(AMIGAOSPATH)/doc
	$(foreach lang, $(DIST_FILES_DOCS_languages), makedir all $(AMIGAOSPATH)/doc/$(lang); $(CP) $(DIST_FILES_DOCS_$(lang)) $(AMIGAOSPATH)/doc/$(lang);)
	# README.md and corresponding scripts must be in cwd
	# when building out of tree.
	$(CP) ${srcdir}/README.md README.tmp
	$(CP) ${srcdir}/dists/amigaos/md2ag.rexx .
	# (buildbot) LC_ALL is here to work around Debian bug #973647
	LC_ALL=C rx md2ag.rexx README.tmp $(AMIGAOSPATH)/doc/
	rm -f md2ag.rexx README.tmp
endif
	# Copy mandatory installation files.
	makedir all $(AMIGAOSPATH)/extras
ifdef DIST_FILES_ENGINEDATA
	$(CP) $(DIST_FILES_ENGINEDATA) $(AMIGAOSPATH)/extras
endif
ifdef DIST_FILES_NETWORKING
	$(CP) $(DIST_FILES_NETWORKING) $(AMIGAOSPATH)/extras
endif
ifdef DIST_FILES_VKEYBD
	$(CP) $(DIST_FILES_VKEYBD) $(AMIGAOSPATH)/extras
endif
ifdef DIST_FILES_THEMES
	makedir all $(AMIGAOSPATH)/themes
	$(CP) $(DIST_FILES_THEMES) $(AMIGAOSPATH)/themes
endif
ifneq ($(DIST_FILES_SHADERS),)
	makedir all $(AMIGAOSPATH)/extras/shaders
	$(CP) $(DIST_FILES_SHADERS) $(AMIGAOSPATH)/extras/shaders
endif
ifdef DYNAMIC_MODULES
	makedir all $(AMIGAOSPATH)/plugins
	# Catch edge-case when no engines/plugins are compiled
	# otherwise cp/strip will error out due to missing source files.
ifneq ($(PLUGINS),)
ifdef DEBUG_BUILD
		# Preserve all debug information on debug builds
		$(CP) $(PLUGINS) $(AMIGAOSPATH)/plugins/$(plugin)
else
		$(foreach plugin, $(PLUGINS), $(STRIP) $(plugin) -o $(AMIGAOSPATH)/$(plugin);)
endif
endif
	makedir all $(AMIGAOSPATH)/sobjs
	# AmigaOS installations, especially vanilla ones, won't have every
	# mandatory shared library in place, let alone the correct versions.
	# Extract and install compiled-in shared libraries to their own subdir.
	$(CP) ${srcdir}/dists/amigaos/Ext_Inst_so.rexx .
	rx Ext_Inst_so.rexx $(EXECUTABLE) $(AMIGAOSPATH)
	rm -f Ext_Inst_so.rexx
endif
ifdef DEBUG_BUILD
	# Preserve all debug information on debug builds
	$(CP) $(EXECUTABLE) $(AMIGAOSPATH)/$(EXECUTABLE)
else
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOSPATH)/$(EXECUTABLE)
endif
