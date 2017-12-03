# This file contains port specific Makefile rules. It is automatically
# included by the default (main) Makefile.
#

#
# POSIX specific
#
install:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.6" "$(DESTDIR)$(mandir)/man6/residualvm.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/residualvm.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/residualvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/residualvm.svg" "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/residualvm.svg"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_NETWORKING) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/applications"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.desktop" "$(DESTDIR)$(datarootdir)/applications/residualvm.desktop"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/appdata"
#	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.appdata.xml" "$(DESTDIR)$(datarootdir)/appdata/residualvm.appdata.xml"
	# ResidualVM specific
ifdef USE_OPENGL_SHADERS
	$(INSTALL) -d "$(DESTDIR)$(datadir)/shaders"
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) "$(DESTDIR)$(datadir)/shaders"
endif
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/residualvm/"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/residualvm/"
endif

install-strip:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -s -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.6" "$(DESTDIR)$(mandir)/man6/residualvm.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/residualvm.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/residualvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/residualvm.svg" "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/residualvm.svg"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_NETWORKING) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/applications"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.desktop" "$(DESTDIR)$(datarootdir)/applications/residualvm.desktop"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/appdata"
#	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.appdata.xml" "$(DESTDIR)$(datarootdir)/appdata/residualvm.appdata.xml"
	# ResidualVM specific
ifdef USE_OPENGL_SHADERS
	$(INSTALL) -d "$(DESTDIR)$(datadir)/shaders"
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) "$(DESTDIR)$(datadir)/shaders"
endif
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/residualvm/"
	$(INSTALL) -c -s -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/residualvm/"
endif

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	rm -f "$(DESTDIR)$(mandir)/man6/residualvm.6"
	rm -f "$(DESTDIR)$(datarootdir)/pixmaps/residualvm.xpm"
	rm -f "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/residualvm.svg"
	rm -rf "$(DESTDIR)$(docdir)"
	rm -rf "$(DESTDIR)$(datadir)"
	rm -f "$(DESTDIR)$(datarootdir)/applications/scummvm.desktop"
	rm -f "$(DESTDIR)$(datarootdir)/appdata/scummvm.appdata.xml"
ifdef DYNAMIC_MODULES
	rm -rf "$(DESTDIR)$(libdir)/residualvm/"
endif

#ResidualVM specific:
deb:
	ln -sf dists/debian;
	debian/prepare
	fakeroot debian/rules binary

ifdef USE_DOCKTILEPLUGIN

# The NsDockTilePlugIn needs to be compiled in both 32 and 64 bits irrespective of how ScummVM itself is compiled.
# Therefore do not use $(CXXFLAGS) and $(LDFLAGS).

ScummVMDockTilePlugin32.o:
	$(CXX) -mmacosx-version-min=10.6 -arch i386 -O2 -c $(srcdir)/backends/taskbar/macosx/dockplugin/dockplugin.m -o ScummVMDockTilePlugin32.o

ScummVMDockTilePlugin32: ScummVMDockTilePlugin32.o
	$(CXX) -mmacosx-version-min=10.6 -arch i386 -bundle -framework Foundation -framework AppKit -fobjc-link-runtime ScummVMDockTilePlugin32.o -o ScummVMDockTilePlugin32

ScummVMDockTilePlugin64.o:
	$(CXX) -mmacosx-version-min=10.6 -arch x86_64 -O2 -c $(srcdir)/backends/taskbar/macosx/dockplugin/dockplugin.m -o ScummVMDockTilePlugin64.o

ScummVMDockTilePlugin64: ScummVMDockTilePlugin64.o
	$(CXX) -mmacosx-version-min=10.6 -arch x86_64 -bundle -framework Foundation -framework AppKit -fobjc-link-runtime ScummVMDockTilePlugin64.o -o ScummVMDockTilePlugin64

ResidualVMDockTilePlugin: ScummVMDockTilePlugin32 ScummVMDockTilePlugin64
	lipo -create ScummVMDockTilePlugin32 ScummVMDockTilePlugin64 -output ResidualVMDockTilePlugin

