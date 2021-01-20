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
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.6" "$(DESTDIR)$(mandir)/man6/scummvm.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/scummvm.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/scummvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/scummvm.svg" "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/scummvm.svg"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/applications"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.desktop" "$(DESTDIR)$(datarootdir)/applications/scummvm.desktop"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/metainfo"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.appdata.xml" "$(DESTDIR)$(datarootdir)/metainfo/scummvm.appdata.xml"
ifneq ($(DIST_FILES_SHADERS),)
	$(INSTALL) -d "$(DESTDIR)$(datadir)/shaders"
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) "$(DESTDIR)$(datadir)/shaders"
endif
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/scummvm/"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/scummvm/"
endif

install-strip:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -s -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.6" "$(DESTDIR)$(mandir)/man6/scummvm.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/scummvm.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/scummvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/scummvm.svg" "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/scummvm.svg"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/applications"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.desktop" "$(DESTDIR)$(datarootdir)/applications/scummvm.desktop"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/metainfo"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.appdata.xml" "$(DESTDIR)$(datarootdir)/metainfo/scummvm.appdata.xml"
ifneq ($(DIST_FILES_SHADERS),)
	$(INSTALL) -d "$(DESTDIR)$(datadir)/shaders"
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) "$(DESTDIR)$(datadir)/shaders"
endif
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/scummvm/"
	$(INSTALL) -c -s -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/scummvm/"
endif

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	rm -f "$(DESTDIR)$(mandir)/man6/scummvm.6"
	rm -f "$(DESTDIR)$(datarootdir)/pixmaps/scummvm.xpm"
	rm -f "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/scummvm.svg"
	rm -rf "$(DESTDIR)$(docdir)"
	rm -rf "$(DESTDIR)$(datadir)"
	rm -f "$(DESTDIR)$(datarootdir)/applications/scummvm.desktop"
	rm -f "$(DESTDIR)$(datarootdir)/metainfo/scummvm.appdata.xml"
ifdef DYNAMIC_MODULES
	rm -rf "$(DESTDIR)$(libdir)/scummvm/"
endif

# Special generic target for simple archive distribution

dist-generic: $(EXECUTABLE)
	mkdir -p ./dist-generic/scummvm/data
	mkdir -p ./dist-generic/scummvm/doc
	cp $(EXECUTABLE) ./dist-generic/scummvm
	cp $(DIST_FILES_DOCS) ./dist-generic/scummvm/doc
	cp $(DIST_FILES_THEMES) ./dist-generic/scummvm/data
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ./dist-generic/scummvm/data
endif
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) ./dist-generic/scummvm/data
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) ./dist-generic/scummvm/data
endif
ifdef DIST_FILES_SHADERS
	mkdir -p ./dist-generic/scummvm/data/shaders
	cp $(DIST_FILES_SHADERS) ./dist-generic/scummvm/data/shaders
endif

# Special target to create a application wrapper for Mac OS X

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

ifdef MACOSX_64_BITS_ONLY
ScummVMDockTilePlugin: ScummVMDockTilePlugin64
	cp ScummVMDockTilePlugin64 ScummVMDockTilePlugin
else
ScummVMDockTilePlugin: ScummVMDockTilePlugin32 ScummVMDockTilePlugin64
	lipo -create ScummVMDockTilePlugin32 ScummVMDockTilePlugin64 -output ScummVMDockTilePlugin
endif

scummvm.docktileplugin: ScummVMDockTilePlugin
	mkdir -p scummvm.docktileplugin/Contents
	cp $(srcdir)/dists/macosx/dockplugin/Info.plist scummvm.docktileplugin/Contents
	mkdir -p scummvm.docktileplugin/Contents/MacOS
	cp ScummVMDockTilePlugin scummvm.docktileplugin/Contents/MacOS/
	chmod 644 scummvm.docktileplugin/Contents/MacOS/ScummVMDockTilePlugin

endif

