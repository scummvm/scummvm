/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "stdafx.h"
#include "engine.h"
#include "gameDetector.h"
#include "config-file.h"
#include "gui/newgui.h"
#include "gui/launcher.h"
#include "gui/message.h"

Config *g_config = 0;
NewGui *g_gui = 0;


#if defined(QTOPIA)
// FIXME - why exactly is this needed?
extern "C" int main(int argc, char *argv[]);
#endif

#if defined(MACOSX) || defined(QTOPIA)
#include <SDL.h>
#elif !defined(__MORPHOS__) && !defined(__DC__)
#undef main
#endif

#if defined(UNIX)
#include <sys/param.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

#if defined(UNIX)
#include <signal.h>

#ifndef SCUMM_NEED_ALIGNMENT
static void handle_errors(int sig_num) {
	error("Your system does not support unaligned memory accesses. Please rebuild with SCUMM_NEED_ALIGNMENT ");
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

static void launcherDialog(GameDetector &detector, OSystem *system)
{
	// FIXME - we need to call init_size() here so that we can display for example
	// the launcher dialog. But the Engine object will also call it again (possibly
	// with a different widht/height!9 However, this method is not for all OSystem 
	// implementations reentrant (it is so now for the SDL backend). Thus we need
	// to fix all backends to support it, if they don't already.
	system->init_size(320, 200);
	
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

	// FIXME - hack we use because LauncherDialog accesses g_system
	extern OSystem *g_system;
	g_system = system;

	Dialog *dlg = new LauncherDialog(g_gui, detector);
	dlg->runModal();
	delete dlg;
}

int main(int argc, char *argv[])
{
	GameDetector detector;
	OSystem::Property prop;

#if defined(UNIX)
	/* On Unix, do a quick endian / alignement check before starting */
	do_memory_test();

	char scummhome[MAXPATHLEN];
	if(getenv("HOME") != NULL)
		sprintf(scummhome,"%s/%s", getenv("HOME"), DEFAULT_CONFIG_FILE);
	else strcpy(scummhome,DEFAULT_CONFIG_FILE);
#else
    char scummhome[255];
	#if defined (WIN32) && !defined(_WIN32_WCE)
		GetWindowsDirectory(scummhome, 255);
		strcat(scummhome, "\\");
		strcat(scummhome, DEFAULT_CONFIG_FILE);
	#else	
		strcpy(scummhome, DEFAULT_CONFIG_FILE);
	#endif
#endif

	// Read the config file
	g_config = new Config(scummhome, "scummvm");
	g_config->set("versioninfo", SCUMMVM_VERSION);
	
	// Parse the command line information
	detector._saveconfig = false;
	detector.updateconfig();
	detector.parseCommandLine(argc, argv);	

	// Create the system object
	OSystem *system = detector.createSystem();

	// Set initial window caption
	prop.caption = "ScummVM";
	system->property(OSystem::PROP_SET_WINDOW_CAPTION, &prop);

	// Create the GUI manager
	g_gui = new NewGui(system);

	// Unless a game was specified, show the launcher dialog
	if (detector._gameFileName.isEmpty())
		launcherDialog(detector, system);

	// Verify the given game name
	if (detector.detectMain()) {
		system->quit();
		return (-1);
	}
	
	// Set the window caption to the game name
	prop.caption = detector.getGameName().c_str();
	system->property(OSystem::PROP_SET_WINDOW_CAPTION, &prop);

	// Create the game engine
	Engine *engine = Engine::createFromDetector(&detector, system);

	// Run the game engine
	engine->go();
	
	// Free up memory
	delete engine;
	delete g_gui;
	delete g_config;
	
	// ...and quit (the return 0 should never be reached)
	system->quit();
	
	return 0;
}

void *operator new(size_t size) {
	return calloc(size, 1);
}

void operator delete(void *ptr) {
	free(ptr);
}

