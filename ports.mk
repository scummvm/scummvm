# This file contains port specific Makefile rules. It is automatically
# included by the default (main) Makefile.
#


#
# POSIX specific
#
install:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -s -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/residualvm.6" "$(DESTDIR)$(mandir)/man6/residualvm.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/residualvm.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/residualvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) "$(DESTDIR)$(datadir)/"
	#$(INSTALL) -c -m 644 $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/residualvm/"
	$(INSTALL) -c -s -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/residualvm/"
endif

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	rm -f "$(DESTDIR)$(mandir)/man6/residualvm.6"
	rm -f "$(DESTDIR)$(datarootdir)/pixmaps/residualvm.xpm"
	rm -rf "$(DESTDIR)$(docdir)"
	rm -rf "$(DESTDIR)$(datadir)"
ifdef DYNAMIC_MODULES
	rm -rf "$(DESTDIR)$(libdir)/residualvm/"
endif

deb:
	ln -sf dists/debian;
	debian/prepare
	fakeroot debian/rules binary

# Special target to create a application wrapper for Mac OS X
bundle_name = ResidualVM.app
bundle: residualvm-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp $(srcdir)/dists/macosx/Info.plist $(bundle_name)/Contents/
	cp $(srcdir)/icons/residualvm.icns $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/Contents/Resources/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/Contents/Resources/
