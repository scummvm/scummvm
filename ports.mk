# This file contains port specific Makefile rules. It is automatically
# included by the default (main) Makefile.
#


#
# UNIX specific
#
install:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -s -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	#$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	#$(INSTALL) -c -m 644 "$(srcdir)/dists/residual.6" "$(DESTDIR)$(mandir)/man6/residual.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/residual.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/residual.xpm"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) "$(DESTDIR)$(datadir)/"
	#$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/residual/"
	$(INSTALL) -c -s -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/residual/"
endif

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	rm -f "$(DESTDIR)$(mandir)/man6/residual.6"
	rm -f "$(DESTDIR)$(datarootdir)/pixmaps/residual.xpm"
	rm -rf "$(DESTDIR)$(docdir)"
	rm -rf "$(DESTDIR)$(datadir)"
ifdef DYNAMIC_MODULES
	rm -rf "$(DESTDIR)$(libdir)/residual/"
endif

deb:
	ln -sf dists/debian;
	debian/prepare
	fakeroot debian/rules binary

# Special target to create a application wrapper for Mac OS X
bundle_name = Residual.app
bundle: residual-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp $(srcdir)/dists/macosx/Info.plist $(bundle_name)/Contents/
	cp $(srcdir)/icons/residual.icns $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/Contents/Resources/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/Contents/Resources/
