/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

/*! \mainpage %ScummVM Source Reference
 *
 * These pages contains a cross referenced documentation for the %ScummVM source code,
 * generated with Doxygen (http://www.doxygen.org) directly from the source.
 * Currently not much is actually properly documented, but at least you can get an overview
 * of almost all the classes, methods and variables, and how they interact.
 */
 
#include "stdafx.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/scaler.h"	// For GFX_NORMAL
#include "common/timer.h"
#include "gui/newgui.h"
#include "gui/launcher.h"
#include "gui/message.h"

#ifdef _WIN32_WCE
#include "backends/wince/CELauncherDialog.h" 
#endif

/*
 * Version string and build date string. These can be used by anything that
 * wants to display this information to the user (e.g. about dialog).
 *
 * Note: it would be very nice if we could instead of (or in addition to) the
 * build date present a date which corresponds to the date our source files
 * were last changed. To understand the difference, imagine that a user
 * makes a checkout of CVS on January 1, then after a week compiles it
 * (e.g. after doing a 'make clean'). The build date then will say January 8
 * even though the files were last changed on January 1.
 *
 * Another problem is that __DATE__/__TIME__ depend on the local time zone.
 *
 * It's clear that such a "last changed" date would be much more useful to us
 * for feedback purposes. After all, when somebody files a bug report, we
 * don't care about the build date, we want to know which date their checkout
 * was made. This is even more important now since anon CVS lags a few
 * days behind developer CVS.
 *
 * So, how could we implement this? At least on unix systems, a special script
 * could do it. Basically, that script would run over all .cpp/.h files and
 * parse the CVS 'Header' keyword we have in our file headers.
 * That line contains a date/time in GMT. Now, the script just has to collect
 * all these times and find the latest. This time then would be inserted into
 * a header file or so (common/date.h ?) which engine.cpp then could
 * include and put into a global variable analog to gScummVMBuildDate.
 *
 * Drawback: scanning all source/header files will be rather slow. Also, this
 * only works on systems which can run powerful enough scripts (so I guess
 * Visual C++ would be out of the game here? don't know VC enough to be sure).
 *
 * Another approach would be to somehow get CVS to update a global file
 * (e.g. LAST_CHANGED) whenever any checkins are made. That would be
 * faster and work w/o much "logic" on the client side, in particular no
 * scripts have to be run. The problem with this is that I am not even
 * sure it's actually possible! Modifying files during commit time is trivial
 * to setup, but I have no idea if/how one can also change files which are not
 * currently being commit'ed.
 */
const char *gScummVMVersion = "0.5.7cvs";
const char *gScummVMBuildDate = __DATE__ " " __TIME__;
const char *gScummVMFullVersion = "ScummVM 0.5.7cvs (" __DATE__ " " __TIME__ ")";
const char *gScummVMFeatures = ""
#ifdef USE_VORBIS
	"Vorbis "
#endif

#ifdef USE_FLAC
	"FLAC "
#endif

#ifdef USE_MAD
	"MP3 "
#endif

#ifdef USE_ALSA
	"ALSA "
#endif

#ifdef USE_ZLIB
	"zLib "
#endif

#ifdef USE_MPEG2
	"MPEG2 "
#endif
	;	

#if defined(WIN32) && defined(NO_CONSOLE)
#include <cstdio>
#define STDOUT_FILE	TEXT("stdout.txt")
#define STDERR_FILE	TEXT("stderr.txt")
#endif

#if defined(QTOPIA)
// FIXME - why exactly is this needed?
extern "C" int main(int argc, char *argv[]);
#endif

#if defined(MACOSX) || defined(QTOPIA)
#include <SDL.h>
#elif !defined(__MORPHOS__) && !defined(__DC__)
#undef main
#endif

#if defined (ALLEGRO_BACKEND)
#include "allegro.h"
#endif

#if defined(UNIX)
#include <signal.h>