residualvm.docktileplugin: ResidualVMDockTilePlugin
	mkdir -p residualvm.docktileplugin/Contents
	cp $(srcdir)/dists/macosx/dockplugin/Info.plist residualvm.docktileplugin/Contents
	mkdir -p residualvm.docktileplugin/Contents/MacOS
	cp ResidualVMDockTilePlugIn residualvm.docktileplugin/Contents/MacOS/
	chmod 644 residualvm.docktileplugin/Contents/MacOS/ResidualVMDockTilePlugIn

endif

bundle_name = ResidualVM.app
ifdef USE_DOCKTILEPLUGIN
bundle: residualvm-static residualvm.docktileplugin
else
bundle: residualvm-static
endif
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	sed -e 's/$$(PRODUCT_BUNDLE_IDENTIFIER)/org.residualvm.residualvm/' $(srcdir)/dists/macosx/Info.plist >$(bundle_name)/Contents/Info.plist
ifdef USE_SPARKLE
	mkdir -p $(bundle_name)/Contents/Frameworks
	cp $(srcdir)/dists/macosx/dsa_pub.pem $(bundle_name)/Contents/Resources/
	rm -rf $(bundle_name)/Contents/Frameworks/Sparkle.framework
	cp -R $(SPARKLEPATH)/Sparkle.framework $(bundle_name)/Contents/Frameworks/
endif
	cp $(srcdir)/icons/residualvm.icns $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/Contents/Resources/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(bundle_name)/Contents/Resources/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/Contents/Resources/
endif
ifdef USE_OPENGL_SHADERS
	mkdir -p $(bundle_name)/Contents/Resources/shaders
	cp $(DIST_FILES_SHADERS) $(bundle_name)/Contents/Resources/shaders/