endif
	$(srcdir)/devtools/credits.pl --rtf > $(bundle_name)/Contents/Resources/Credits.rtf
	chmod 644 $(bundle_name)/Contents/Resources/*
	cp residual-static $(bundle_name)/Contents/MacOS/residual
	chmod 755 $(bundle_name)/Contents/MacOS/residual
	$(STRIP) $(bundle_name)/Contents/MacOS/residual

iphonebundle: iphone
	mkdir -p $(bundle_name)
	cp $(srcdir)/dists/iphone/Info.plist $(bundle_name)/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(STRIP) residual
	ldid -S residual
	chmod 755 residual
	cp residual $(bundle_name)/Residual
	cp $(srcdir)/dists/iphone/icon.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/icon-72.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/Default.png $(bundle_name)/

# Location of static libs for the iPhone
ifneq ($(BACKEND), iphone)
# Static libaries, used for the residual-static and iphone targets
OSX_STATIC_LIBS := `$(SDLCONFIG) --static-libs`
endif

ifdef USE_VORBIS
OSX_STATIC_LIBS += \
		$(STATICLIBPATH)/lib/libvorbisfile.a \
		$(STATICLIBPATH)/lib/libvorbis.a \
		$(STATICLIBPATH)/lib/libogg.a
endif

ifdef USE_TREMOR
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libvorbisidec.a
endif

ifdef USE_FLAC
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libFLAC.a
endif

ifdef USE_MAD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libmad.a
endif

ifdef USE_MPEG2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libmpeg2.a
endif

ifdef USE_PNG
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libpng.a
endif

ifdef USE_THEORADEC
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libtheoradec.a
endif

ifdef USE_ZLIB
OSX_ZLIB ?= -lz
endif

ifdef USE_TERMCONV
OSX_ICONV ?= -liconv
endif

# Special target to create a static linked binary for Mac OS X.
# We use -force_cpusubtype_ALL to ensure the binary runs on every
# PowerPC machine.
residual-static: $(OBJS)
	$(CXX) $(LDFLAGS) -force_cpusubtype_ALL -o residual-static $(OBJS) \
		-framework CoreMIDI \
		$(OSX_STATIC_LIBS) \
		$(OSX_ZLIB) \
		$(OSX_ICONV) \
		-lSystemStubs

# Special target to create a static linked binary for the iPhone
iphone: $(OBJS)
	$(CXX) $(LDFLAGS) -o residual $(OBJS) \
		$(OSX_STATIC_LIBS) \
		-framework UIKit -framework CoreGraphics -framework OpenGLES \
		-framework GraphicsServices -framework CoreFoundation -framework QuartzCore \
		-framework Foundation -framework AudioToolbox -framework CoreAudio \
		-lobjc -lz

# Special target to create a snapshot disk image for Mac OS X
# TODO: Replace AUTHORS by Credits.rtf
osxsnap: bundle
	mkdir Residual-snapshot
	$(srcdir)/devtools/credits.pl --text > $(srcdir)/AUTHORS
	cp $(srcdir)/AUTHORS ./Residual-snapshot/Authors
	cp $(srcdir)/COPYING ./Residual-snapshot/License\ \(GPL\)
	cp $(srcdir)/COPYING.LGPL ./Residual-snapshot/License\ \(LGPL\)
	cp $(srcdir)/COPYRIGHT ./Residual-snapshot/Copyright\ Holders
	cp $(srcdir)/NEWS ./Residual-snapshot/News
	cp $(srcdir)/README ./Residual-snapshot/Residual\ ReadMe
	/Developer/Tools/SetFile -t ttro -c ttxt ./Residual-snapshot/*
	/Developer/Tools/CpMac -r $(bundle_name) ./Residual-snapshot/
	#cp $(srcdir)/dists/macosx/DS_Store ./Residual-snapshot/.DS_Store
	#cp $(srcdir)/dists/macosx/background.jpg ./Residual-snapshot/background.jpg
	#/Developer/Tools/SetFile -a V ./Residual-snapshot/.DS_Store
	#/Developer/Tools/SetFile -a V ./Residual-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder Residual-snapshot \
					-volname "Residual" \
					Residual-snapshot.dmg
	rm -rf Residual-snapshot

#
# Windows specific
#

residualwinres.o: $(srcdir)/icons/residual.ico $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(srcdir)/dists/residual.rc
	$(QUIET_WINDRES)$(WINDRES) -DHAVE_CONFIG_H $(WINDRESFLAGS) $(DEFINES) -I. -I$(srcdir) $(srcdir)/dists/residual.rc residualwinres.o

# Special target to create a win32 snapshot binary
win32dist: $(EXECUTABLE)
	mkdir -p $(WIN32PATH)
	$(STRIP) $(EXECUTABLE) -o $(WIN32PATH)/$(EXECUTABLE)
	cp $(DIST_FILES_THEMES) $(WIN32PATH)
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(WIN32PATH)
endif
	cp $(srcdir)/AUTHORS $(WIN32PATH)/AUTHORS.txt
	cp $(srcdir)/COPYING $(WIN32PATH)/COPYING.txt
	cp $(srcdir)/COPYING.LGPL $(WIN32PATH)/COPYING.LGPL.txt
	cp $(srcdir)/COPYRIGHT $(WIN32PATH)/COPYRIGHT.txt
	cp $(srcdir)/NEWS $(WIN32PATH)/NEWS.txt
	cp $(srcdir)/README $(WIN32PATH)/README.txt
	cp /usr/local/README-SDL.txt $(WIN32PATH)
	cp /usr/local/bin/SDL.dll $(WIN32PATH)
	cp $(srcdir)/icons/residual.ico $(WIN32PATH)
	cp $(srcdir)/dists/win32/residual.iss $(WIN32PATH)
	unix2dos $(WIN32PATH)/*.txt

# Special target to create a win32 snapshot binary under Debian Linux using cross mingw32 toolchain
crosswin32dist: $(EXECUTABLE)
	mkdir -p ResidualWin32
	$(STRIP) $(EXECUTABLE) -o ResidualWin32/$(EXECUTABLE)
	cp $(DIST_FILES_THEMES) ResidualWin32
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ResidualWin32
endif
	cp $(srcdir)/AUTHORS ResidualWin32/AUTHORS.txt
	cp $(srcdir)/COPYING ResidualWin32/COPYING.txt
	cp $(srcdir)/COPYING.LGPL ResidualWin32/COPYING.LGPL.txt
	cp $(srcdir)/COPYRIGHT ResidualWin32/COPYRIGHT.txt
	cp $(srcdir)/NEWS ResidualWin32/NEWS.txt
	cp $(srcdir)/README ResidualWin32/README.txt
	cp $(srcdir)/dists/residual.iss ResidualWin32
	cp /usr/i586-mingw32msvc/README-SDL.txt ResidualWin32
	cp /usr/i586-mingw32msvc/bin/SDL.dll ResidualWin32
	toms ResidualWin32/*.txt
	toms ResidualWin32/residual.iss

#
# AmigaOS specific
#

# Special target to create an AmigaOS snapshot installation
aos4dist: $(EXECUTABLE)
	mkdir -p $(AOS4PATH)
	$(STRIP) $(EXECUTABLE) -o $(AOS4PATH)/$(EXECUTABLE)
	cp icons/residual.info $(AOS4PATH)/$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(AOS4PATH)/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AOS4PATH)/extras/
endif
	cp $(DIST_FILES_DOCS) $(AOS4PATH)

# Mark special targets as phony
.PHONY: deb bundle osxsnap win32dist install uninstall