#ifndef SCUMM_NEED_ALIGNMENT
static void handle_errors(int sig_num) {
	error("Your system does not support unaligned memory accesses. Please rebuild with SCUMM_NEED_ALIGNMENT (signal %d)", sig_num);
}
#endif

/* This function is here to test if the endianness / alignement compiled it is matching
   with the one at run-time. */
static void do_memory_test(void) {
	unsigned char test[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
	unsigned int value;
	/* First test endianness */
#ifdef SCUMM_LITTLE_ENDIAN
	if (*((int *) test) != 0x44332211) {
		error("Compiled as LITTLE_ENDIAN on a big endian system. Please rebuild ");
	}
	value = 0x55443322;
#else
	if (*((int *) test) != 0x11223344) {
		error("Compiled as BIG_ENDIAN on a little endian system. Please rebuild ");
	}
	value = 0x22334455;
#endif
	/* Then check if one really supports unaligned memory accesses */
#ifndef SCUMM_NEED_ALIGNMENT
	signal(SIGBUS, handle_errors);
	signal(SIGABRT, handle_errors);
	signal(SIGSEGV, handle_errors);
	if (*((unsigned int *) ((char *) test + 1)) != value) {
		error("Your system does not support unaligned memory accesses. Please rebuild with SCUMM_NEED_ALIGNMENT ");
	}
	signal(SIGBUS, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
#endif
}

#endif

static int launcherDialog(GameDetector &detector, OSystem *system) {
	// Set the user specified graphics mode (if any).
	system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());

	// FIXME - we need to call initSize() here so that we can display for example
	// the launcher dialog. But the Engine object will also call it again (possibly
	// with a different widht/height!9 However, this method is not for all OSystem 
	// implementations reentrant (it is so now for the SDL backend). Thus we need
	// to fix all backends to support it, if they don't already.
	system->initSize(320, 200);
	
	// FIXME - mouse cursors are currently always set via 8 bit data.
	// Thus for now we need to setup a dummy palette. On the long run, we might
	// want to add a set_mouse_cursor_overlay() method to OSystem, which would serve
	// two purposes:
	// 1) allow for 16 bit mouse cursors in overlay mode
	// 2) no need to backup & restore the mouse cursor before/after the overlay is shown
	const byte dummy_palette[] = {
		0, 0, 0, 0, 
		0, 0, 171, 0, 
		0, 171, 0, 0, 
		0, 171, 171, 0, 
		171, 0, 0, 0, 
		171, 0, 171, 0, 
		171, 87, 0, 0, 
		171, 171, 171, 0, 
		87, 87, 87, 0, 
		87, 87, 255, 0, 
		87, 255, 87, 0, 
		87, 255, 255, 0, 
		255, 87, 87, 0, 
		255, 87, 255, 0, 
		255, 255, 87, 0, 
		255, 255, 255, 0, 
	};

	system->set_palette(dummy_palette, 0, 16);

#ifndef _WIN32_WCE
	GUI::LauncherDialog dlg(detector);
#else
	CELauncherDialog dlg(detector);
#endif
	return dlg.runModal();
}

static void runGame(GameDetector &detector, OSystem *system) {

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description", detector._targetName));

	if (caption.isEmpty() && detector._game.description)
		caption = detector._game.description;
	if (caption.isEmpty())	
		caption = detector._targetName;
	if (!caption.isEmpty())	{
		system->setWindowCaption(caption.c_str());
	}

	// See if the game should default to 1x scaler
	if (!ConfMan.hasKey("gfx_mode", detector._targetName) && 
		(detector._game.features & GF_DEFAULT_TO_1X_SCALER)) {
		system->setGraphicsMode(GFX_NORMAL);
	} else {
	// Override global scaler with any game-specific define
		if (ConfMan.hasKey("gfx_mode")) {
			system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
		}
	}

	// (De)activate aspect-ratio correction as determined by the config settings
	system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));
	
	// (De)activate fullscreen mode as determined by the config settings 
	system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));

	// Create the game engine
	Engine *engine = detector.createEngine(system);
	assert(engine);

	// Run the game engine
	engine->go();

	// Stop all sound processing now (this prevents some race conditions later on)
	system->clearSoundCallback();

	// Free up memory
	delete engine;
};

