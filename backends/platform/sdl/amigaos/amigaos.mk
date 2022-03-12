# Special target to create an AmigaOS snapshot installation.
#
# WORKAROUNDS:
#
# 'mkdir' seems to incorrectly set permissions to path/dirs on AmigaOS.
# Once a vanilla installation was created, none of the special subdirectories
# are found/accessible (extras, themes, plugins), instead ScummVM reports
# missing theme files and a missing valid translation.dat.
# Switching to AmigaOS' own "makedir" until there is a fix or other solution.
#
amigaosdist: $(EXECUTABLE) $(PLUGINS)
	makedir all $(AMIGAOSPATH)
	cp ${srcdir}/dists/amigaos/scummvm_drawer.info $(patsubst %/,%,$(AMIGAOSPATH)).info
	cp ${srcdir}/dists/amigaos/scummvm.info $(AMIGAOSPATH)/$(EXECUTABLE).info
ifdef DIST_FILES_DOCS
	makedir all $(AMIGAOSPATH)/doc
	cp $(DIST_FILES_DOCS) $(AMIGAOSPATH)/doc
	$(foreach lang, $(DIST_FILES_DOCS_languages), makedir all $(AMIGAOSPATH)/doc/$(lang); cp $(DIST_FILES_DOCS_$(lang)) $(AMIGAOSPATH)/doc/$(lang);)
	# README.md must be in the current working directory
	# when building out of tree.
	cp ${srcdir}/README.md README.tmp
	# AmigaOS AREXX will error with a "Program not found" message
	# if srcdir is '.'. Copy the script to cwd instead.
	cp ${srcdir}/dists/amigaos/md2ag.rexx .
	# LC_ALL is here to workaround Debian bug #973647
	LC_ALL=C rx md2ag.rexx README.tmp $(AMIGAOSPATH)/doc/
	rm -f md2ag.rexx README.tmp
endif
	# Copy mandatory installation files.
	makedir all $(AMIGAOSPATH)/extras
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AMIGAOSPATH)/extras
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(AMIGAOSPATH)/extras
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(AMIGAOSPATH)/extras
endif
ifdef DIST_FILES_THEMES
	makedir all $(AMIGAOSPATH)/themes
	cp $(DIST_FILES_THEMES) $(AMIGAOSPATH)/themes
endif
ifneq ($(DIST_FILES_SHADERS),)
	makedir all $(AMIGAOSPATH)/shaders
	cp $(DIST_FILES_SHADERS) $(AMIGAOSPATH)/shaders
endif
	# Strip and copy engine plugins.
ifdef DYNAMIC_MODULES
	makedir all $(AMIGAOSPATH)/plugins
	$(foreach plugin, $(PLUGINS), $(STRIP) $(plugin) -o $(AMIGAOSPATH)/$(plugin);)
	# Shared objects get updates. To avoid conflicts with obsolete
	# or outdated .so's, always remove and install them completely.
	rm -rf $(AMIGAOSPATH)/sobjs
	makedir all $(AMIGAOSPATH)/sobjs
	# Extract and install compiled-in shared libraries.
	# Not every AmigaOS install, especially vanilla ones, will have
	# every mandatory shared library, in the correct place, available.
	cp ${srcdir}/dists/amigaos/Ext_Inst_so.rexx .
	rx Ext_Inst_so.rexx $(EXECUTABLE) $(AMIGAOSPATH)
	rm -f Ext_Inst_so.rexx
endif
	$(STRIP) $(EXECUTABLE) -o $(AMIGAOSPATH)/$(EXECUTABLE)