bundle_name = ScummVM.app

bundle-pack:
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	sed -e 's/$$(PRODUCT_BUNDLE_IDENTIFIER)/org.scummvm.scummvm/' $(srcdir)/dists/macosx/Info.plist >$(bundle_name)/Contents/Info.plist
ifdef USE_SPARKLE
	mkdir -p $(bundle_name)/Contents/Frameworks
	cp $(srcdir)/dists/macosx/dsa_pub.pem $(bundle_name)/Contents/Resources/
	rm -rf $(bundle_name)/Contents/Frameworks/Sparkle.framework
	cp -R $(SPARKLEPATH)/Sparkle.framework $(bundle_name)/Contents/Frameworks/
endif
ifdef MACOSX_USE_LEGACY_ICONS
	cp $(srcdir)/icons/scummvm_legacy.icns $(bundle_name)/Contents/Resources/scummvm.icns
else
	cp $(srcdir)/icons/scummvm.icns $(bundle_name)/Contents/Resources/scummvm.icns
endif
	cp $(DIST_FILES_DOCS) $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_THEMES) $(bundle_name)/Contents/Resources/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(bundle_name)/Contents/Resources/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/Contents/Resources/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(bundle_name)/Contents/Resources/
endif
ifneq ($(DIST_FILES_SHADERS),)
	mkdir -p $(bundle_name)/Contents/Resources/shaders
	cp $(DIST_FILES_SHADERS) $(bundle_name)/Contents/Resources/shaders/
endif
	$(srcdir)/devtools/credits.pl --rtf > $(bundle_name)/Contents/Resources/AUTHORS.rtf
	rm $(bundle_name)/Contents/Resources/AUTHORS
	@sed -i'' -e "s/AUTHORS/AUTHORS.rtf/g" $(bundle_name)/Contents/Resources/README.md
ifdef USE_PANDOC
	@sed -i'' -e "s|href=\"AUTHORS\"|href=\"https://www.scummvm.org/credits/\"|g" $(bundle_name)/Contents/Resources/README$(PANDOCEXT)
