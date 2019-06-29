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
	cp ResidualVMDockTilePlugin residualvm.docktileplugin/Contents/MacOS/
	chmod 644 residualvm.docktileplugin/Contents/MacOS/ResidualVMDockTilePlugin

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
	cp $(DIST_FILES_DOCS) $(bundle_name)/Contents/Resources/
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
	$(srcdir)/devtools/credits.pl --rtf > $(bundle_name)/Contents/Resources/AUTHORS.rtf
	rm $(bundle_name)/Contents/Resources/AUTHORS
	mv $(bundle_name)/Contents/Resources/README.md $(bundle_name)/Contents/Resources/README # ResidualVM
	cp $(bundle_name)/Contents/Resources/COPYING.LGPL $(bundle_name)/Contents/Resources/COPYING-LGPL
	cp $(bundle_name)/Contents/Resources/COPYING.FREEFONT $(bundle_name)/Contents/Resources/COPYING-FREEFONT
	cp $(bundle_name)/Contents/Resources/COPYING.OFL $(bundle_name)/Contents/Resources/COPYING-OFL
	cp $(bundle_name)/Contents/Resources/COPYING.BSD $(bundle_name)/Contents/Resources/COPYING-BSD
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

ifdef USE_A52
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/liba52.a
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
ifdef USE_GLEW
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
	cp $(DIST_FILES_DOCS) ./ResidualVM-snapshot/
	mv ./ResidualVM-snapshot/COPYING ./ResidualVM-snapshot/License\ \(GPL\)
	mv ./ResidualVM-snapshot/COPYING.LGPL ./ResidualVM-snapshot/License\ \(LGPL\)
	mv ./ResidualVM-snapshot/COPYING.FREEFONT ./ResidualVM-snapshot/License\ \(FREEFONT\)
	mv ./ResidualVM-snapshot/COPYING.OFL ./ResidualVM-snapshot/License\ \(OFL\)
	mv ./ResidualVM-snapshot/COPYING.BSD ./ResidualVM-snapshot/License\ \(BSD\)
	mv ./ResidualVM-snapshot/COPYING.ISC ./ResidualVM-snapshot/License\ \(ISC\)
	mv ./ResidualVM-snapshot/COPYING.LUA ./ResidualVM-snapshot/License\ \(Lua\)
	mv ./ResidualVM-snapshot/COPYING.MIT ./ResidualVM-snapshot/License\ \(MIT\)
	mv ./ResidualVM-snapshot/COPYING.TINYGL ./ResidualVM-snapshot/License\ \(TinyGL\)
	$(XCODETOOLSPATH)/SetFile -t ttro -c ttxt ./ResidualVM-snapshot/*
	mkdir ResidualVM-snapshot/doc
	cp $(srcdir)/doc/QuickStart ./ResidualVM-snapshot/doc/QuickStart
	$(XCODETOOLSPATH)/SetFile -t ttro -c ttxt ./ResidualVM-snapshot/doc/QuickStart
	$(XCODETOOLSPATH)/SetFile -t ttro -c ttxt ./ResidualVM-snapshot/doc/*/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./ResidualVM-snapshot/doc/*/*
	$(XCODETOOLSPATH)/CpMac -r $(bundle_name) ./ResidualVM-snapshot/
# ResidualVM missing background file:
#	cp $(srcdir)/dists/macosx/DS_Store ./ResidualVM-snapshot/.DS_Store
#	cp $(srcdir)/dists/macosx/background.jpg ./ResidualVM-snapshot/background.jpg
#	$(XCODETOOLSPATH)/SetFile -a V ./ResidualVM-snapshot/.DS_Store
#	$(XCODETOOLSPATH)/SetFile -a V ./ResidualVM-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder ResidualVM-snapshot \
					-volname "ResidualVM" \
					ResidualVM-snapshot.dmg
	rm -rf ResidualVM-snapshot

publish-appcast:
	scp dists/macosx/residualvm_appcast.xml www.residualvm.org:/var/www/appcasts/macosx/release.xml


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
	@echo Creating MSVC project files...
	@cd $(srcdir)/dists/msvc && ../../devtools/create_project/create_project ../.. --msvc >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo
	@echo All is done.
	@echo Now run
	@echo "\tgit commit -m 'DISTS: Generated Code::Blocks and MSVC project files'"


# Mark special targets as phony
.PHONY: deb bundle osxsnap install uninstall
