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
#include "scumm.h"
#include "mididrv.h"
#include "gameDetector.h"
#include "gui.h"
#include "simon/simon.h"
#include "config-file.h"

GameDetector detector;
Gui gui;

Scumm *g_scumm;
/* FIXME */
OSystem *g_system;
SoundMixer *g_mixer;

Config * scummcfg;


#if defined(QTOPIA)
// FIXME - why exactly is this needed?
extern "C" int main(int argc, char *argv[]);
#endif

#if defined(MACOSX) || defined(QTOPIA)
#include <SDL.h>
#elif !defined(__MORPHOS__)
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

int main(int argc, char *argv[])
{
/*
Disabled this for now. What good does it do, anyway, we now have real config
files, and a proper port to MacOS classic should offer a dialog or so for any
game settings!

#if defined(MACOS_SDL)
	// support for config file for macos SDL port

	char *argitem;
	char *argstr;
	FILE *argf;

	if ((argf = fopen("configuration.macos", "r")) == NULL) {
		error("Can't open configuration file.\n");
		exit(1);
	}

	argc = 0;
	argstr = (char *)malloc(64);
	argstr = fgets(argstr, 64, argf);
	if ((argitem = strchr(argstr, '\n')) != NULL)
		*argitem = '\0';

	argitem = strtok(argstr, " ");

	while (argitem != NULL) {
		argv = (char **)realloc(argv, (argc + 1) * 8);
		argv[argc] = (char *)malloc(64);
		strcpy(argv[argc], argitem);
		argc++;

		argitem = strtok(NULL, " ");
	}

	free(argstr);
	fclose(argf);

#endif
*/

#ifdef __DC__
	extern void dc_init_hardware();
	dc_init_hardware();
#endif

#if defined(UNIX)
	char scummhome[MAXPATHLEN];
	if(getenv("HOME") != NULL)
		sprintf(scummhome,"%s/%s", getenv("HOME"), DEFAULT_CONFIG_FILE);
	else strcpy(scummhome,DEFAULT_CONFIG_FILE);
	scummcfg = new Config(scummhome, "scummvm");

	/* On Unix, do a quick endian / alignement check before starting */
	do_memory_test();
#else
    char scummhome[255];
	#if defined (WIN32) && !defined(_WIN32_WCE)
		GetWindowsDirectory(scummhome, 255);
		strcat(scummhome, "\\");
		strcat(scummhome, DEFAULT_CONFIG_FILE);
	#else	
		strcpy(scummhome,DEFAULT_CONFIG_FILE);
	#endif
	scummcfg = new Config(scummhome, "scummvm");
#endif
	scummcfg->set("versioninfo", SCUMMVM_VERSION);
	if (detector.detectMain(argc, argv))
		return (-1);

	OSystem *system = detector.createSystem();

	{
		char *s = detector.getGameName();
		OSystem::Property prop;

		prop.caption = s;
		system->property(OSystem::PROP_SET_WINDOW_CAPTION, &prop);
		free(s);
	}

	/* Simon the Sorcerer? */
	if (detector._gameId >= GID_SIMON_FIRST && detector._gameId <= GID_SIMON_LAST) {
		/* Simon the Sorcerer. Completely different initialization */
		MidiDriver *midi = detector.createMidi();
		
		SimonState *simon = SimonState::create(system, midi);
		g_system = simon->_system;
		g_mixer = &simon->_mixer[0];
		simon->_game = detector._gameId - GID_SIMON_FIRST;
		simon->set_volume(detector._sfx_volume);
		simon->_game_path = detector._gameDataPath;
		simon->go();

	} else {
		Scumm *scumm = Scumm::createFromDetector(&detector, system);
		g_scumm = scumm;
		g_system = scumm->_system;
		g_mixer = &scumm->_mixer[0];
		scumm->_sound->_sound_volume_master = 0;
		scumm->_sound->_sound_volume_music = detector._music_volume;
		scumm->_sound->_sound_volume_sfx = detector._sfx_volume;

		/* bind to Gui */
		scumm->_gui = &gui;
		gui.init(scumm);							/* Reinit GUI after loading a game */

		scumm->go();
	}
	
	delete scummcfg;
	
	return 0;
}