endif
	$(srcdir)/devtools/credits.pl --rtf > $(bundle_name)/Contents/Resources/Credits.rtf
	chmod 644 $(bundle_name)/Contents/Resources/*
ifdef USE_OPENGL_SHADERS
	chmod 755 $(bundle_name)/Contents/Resources/shaders
endif
	cp residualvm-static $(bundle_name)/Contents/MacOS/residualvm
	chmod 755 $(bundle_name)/Contents/MacOS/residualvm
	$(STRIP) $(bundle_name)/Contents/MacOS/residualvm
ifdef USE_DOCKTILEPLUGIN
	mkdir -p $(bundle_name)/Contents/PlugIns
	cp -r residualvm.docktileplugin $(bundle_name)/Contents/PlugIns/
endif

iphonebundle: iphone
	mkdir -p $(bundle_name)
	cp $(srcdir)/dists/iphone/Info.plist $(bundle_name)/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(bundle_name)/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(STRIP) residualvm
	ldid -S residualvm
	chmod 755 residualvm
	cp residualvm $(bundle_name)/ResidualVM
	cp $(srcdir)/dists/iphone/icon.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/icon-72.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/Default.png $(bundle_name)/

# Location of static libs for the iPhone
ifneq ($(BACKEND), iphone)
# Static libaries, used for the residualvm-static and iphone targets
OSX_STATIC_LIBS := `$(SDLCONFIG) --static-libs`
ifdef USE_SDL_NET
ifdef USE_SDL2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libSDL2_net.a
else
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libSDL_net.a
endif
endif
# With sdl2-config we don't always get the OpenGL framework
OSX_STATIC_LIBS += -framework OpenGL
endif

ifdef USE_LIBCURL
OSX_STATIC_LIBS += -lcurl
endif

ifdef USE_FREETYPE2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfreetype.a $(STATICLIBPATH)/lib/libbz2.a
endif

ifdef USE_VORBIS
OSX_STATIC_LIBS += \
		$(STATICLIBPATH)/lib/libvorbisfile.a \
		$(STATICLIBPATH)/lib/libvorbis.a
endif

ifdef USE_TREMOR
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libvorbisidec.a
endif

ifdef USE_FLAC
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libFLAC.a
endif

ifdef USE_OGG
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libogg.a
endif

ifdef USE_FLUIDSYNTH
OSX_STATIC_LIBS += \
                -liconv -framework CoreMIDI -framework CoreAudio\
                $(STATICLIBPATH)/lib/libfluidsynth.a \
                $(STATICLIBPATH)/lib/libglib-2.0.a \
                $(STATICLIBPATH)/lib/libintl.a

ifneq ($(BACKEND), iphone)
ifneq ($(BACKEND), ios7)
OSX_STATIC_LIBS += -lreadline -framework AudioUnit
endif
endif
endif

ifdef USE_MAD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libmad.a
endif

ifdef USE_PNG
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libpng.a
endif

ifdef USE_THEORADEC
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libtheoradec.a
endif

ifdef USE_FAAD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfaad.a
endif

ifdef USE_MPEG2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libmpeg2.a
endif

ifdef USE_JPEG
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libjpeg.a
endif

ifdef USE_ZLIB
OSX_ZLIB ?= $(STATICLIBPATH)/lib/libz.a
endif

ifdef USE_SPARKLE
ifneq ($(SPARKLEPATH),)
OSX_STATIC_LIBS += -F$(SPARKLEPATH)
endif
OSX_STATIC_LIBS += -framework Sparkle -Wl,-rpath,@loader_path/../Frameworks
endif

# ResidualVM specific:
ifdef USE_OPENGL_SHADERS
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libglew.a
endif

# ResidualVM specific:
ifdef USE_ICONV
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libiconv.a
endif

# Special target to create a static linked binary for Mac OS X.
# We use -force_cpusubtype_ALL to ensure the binary runs on every
# PowerPC machine.
residualvm-static: $(OBJS)
	$(CXX) $(LDFLAGS) -force_cpusubtype_ALL -o residualvm-static $(OBJS) \
		-framework CoreMIDI \
		$(OSX_STATIC_LIBS) \
		$(OSX_ZLIB)

# Special target to create a static linked binary for the iPhone (legacy, and iOS 7+)
iphone: $(OBJS)
	$(CXX) $(LDFLAGS) -o residualvm $(OBJS) \
		$(OSX_STATIC_LIBS) \
		-framework UIKit -framework CoreGraphics -framework OpenGLES \
		-framework CoreFoundation -framework QuartzCore -framework Foundation \
		-framework AudioToolbox -framework CoreAudio -lobjc -lz

# Special target to create a snapshot disk image for Mac OS X
# TODO: Replace AUTHORS by Credits.rtf
osxsnap: bundle
	mkdir ResidualVM-snapshot
	$(srcdir)/devtools/credits.pl --text > $(srcdir)/AUTHORS
	cp $(srcdir)/AUTHORS ./ResidualVM-snapshot/Authors
	cp $(srcdir)/COPYING ./ResidualVM-snapshot/License\ \(GPL\)
	cp $(srcdir)/COPYING.BSD ./ResidualVM-snapshot/License\ \(BSD\)
	cp $(srcdir)/COPYING.LGPL ./ResidualVM-snapshot/License\ \(LGPL\)
	cp $(srcdir)/COPYING.FREEFONT ./ResidualVM-snapshot/License\ \(FREEFONT\)
	cp $(srcdir)/COPYING.ISC ./ResidualVM-snapshot/License\ \(ISC\)
	cp $(srcdir)/COPYING.LUA ./ResidualVM-snapshot/License\ \(Lua\)
	cp $(srcdir)/COPYING.MIT ./ResidualVM-snapshot/License\ \(MIT\)
	cp $(srcdir)/COPYING.TINYGL ./ResidualVM-snapshot/License\ \(TinyGL\)
	cp $(srcdir)/COPYRIGHT ./ResidualVM-snapshot/Copyright\ Holders
	cp $(srcdir)/KNOWN_BUGS ./ResidualVM-snapshot/Known\ Bugs
	cp $(srcdir)/NEWS ./ResidualVM-snapshot/News
	cp $(srcdir)/README.md ./ResidualVM-snapshot/ResidualVM\ ReadMe
	mkdir ResidualVM-snapshot/doc
	cp $(srcdir)/doc/QuickStart ./ResidualVM-snapshot/doc/QuickStart
	SetFile -t ttro -c ttxt ./ResidualVM-snapshot/*
	CpMac -r $(bundle_name) ./ResidualVM-snapshot/
	#cp $(srcdir)/dists/macosx/DS_Store ./ResidualVM-snapshot/.DS_Store
	#cp $(srcdir)/dists/macosx/background.jpg ./ResidualVM-snapshot/background.jpg
	#SetFile -a V ./ResidualVM-snapshot/.DS_Store
	#SetFile -a V ./ResidualVM-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder ResidualVM-snapshot \
					-volname "ResidualVM" \
					ResidualVM-snapshot.dmg
	rm -rf ResidualVM-snapshot
publish-appcast:
	scp dists/macosx/residualvm_appcast.xml www.residualvm.org:/var/www/appcasts/macosx/release.xml

#
# Windows specific
#

residualvmwinres.o: $(srcdir)/icons/residualvm.ico $(DIST_FILES_THEMES) $(DIST_FILES_NETWORKING) $(DIST_FILES_ENGINEDATA) $(srcdir)/dists/residualvm.rc
	$(QUIET_WINDRES)$(WINDRES) -DHAVE_CONFIG_H $(WINDRESFLAGS) $(DEFINES) -I. -I$(srcdir) $(srcdir)/dists/residualvm.rc residualvmwinres.o

# Special target to create a win32 snapshot binary (for Inno Setup)
win32dist: $(EXECUTABLE)
	mkdir -p $(WIN32PATH)
	mkdir -p $(WIN32PATH)/graphics
	mkdir -p $(WIN32PATH)/doc
	$(STRIP) $(EXECUTABLE) -o $(WIN32PATH)/$(EXECUTABLE)
	cp $(srcdir)/AUTHORS $(WIN32PATH)/AUTHORS.txt
	cp $(srcdir)/COPYING $(WIN32PATH)/COPYING.txt
	cp $(srcdir)/COPYING.BSD $(WIN32PATH)/COPYING.BSD.txt
	cp $(srcdir)/COPYING.LGPL $(WIN32PATH)/COPYING.LGPL.txt
	cp $(srcdir)/COPYING.FREEFONT $(WIN32PATH)/COPYING.FREEFONT.txt
	cp $(srcdir)/COPYING.ISC $(WIN32PATH)/COPYING.ISC.txt
	cp $(srcdir)/COPYING.LUA $(WIN32PATH)/COPYING.LUA.txt
	cp $(srcdir)/COPYING.MIT $(WIN32PATH)/COPYING.MIT.txt
	cp $(srcdir)/COPYING.TINYGL $(WIN32PATH)/COPYING.TINYGL.txt
	cp $(srcdir)/COPYRIGHT $(WIN32PATH)/COPYRIGHT.txt
	cp $(srcdir)/KNOWN_BUGS $(WIN32PATH)/KNOWN_BUGS.txt
	cp $(srcdir)/NEWS $(WIN32PATH)/NEWS.txt
	cp $(srcdir)/doc/QuickStart $(WIN32PATH)/doc/QuickStart.txt
	cp $(srcdir)/README.md $(WIN32PATH)/README.txt
	cp /usr/local/README-SDL.txt $(WIN32PATH)
	cp /usr/local/bin/SDL2.dll $(WIN32PATH)
	cp $(srcdir)/icons/residualvm.ico $(WIN32PATH)
	cp $(srcdir)/dists/win32/ResidualVM.iss $(WIN32PATH)
	unix2dos $(WIN32PATH)/*.txt
	unix2dos $(WIN32PATH)/doc/*.txt
# Special target to create a win32 NSIS installer
win32setup: $(EXECUTABLE)
	mkdir -p $(srcdir)/$(STAGINGPATH)
	$(STRIP) $(EXECUTABLE) -o $(srcdir)/$(STAGINGPATH)/$(EXECUTABLE)
	cp /usr/local/bin/SDL.dll $(srcdir)/$(STAGINGPATH)
	makensis -V2 -Dtop_srcdir="../.." -Dstaging_dir="../../$(STAGINGPATH)" -Darch=$(ARCH) $(srcdir)/dists/win32/residualvm.nsi


#
# Special target to generate project files for various IDEs
# Mainly Win32-specific
#

# The release branch is in form 'heads/branch-1-4-1', for this case
# $CUR_BRANCH will be equal to '1', for the rest cases it will be empty
CUR_BRANCH := $(shell cd $(srcdir); git describe --all |cut -d '-' -f 4-)

ideprojects: devtools/create_project
ifeq ($(VER_DIRTY), -dirty)
	$(error You have uncommitted changes)
endif
ifeq "$(CUR_BRANCH)" "heads/master"
	$(error You cannot do it on master)
else ifeq "$(CUR_BRANCH)" ""
	$(error You must be on a release branch)
endif
	@echo Creating Code::Blocks project files...
	@cd $(srcdir)/dists/codeblocks && ../../devtools/create_project/create_project ../.. --codeblocks >/dev/null && git add -f engines/plugins_table.h *.workspace *.cbp
	@echo Creating MSVC9 project files...
	@cd $(srcdir)/dists/msvc9 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 9 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcproj *.vsprops
	@echo Creating MSVC10 project files...
	@cd $(srcdir)/dists/msvc10 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 10 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo Creating MSVC11 project files...
	@cd $(srcdir)/dists/msvc11 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 11 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo Creating MSVC12 project files...
	@cd $(srcdir)/dists/msvc12 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 12 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo
	@echo All is done.
	@echo Now run
	@echo "\tgit commit -m 'DISTS: Generated Code::Blocks and MSVC project files'"

# Special target to create a win32 snapshot binary under Debian Linux using cross mingw32 toolchain
crosswin32dist: $(EXECUTABLE)
	mkdir -p ResidualVMWin32
	mkdir -p ResidualVMWin32/doc
	$(STRIP) $(EXECUTABLE) -o ResidualVMWin32/$(EXECUTABLE)
	cp $(DIST_FILES_THEMES) ResidualVMWin32
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ResidualVMWin32
endif
	cp $(srcdir)/AUTHORS ResidualVMWin32/AUTHORS.txt
	cp $(srcdir)/COPYING ResidualVMWin32/COPYING.txt
	cp $(srcdir)/COPYING.BSD ResidualVMWin32/COPYING.BSD.txt
	cp $(srcdir)/COPYING.LGPL ResidualVMWin32/COPYING.LGPL.txt
	cp $(srcdir)/COPYING.FREEFONT ResidualVMWin32/COPYING.FREEFONT.txt
	cp $(srcdir)/COPYING.ISC ResidualVMWin32/COPYING.ISC.txt
	cp $(srcdir)/COPYING.LUA ResidualVMWin32/COPYING.LUA.txt
	cp $(srcdir)/COPYING.MIT ResidualVMWin32/COPYING.MIT.txt
	cp $(srcdir)/COPYING.TINYGL ResidualVMWin32/COPYING.TINYGL.txt
	cp $(srcdir)/COPYRIGHT ResidualVMWin32/COPYRIGHT.txt
	cp $(srcdir)/KNOWN_BUGS ResidualVMWin32/KNOWN_BUGS.txt
	cp $(srcdir)/NEWS ResidualVMWin32/NEWS.txt
	cp $(srcdir)/doc/QuickStart ResidualVMWin32/doc/QuickStart.txt
	cp $(srcdir)/README.md ResidualVMWin32/README.txt
	cp $(srcdir)/dists/win32/ResidualVM.iss ResidualVMWin32
	cp /usr/i586-mingw32msvc/README-SDL.txt ResidualVMWin32
	cp /usr/i586-mingw32msvc/bin/SDL.dll ResidualVMWin32
	unix2dos ResidualVMWin32/*.txt
	unix2dos ResidualVMWin32/doc/*.txt
	unix2dos ResidualVMWin32/ResidualVM.iss

# Mark special targets as phony
.PHONY: deb bundle osxsnap win32dist install uninstall
