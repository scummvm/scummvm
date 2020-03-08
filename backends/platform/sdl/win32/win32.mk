#
# Windows specific
#

# ResidualVM: Added DIST_FILES_SHADERS
dists/residualvm.o: $(srcdir)/icons/residualvm.ico $(DIST_FILES_THEMES) $(DIST_FILES_NETWORKING) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_SHADERS) config.h $(srcdir)/base/internal_version.h

# Special target to create a win32 snapshot binary (for Inno Setup)
win32dist: all
	mkdir -p $(WIN32PATH)
	mkdir -p $(WIN32PATH)/graphics
	mkdir -p $(WIN32PATH)/doc
	$(STRIP) $(EXECUTABLE) -o $(WIN32PATH)/$(EXECUTABLE)
	cp $(srcdir)/AUTHORS $(WIN32PATH)/AUTHORS.txt
	cp $(srcdir)/COPYING $(WIN32PATH)/COPYING.txt
	cp $(srcdir)/COPYING.BSD $(WIN32PATH)/COPYING.BSD.txt
	cp $(srcdir)/COPYING.LGPL $(WIN32PATH)/COPYING.LGPL.txt
	cp $(srcdir)/COPYING.FREEFONT $(WIN32PATH)/COPYING.FREEFONT.txt
	cp $(srcdir)/COPYING.OFL $(WIN32PATH)/COPYING.OFL.txt
	cp $(srcdir)/COPYING.ISC $(WIN32PATH)/COPYING.ISC.txt
	cp $(srcdir)/COPYING.LUA $(WIN32PATH)/COPYING.LUA.txt
	cp $(srcdir)/COPYING.MIT $(WIN32PATH)/COPYING.MIT.txt
	cp $(srcdir)/COPYING.TINYGL $(WIN32PATH)/COPYING.TINYGL.txt
	cp $(srcdir)/COPYRIGHT $(WIN32PATH)/COPYRIGHT.txt
	cp $(srcdir)/doc/QuickStart $(WIN32PATH)/doc/QuickStart.txt
ifdef USE_PANDOC
	cp NEWS$(PANDOCEXT) $(WIN32PATH)/NEWS.txt
	cp README$(PANDOCEXT) $(WIN32PATH)/README.txt
else
	cp $(srcdir)/NEWS.md $(WIN32PATH)/NEWS.txt
	cp $(srcdir)/README.md $(WIN32PATH)/README.txt
endif
	cp $(WIN32SDLDOCPATH)/README-SDL.txt $(WIN32PATH)/README-SDL.txt
	cp $(WIN32SDLPATH)/SDL2.dll $(WIN32PATH)
	cp $(srcdir)/dists/win32/ResidualVM.iss $(WIN32PATH)
ifdef USE_SDL_NET
	cp $(WIN32SDLPATH)/SDL2_net.dll $(WIN32PATH)
	sed -e '/SDL2_net\.dll/ s/^;//' -i $(WIN32PATH)/ResidualVM.iss
endif
ifdef USE_SPARKLE
	cp $(WIN32SPARKLEPATH)/WinSparkle.dll $(WIN32PATH)
	sed -e '/WinSparkle\.dll/ s/^;//' -i $(WIN32PATH)/ResidualVM.iss
endif
	unix2dos $(WIN32PATH)/*.txt
	unix2dos $(WIN32PATH)/doc/*.txt

.PHONY: win32dist

include $(srcdir)/ports.mk