#ifndef _WIN32_WCE
extern "C" int main(int argc, char *argv[]) {
#else
extern "C" int scummvm_main(GameDetector &detector, int argc, char *argv[]) {
#endif
	char *cfgFilename = NULL, *s=argv[1];

#if defined(UNIX)
	/* On Unix, do a quick endian / alignement check before starting */
	do_memory_test();
#endif

// Code copied from SDL_main
#if defined(WIN32) && defined(NO_CONSOLE)

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

	/* Redirect standard input and standard output */
	FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(STDOUT_FILE, "w");
#else
		newfp = fopen(STDOUT_FILE, "w");
		if (newfp) {
			*stdout = *newfp;
		}
#endif
	}
	newfp = freopen(STDERR_FILE, "w", stderr);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(STDERR_FILE, "w");
#else
		newfp = fopen(STDERR_FILE, "w");
		if (newfp) {
			*stderr = *newfp;
		}
#endif
	}
	setlinebuf(stdout);	/* Line buffered */
	setbuf(stderr, NULL);			/* No buffering */

#endif //defined(WIN32) && defined(USE_CONSOLE)


	// Quick preparse of command-line, looking for alt configfile path
	for (int i = argc - 1; i >= 1; i--) {
		s = argv[i];
		bool shortOpt = (s[0] == '-' && tolower(s[1]) == 'c');
		bool longOpt  = (s[0] == '-' && s[1] == '-'  && s[2] == 'c' && s[3] == 'o' \
				 && s[4] == 'n' && s[5] == 'f' && s[6] == 'i' && s[7] == 'g');

		if (shortOpt || longOpt) {
			if (longOpt) s+=9;
			if (shortOpt) s+=2;

			if (*s == '\0')
				break;

			cfgFilename = s;
			break;
		}
	}
	if (cfgFilename != NULL)
		ConfMan.switchFile(cfgFilename);

	// Update the config file
	ConfMan.set("versioninfo", gScummVMVersion, Common::ConfigManager::kApplicationDomain);

	// Load the plugins
	PluginManager::instance().loadPlugins();

	// Parse the command line information
#ifndef _WIN32_WCE
	GameDetector detector;
#endif
	detector.parseCommandLine(argc, argv);

	// Ensure the system object exists (it may have already been created 
	// at an earlier point, though!)
	OSystem *system = OSystem::instance();

	// Create the timer services
	g_timer = new Timer(system);

	// Set initial window caption
	system->setWindowCaption(gScummVMFullVersion);

	// Unless a game was specified, show the launcher dialog
	if (detector._targetName.isEmpty())
		launcherDialog(detector, system);

	// Uncomment the while loop, and the launcherDialog call, to allow
	// returning to the Launcher after an engine quit. Disabled currently
	// as it probably leaks memory like a sieve.
	//while(1) {
		// Verify the given game name is a valid supported game
		if (detector.detectMain()) {
			runGame(detector, system);

                	 // launcherDialog(detector, system);
		}
	//}

	// ...and quit (the return 0 should never be reached)
	system->quit();
	delete system;
	return 0;
}
// allegro needs this for some reason...
#if defined(ALLEGRO_BACKEND)
END_OF_MAIN();
#endif

/*
#if !defined(__PALM_OS__) && !defined(_WIN32_WCE)
void *operator new(size_t size) {
	return memset(malloc(size), 0xE7, size);
}

void operator delete(void *ptr) {
	free(ptr);
}

#undef free
void free_check(void *ptr) {
	if ((unsigned long)ptr == 0xE7E7E7E7UL) {
		printf("ERROR: freeing 0xE7E7E7E7\n");
		exit(1);
	}
	if ((unsigned long)ptr & 1) {
		warning("Freeing odd address 0x%x", ptr);
	}
	free(ptr);
}
#endif
*/
