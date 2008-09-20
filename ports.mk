# This file contains port specific Makefile rules. It is automatically
# included by the default (main) Makefile.
#
# $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/tools/trunk/Makefile $
# $Id: Makefile 30664 2008-01-27 19:47:41Z jvprat $


#
# UNIX specific
#
install: all
	$(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	$(INSTALL) -c -s -m 755 "./scummvm$(EXEEXT)" "$(DESTDIR)$(BINDIR)/scummvm$(EXEEXT)"
	$(INSTALL) -d "$(DESTDIR)$(MANDIR)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.6" "$(DESTDIR)$(MANDIR)/man6/scummvm.6"
	$(INSTALL) -d "$(DESTDIR)$(PREFIX)/share/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/scummvm.xpm" "$(DESTDIR)$(PREFIX)/share/pixmaps/scummvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(PREFIX)/share/doc/scummvm/"
	$(INSTALL) -c -m 644 "$(srcdir)/AUTHORS" "$(srcdir)/COPYING" "$(srcdir)/COPYRIGHT" "$(srcdir)/NEWS" "$(srcdir)/README" "$(DESTDIR)$(PREFIX)/share/doc/scummvm/"
	$(INSTALL) -d "$(DESTDIR)$(DATADIR)/scummvm/"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(DATADIR)/scummvm/"
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(LIBDIR)/scummvm/"
	$(INSTALL) -c -s -m 644 $(DIST_FILES_PLUGINS) "$(DESTDIR)$(LIBDIR)/scummvm/"
endif

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/scummvm$(EXEEXT)"
	rm -f "$(DESTDIR)$(MANDIR)/man6/scummvm.6"
	rm -f "$(DESTDIR)$(PREFIX)/share/pixmaps/scummvm.xpm"
	rm -rf "$(DESTDIR)$(PREFIX)/share/doc/scummvm/"
	rm -rf "$(DESTDIR)$(DATADIR)/scummvm/"
ifdef DYNAMIC_MODULES
	rm -rf "$(DESTDIR)$(LIBDIR)/scummvm/"
endif

deb:
	ln -sf dists/debian;
	debian/prepare
	fakeroot debian/rules binary


# Special target to create a application wrapper for Mac OS X
bundle_name = ScummVM.app
bundle: scummvm-static $(srcdir)/dists/macosx/Info.plist
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp $(srcdir)/dists/macosx/Info.plist $(bundle_name)/Contents/
	cp $(srcdir)/icons/scummvm.icns $(bundle_name)/Contents/Resources/
	cp $(srcdir)/dists/pred.dic $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_THEMES) $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/Contents/Resources/
	$(srcdir)/tools/credits.pl --rtf > $(bundle_name)/Contents/Resources/Credits.rtf
	chmod 644 $(bundle_name)/Contents/Resources/*
	cp scummvm-static $(bundle_name)/Contents/MacOS/scummvm
	chmod 755 $(bundle_name)/Contents/MacOS/scummvm
	strip $(bundle_name)/Contents/MacOS/scummvm

iphonebundle: $(srcdir)/dists/iphone/Info.plist
	mkdir -p $(bundle_name)
	cp $(srcdir)/dists/iphone/Info.plist $(bundle_name)/
	cp $(srcdir)/dists/pred.dic $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
	cp scummvm $(bundle_name)/ScummVM
	cp $(srcdir)/dists/iphone/icon.png $(bundle_name)/icon.png
	cp $(srcdir)/dists/iphone/Default.png $(bundle_name)/Default.png

# location of additional libs for OS X usually /sw/ for fink or
# /opt/local/ for darwinports
OSXOPT=/sw

# Location of static libs for the iPhone
ifeq ($(BACKEND), iphone)
OSXOPT=/usr/local/arm-apple-darwin
else
# Static libaries, used for the scummvm-static and iphone targets
OSX_STATIC_LIBS := `$(OSXOPT)/bin/sdl-config --static-libs`
endif

ifdef USE_VORBIS
OSX_STATIC_LIBS += \
		$(OSXOPT)/lib/libvorbisfile.a \
		$(OSXOPT)/lib/libvorbis.a \
		$(OSXOPT)/lib/libogg.a
endif

ifdef USE_TREMOR
OSX_STATIC_LIBS += $(OSXOPT)/lib/libvorbisidec.a
endif

ifdef USE_FLAC
OSX_STATIC_LIBS += $(OSXOPT)/lib/libFLAC.a
endif

ifdef USE_MAD
OSX_STATIC_LIBS += $(OSXOPT)/lib/libmad.a
endif

ifdef USE_MPEG2
OSX_STATIC_LIBS += $(OSXOPT)/lib/libmpeg2.a
endif

# Special target to create a static linked binary for Mac OS X.
# We use -force_cpusubtype_ALL to ensure the binary runs on every
# PowerPC machine.
scummvm-static: $(OBJS)
	$(CXX) $(LDFLAGS) -force_cpusubtype_ALL -o scummvm-static $(OBJS) \
		-framework CoreMIDI \
		$(OSX_STATIC_LIBS) \
		-lSystemStubs \
		-lz
#		$(OSXOPT)/lib/libz.a

# Special target to create a static linked binary for the iPhone
iphone: $(OBJS)
	$(CXX) $(LDFLAGS) -o scummvm $(OBJS) \
		$(OSX_STATIC_LIBS) \
		-framework UIKit -framework CoreGraphics -framework CoreSurface \
		-framework GraphicsServices -framework CoreFoundation -framework QuartzCore \
		-framework Foundation -framework AudioToolbox -framework CoreAudio \
		-lobjc -lz

# Special target to create a snapshot disk image for Mac OS X
# TODO: Replace AUTHORS by Credits.rtf
osxsnap: bundle credits
	mkdir ScummVM-snapshot
	cp $(srcdir)/AUTHORS ./ScummVM-snapshot/Authors
	cp $(srcdir)/COPYING ./ScummVM-snapshot/License
	cp $(srcdir)/COPYRIGHT ./ScummVM-snapshot/Copyright\ Holders
	cp $(srcdir)/NEWS ./ScummVM-snapshot/News
	cp $(srcdir)/README ./ScummVM-snapshot/ScummVM\ ReadMe
	/Developer/Tools/SetFile -t ttro -c ttxt ./ScummVM-snapshot/*
	/Developer/Tools/CpMac -r $(bundle_name) ./ScummVM-snapshot/
	cp $(srcdir)/dists/macosx/DS_Store ./ScummVM-snapshot/.DS_Store
	cp $(srcdir)/dists/macosx/background.jpg ./ScummVM-snapshot/background.jpg
	/Developer/Tools/SetFile -a V ./ScummVM-snapshot/.DS_Store
	/Developer/Tools/SetFile -a V ./ScummVM-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder ScummVM-snapshot \
					-volname "ScummVM snapshot" \
					ScummVM-snapshot.dmg
	rm -rf ScummVM-snapshot

#
# Windows specific
#

scummvmico.o: $(srcdir)/icons/scummvm.ico
	$(WINDRES) -I$(srcdir) $(srcdir)/dists/scummvm.rc scummvmico.o

# Special target to create a win32 snapshot binary
win32dist: scummvm$(EXEEXT)
	mkdir -p $(WIN32PATH)
	strip scummvm.exe -o $(WIN32PATH)/scummvm$(EXEEXT)
	cp $(srcdir)/dists/pred.dic $(WIN32PATH)
	cp $(DIST_FILES_THEMES) $(WIN32PATH)
	cp $(DIST_FILES_ENGINEDATA) $(WIN32PATH)
	cp $(srcdir)/AUTHORS $(WIN32PATH)/AUTHORS.txt
	cp $(srcdir)/COPYING $(WIN32PATH)/COPYING.txt
	cp $(srcdir)/COPYRIGHT $(WIN32PATH)/COPYRIGHT.txt
	cp $(srcdir)/NEWS $(WIN32PATH)/NEWS.txt
	cp $(srcdir)/README $(WIN32PATH)/README.txt
	cp /usr/local/README-SDL.txt $(WIN32PATH)
	cp /usr/local/bin/SDL.dll $(WIN32PATH)
	u2d $(WIN32PATH)/*.txt

#
# AmigaOS specific
#

# Special target to create an AmigaOS snapshot installation
aos4dist: scummvm
	mkdir -p $(AOS4PATH)
	strip -R.comment $< -o $(AOS4PATH)/$<_SVN
	cp icons/scummvm.info $(AOS4PATH)/$<_SVN.info
	cp $(DIST_FILES_THEMES) $(AOS4PATH)/themes/
	cp $(DIST_FILES_ENGINEDATA) $(AOS4PATH)/extras/
	cp $(srcdir)/dists/pred.dic $(AOS4PATH)/extras/
	cp $(srcdir)/AUTHORS $(AOS4PATH)/AUTHORS.txt
	cp $(srcdir)/COPYING $(AOS4PATH)/COPYING.txt
	cp $(srcdir)/COPYRIGHT $(AOS4PATH)/COPYRIGHT.txt
	cp $(srcdir)/NEWS $(AOS4PATH)/NEWS.txt
	cp $(srcdir)/README $(AOS4PATH)/README.txt
	cp /sdk/local/documentation/SDL-1.2.9/README-SDL.txt $(AOS4PATH)

.PHONY: deb bundle osxsnap win32dist install uninstall