endif
	cp $(bundle_name)/Contents/Resources/COPYING.LGPL $(bundle_name)/Contents/Resources/COPYING-LGPL
	cp $(bundle_name)/Contents/Resources/COPYING.FREEFONT $(bundle_name)/Contents/Resources/COPYING-FREEFONT
	cp $(bundle_name)/Contents/Resources/COPYING.OFL $(bundle_name)/Contents/Resources/COPYING-OFL
	cp $(bundle_name)/Contents/Resources/COPYING.BSD $(bundle_name)/Contents/Resources/COPYING-BSD
	chmod 644 $(bundle_name)/Contents/Resources/*
ifneq ($(DIST_FILES_SHADERS),)
	chmod 755 $(bundle_name)/Contents/Resources/shaders
endif
	cp scummvm-static $(bundle_name)/Contents/MacOS/scummvm
	chmod 755 $(bundle_name)/Contents/MacOS/scummvm
	$(STRIP) $(bundle_name)/Contents/MacOS/scummvm
ifdef USE_DOCKTILEPLUGIN
	mkdir -p $(bundle_name)/Contents/PlugIns
	cp -r scummvm.docktileplugin $(bundle_name)/Contents/PlugIns/
endif

ifdef USE_DOCKTILEPLUGIN
bundle: scummvm-static scummvm.docktileplugin bundle-pack
else
bundle: scummvm-static bundle-pack
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
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(bundle_name)/
endif
	$(STRIP) scummvm
	ldid -S scummvm
	chmod 755 scummvm
	cp scummvm $(bundle_name)/ScummVM
	cp $(srcdir)/dists/iphone/icon.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/icon-72.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/Default.png $(bundle_name)/

ios7bundle: iphone
	mkdir -p $(bundle_name)
	awk 'BEGIN {s=0}\
		/<key>CFBundleIcons<\/key>/ {\
			print $$0;\
			print "\t<dict>";\
			print "\t\t<key>CFBundlePrimaryIcon</key>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>CFBundleIconFiles</key>";\
			print "\t\t\t<array>";\
			print "\t\t\t\t<string>AppIcon29x29</string>";\
			print "\t\t\t\t<string>AppIcon40x40</string>";\
			print "\t\t\t\t<string>AppIcon60x60</string>";\
			print "\t\t\t</array>";\
			print "\t\t</dict>";\
			print "\t</dict>";\
			s=2}\
		/<key>CFBundleIcons~ipad<\/key>/ {\
			print $$0;\
			print "\t<dict>";\
			print "\t\t<key>CFBundlePrimaryIcon</key>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>CFBundleIconFiles</key>";\
			print "\t\t\t<array>";\
			print "\t\t\t\t<string>AppIcon29x29</string>";\
			print "\t\t\t\t<string>AppIcon40x40</string>";\
			print "\t\t\t\t<string>AppIcon60x60</string>";\
			print "\t\t\t\t<string>AppIcon76x76</string>";\
			print "\t\t\t\t<string>AppIcon83.5x83.5</string>";\
			print "\t\t\t</array>";\
			print "\t\t</dict>";\
			print "\t</dict>";\
			s=2}\
		/<key>UILaunchImages<\/key>/ {\
			print $$0;\
			print "\t<array>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>8.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-800-Portrait-736h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{414, 736}</string>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>8.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-800-Landscape-736h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Landscape</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{414, 736}</string>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>8.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-800-667h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{375, 667}</string>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>7.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-700-568h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{320, 568}</string>";\
			print "\t\t</dict>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>7.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-700-Portrait</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{768, 1024}</string>";\
			print "\t\t</dict>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>7.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-700-Landscape</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Landscape</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{768, 1024}</string>";\
			print "\t\t</dict>";\
			print "\t</array>";\
			s=2}\
		s==0 {print $$0}\
		s > 0 { s-- }' $(srcdir)/dists/ios7/Info.plist >$(bundle_name)/Info.plist
	sed -i'' -e 's/$$(PRODUCT_BUNDLE_IDENTIFIER)/org.scummvm.scummvm/' $(bundle_name)/Info.plist
	sed -i'' -e '/UILaunchStoryboardName/{N;d;}' $(bundle_name)/Info.plist
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(bundle_name)/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
ifdef DIST_FILES_VKEYBD
	cp $(DIST_FILES_VKEYBD) $(bundle_name)/
endif
	$(STRIP) scummvm
	ldid -S scummvm
	chmod 755 scummvm
	cp scummvm $(bundle_name)/ScummVM
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29@2x.png $(bundle_name)/AppIcon29x29@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29@2x.png $(bundle_name)/AppIcon29x29@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29@3x.png $(bundle_name)/AppIcon29x29@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29.png $(bundle_name)/AppIcon29x29~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40@2x.png $(bundle_name)/AppIcon40x40@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40@2x.png $(bundle_name)/AppIcon40x40@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40@3x.png $(bundle_name)/AppIcon40x40@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40.png $(bundle_name)/AppIcon40x40~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-60@2x.png $(bundle_name)/AppIcon60x60@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-60@3x.png $(bundle_name)/AppIcon60x60@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-76@2x.png $(bundle_name)/AppIcon76x76@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-76.png $(bundle_name)/AppIcon76x76~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-83.5@2x.png $(bundle_name)/AppIcon83.5x83.5@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-640x1136-1.png $(bundle_name)/LaunchImage-700-568h@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-2048x1536.png $(bundle_name)/LaunchImage-700-Landscape@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-1024x768.png $(bundle_name)/LaunchImage-700-Landscape~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-1536x2048.png $(bundle_name)/LaunchImage-700-Portrait@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-768x1024.png $(bundle_name)/LaunchImage-700-Portrait~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-1242x2208.png $(bundle_name)/LaunchImage-800-Portrait-736h@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-2208x1242.png $(bundle_name)/LaunchImage-800-Landscape-736h@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-750x1334.png $(bundle_name)/LaunchImage-800-667h@2x.png


ifndef WITHOUT_SDL
OSX_STATIC_LIBS := `$(SDLCONFIG) --prefix=$(STATICLIBPATH) --static-libs`

ifdef USE_SDL_NET
ifdef USE_SDL2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libSDL2_net.a
else
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libSDL_net.a
endif
endif

# With sdl2-config we don't always get the OpenGL framework
OSX_STATIC_LIBS += -framework OpenGL

else # WITHOUT_SDL

# Special SDL_Net library without SDL (iPhone)
ifdef USE_SDL_NET
ifeq ($(SDL_NET_MAJOR),1)
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libSDL_net.a
else
ifeq ($(SDL_NET_MAJOR),2)
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libSDL2_net.a
endif
endif
endif # USE_SDL_NET

endif # WITHOUT_SDL

ifdef USE_LIBCURL
OSX_STATIC_LIBS += -lcurl -framework Security
endif

ifdef USE_FREETYPE2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfreetype.a $(STATICLIBPATH)/lib/libbz2.a
endif

ifdef USE_FRIBIDI
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfribidi.a
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
OSX_STATIC_LIBS += -liconv \
                -framework CoreMIDI -framework CoreAudio\
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

ifdef USE_DISCORD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libdiscord-rpc.a
endif

ifdef USE_SPARKLE
ifdef MACOSX
ifneq ($(SPARKLEPATH),)
OSX_STATIC_LIBS += -F$(SPARKLEPATH)
endif
OSX_STATIC_LIBS += -framework Sparkle -Wl,-rpath,@loader_path/../Frameworks
endif
endif
ifdef USE_GLEW
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libGLEW.a
endif

# Special target to create a static linked binary for Mac OS X.
# We use -force_cpusubtype_ALL to ensure the binary runs on every
# PowerPC machine.
scummvm-static: $(DETECT_OBJS) $(OBJS)
	+$(LD) $(LDFLAGS) -force_cpusubtype_ALL -o scummvm-static $(DETECT_OBJS) $(OBJS) \
		-framework CoreMIDI \
		$(OSX_STATIC_LIBS) \
		$(OSX_ZLIB)

# Special target to create a static linked binary for the iPhone (legacy, and iOS 7+)
iphone: $(DETECT_OBJS) $(OBJS)
	+$(LD) $(LDFLAGS) -o scummvm $(DETECT_OBJS) $(OBJS) \
		$(OSX_STATIC_LIBS) \
		-framework UIKit -framework CoreGraphics -framework OpenGLES \
		-framework CoreFoundation -framework QuartzCore -framework Foundation \
		-framework AudioToolbox -framework CoreAudio -framework SystemConfiguration -lobjc -lz

# Special target to create a snapshot disk image for Mac OS X
# TODO: Replace AUTHORS by Credits.rtf
osxsnap: bundle
	mkdir ScummVM-snapshot
	cp $(DIST_FILES_DOCS) ./ScummVM-snapshot/
	mv ./ScummVM-snapshot/COPYING ./ScummVM-snapshot/License\ \(GPL\)
	mv ./ScummVM-snapshot/COPYING.LGPL ./ScummVM-snapshot/License\ \(LGPL\)
	mv ./ScummVM-snapshot/COPYING.FREEFONT ./ScummVM-snapshot/License\ \(FREEFONT\)
	mv ./ScummVM-snapshot/COPYING.OFL ./ScummVM-snapshot/License\ \(OFL\)
	mv ./ScummVM-snapshot/COPYING.BSD ./ScummVM-snapshot/License\ \(BSD\)
	mv ./ScummVM-snapshot/COPYING.ISC ./ScummVM-snapshot/License\ \(ISC\)
	mv ./ScummVM-snapshot/COPYING.LUA ./ScummVM-snapshot/License\ \(Lua\)
	mv ./ScummVM-snapshot/COPYING.MIT ./ScummVM-snapshot/License\ \(MIT\)
	mv ./ScummVM-snapshot/COPYING.TINYGL ./ScummVM-snapshot/License\ \(TinyGL\)
	$(XCODETOOLSPATH)/SetFile -t ttro -c ttxt ./ScummVM-snapshot/*
	mkdir ScummVM-snapshot/doc
	cp $(srcdir)/doc/QuickStart ./ScummVM-snapshot/doc/QuickStart
	mkdir ScummVM-snapshot/doc/cz
	cp $(DIST_FILES_DOCS_cz) ./ScummVM-snapshot/doc/cz/
	mkdir ScummVM-snapshot/doc/da
	cp $(DIST_FILES_DOCS_da) ./ScummVM-snapshot/doc/da/
	mkdir ScummVM-snapshot/doc/de
	cp $(DIST_FILES_DOCS_de) ./ScummVM-snapshot/doc/de/
	mkdir ScummVM-snapshot/doc/es
	cp $(DIST_FILES_DOCS_es) ./ScummVM-snapshot/doc/es/
	mkdir ScummVM-snapshot/doc/fr
	cp $(DIST_FILES_DOCS_fr) ./ScummVM-snapshot/doc/fr/
	mkdir ScummVM-snapshot/doc/it
	cp $(DIST_FILES_DOCS_it) ./ScummVM-snapshot/doc/it/
	mkdir ScummVM-snapshot/doc/no-nb
	cp $(DIST_FILES_DOCS_no-nb) ./ScummVM-snapshot/doc/no-nb/
	mkdir ScummVM-snapshot/doc/se
	cp $(DIST_FILES_DOCS_se) ./ScummVM-snapshot/doc/se/
	$(XCODETOOLSPATH)/SetFile -t ttro -c ttxt ./ScummVM-snapshot/doc/QuickStart
	$(XCODETOOLSPATH)/SetFile -t ttro -c ttxt ./ScummVM-snapshot/doc/*/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./ScummVM-snapshot/doc/*/*
	$(XCODETOOLSPATH)/CpMac -r $(bundle_name) ./ScummVM-snapshot/
	cp $(srcdir)/dists/macosx/DS_Store ./ScummVM-snapshot/.DS_Store
	cp $(srcdir)/dists/macosx/background.jpg ./ScummVM-snapshot/background.jpg
	$(XCODETOOLSPATH)/SetFile -a V ./ScummVM-snapshot/.DS_Store
	$(XCODETOOLSPATH)/SetFile -a V ./ScummVM-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder ScummVM-snapshot \
					-volname "ScummVM" \
					ScummVM-snapshot.dmg
	rm -rf ScummVM-snapshot

publish-appcast:
	cp dists/macosx/scummvm_appcast.xml ../scummvm-web/public_html/appcasts/macosx/release.xml


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
	@cd $(srcdir)/dists/msvc && ../../devtools/create_project/create_project ../.. --use-canonical-lib-names --msvc-version 12 --msvc >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo
	@echo All is done.
	@echo Now run
	@echo "\tgit commit -m 'DISTS: Generated Code::Blocks and MSVC project files'"

# Target to create Raspberry Pi zip containig binary and specific README
raspberrypi_dist:
	mkdir -p $(srcdir)/scummvm-rpi
	cp $(srcdir)/backends/platform/sdl/raspberrypi/README.RASPBERRYPI $(srcdir)/scummvm-rpi/README
	cp $(srcdir)/scummvm $(srcdir)/scummvm-rpi
	zip -r scummvm-rpi.zip scummvm-rpi
	rm -f -R scummvm-rpi

# Mark special targets as phony
.PHONY: deb bundle osxsnap install uninstall