endif
	$(srcdir)/devtools/credits.pl --rtf > $(bundle_name)/Contents/Resources/Credits.rtf
	chmod 644 $(bundle_name)/Contents/Resources/*
	cp residualvm-static $(bundle_name)/Contents/MacOS/residualvm
	chmod 755 $(bundle_name)/Contents/MacOS/residualvm
	$(STRIP) $(bundle_name)/Contents/MacOS/residualvm

iphonebundle: iphone
	mkdir -p $(bundle_name)
	cp $(srcdir)/dists/iphone/Info.plist $(bundle_name)/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(STRIP) residualvm
	ldid -S residualvm
	chmod 755 residualvm
	cp residual $(bundle_name)/ResidualVM
	cp $(srcdir)/dists/iphone/icon.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/icon-72.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/Default.png $(bundle_name)/

# Location of static libs for the iPhone
ifneq ($(BACKEND), iphone)
# Static libaries, used for the residualvm-static and iphone targets
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

ifdef USE_FAAD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfaad.a
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
residualvm-static: $(OBJS)
	$(CXX) $(LDFLAGS) -force_cpusubtype_ALL -o residualvm-static $(OBJS) \
		-framework CoreMIDI \
		$(OSX_STATIC_LIBS) \
		$(OSX_ZLIB) \
		$(OSX_ICONV) 

# Special target to create a static linked binary for the iPhone
iphone: $(OBJS)
	$(CXX) $(LDFLAGS) -o residualvm $(OBJS) \
		$(OSX_STATIC_LIBS) \
		-framework UIKit -framework CoreGraphics -framework OpenGLES \
		-framework GraphicsServices -framework CoreFoundation -framework QuartzCore \
		-framework Foundation -framework AudioToolbox -framework CoreAudio \
		-lobjc -lz

# Special target to create a snapshot disk image for Mac OS X
# TODO: Replace AUTHORS by Credits.rtf
osxsnap: bundle
	mkdir ResidualVM-snapshot
	$(srcdir)/devtools/credits.pl --text > $(srcdir)/AUTHORS
	cp $(srcdir)/AUTHORS ./ResidualVM-snapshot/Authors
	cp $(srcdir)/COPYING ./ResidualVM-snapshot/License\ \(GPL\)
	cp $(srcdir)/COPYING.LGPL ./ResidualVM-snapshot/License\ \(LGPL\)
	cp $(srcdir)/COPYRIGHT ./ResidualVM-snapshot/Copyright\ Holders
	cp $(srcdir)/NEWS ./ResidualVM-snapshot/News
	cp $(srcdir)/README ./ResidualVM-snapshot/Residual\ ReadMe
	/Developer/Tools/SetFile -t ttro -c ttxt ./ResidualVM-snapshot/*
	/Developer/Tools/CpMac -r $(bundle_name) ./ResidualVM-snapshot/
	#cp $(srcdir)/dists/macosx/DS_Store ./ResidualVM-snapshot/.DS_Store
	#cp $(srcdir)/dists/macosx/background.jpg ./ResidualVM-snapshot/background.jpg
	#/Developer/Tools/SetFile -a V ./ResidualVM-snapshot/.DS_Store
	#/Developer/Tools/SetFile -a V ./ResidualVM-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder ResidualVM-snapshot \
					-volname "ResidualVM" \
					ResidualVM-snapshot.dmg
	rm -rf ResidualVM-snapshot

#
# Windows specific
#

residualvmwinres.o: $(srcdir)/icons/residualvm.ico $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(srcdir)/dists/residualvm.rc
	$(QUIET_WINDRES)$(WINDRES) -DHAVE_CONFIG_H $(WINDRESFLAGS) $(DEFINES) -I. -I$(srcdir) $(srcdir)/dists/residualvm.rc residualvmwinres.o

# Special target to create a win32 snapshot binary
win32dist: $(EXECUTABLE)
	mkdir -p $(WIN32PATH)
	mkdir -p $(WIN32PATH)/graphics
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
	cp $(srcdir)/icons/residualvm.ico $(WIN32PATH)
	cp $(srcdir)/dists/residualvm.iss $(WIN32PATH)
	unix2dos $(WIN32PATH)/*.txt

# Special target to create a win32 snapshot binary under Debian Linux using cross mingw32 toolchain
crosswin32dist: $(EXECUTABLE)
	mkdir -p ResidualVMWin32
	$(STRIP) $(EXECUTABLE) -o ResidualVMWin32/$(EXECUTABLE)
	cp $(DIST_FILES_THEMES) ResidualVMWin32
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ResidualVMWin32
endif
	cp $(srcdir)/AUTHORS ResidualVMWin32/AUTHORS.txt
	cp $(srcdir)/COPYING ResidualVMWin32/COPYING.txt
	cp $(srcdir)/COPYING.LGPL ResidualVMWin32/COPYING.LGPL.txt
	cp $(srcdir)/COPYRIGHT ResidualVMWin32/COPYRIGHT.txt
	cp $(srcdir)/NEWS ResidualVMWin32/NEWS.txt
	cp $(srcdir)/README ResidualVMWin32/README.txt
	cp $(srcdir)/dists/residualvm.iss ResidualVMWin32
	cp /usr/i586-mingw32msvc/README-SDL.txt ResidualVMWin32
	cp /usr/i586-mingw32msvc/bin/SDL.dll ResidualVMWin32
	toms ResidualVMWin32/*.txt
	toms ResidualVMWin32/residualvm.iss

#
# AmigaOS specific
#

# Special target to create an AmigaOS snapshot installation
aos4dist: $(EXECUTABLE)
	mkdir -p $(AOS4PATH)
	$(STRIP) $(EXECUTABLE) -o $(AOS4PATH)/$(EXECUTABLE)
	cp icons/residualvm.info $(AOS4PATH)/$(EXECUTABLE).info
	cp $(DIST_FILES_THEMES) $(AOS4PATH)/themes/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(AOS4PATH)/extras/
endif
	cp $(DIST_FILES_DOCS) $(AOS4PATH)

#
# PlayStation 3 specific
#
ps3pkg: $(EXECUTABLE)
	$(STRIP) $(EXECUTABLE)
	sprxlinker $(EXECUTABLE)
	mkdir -p ps3pkg/USRDIR/data/
	mkdir -p ps3pkg/USRDIR/doc/
	mkdir -p ps3pkg/USRDIR/saves/
	make_self_npdrm "$(EXECUTABLE)" ps3pkg/USRDIR/EBOOT.BIN UP0001-SCUM12000_00-0000000000000000
	cp $(DIST_FILES_THEMES) ps3pkg/USRDIR/data/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) ps3pkg/USRDIR/data/
endif
	cp $(DIST_FILES_DOCS) ps3pkg/USRDIR/doc/
	cp $(srcdir)/dists/ps3/readme-ps3.md ps3pkg/USRDIR/doc/
	cp $(srcdir)/backends/vkeybd/packs/vkeybd_default.zip ps3pkg/USRDIR/data/
	cp $(srcdir)/dists/ps3/ICON0.PNG ps3pkg/
	cp $(srcdir)/dists/ps3/PIC1.PNG ps3pkg/
	sfo.py -f $(srcdir)/dists/ps3/sfo.xml ps3pkg/PARAM.SFO
	pkg.py --contentid UP0001-SCUM12000_00-0000000000000000 ps3pkg/ residualvm-ps3.pkg
	package_finalize residual-ps3.pkg

# Mark special targets as phony
.PHONY: deb bundle osxsnap win32dist install uninstall ps3pkg

