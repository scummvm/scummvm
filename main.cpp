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

Config * scummcfg;


#if defined(__APPLE__)
#include <SDL.h>
#elif !defined(__MORPHOS__)
#undef main
#endif

#if defined(UNIX) || defined(UNIX_X11)
#include <sys/param.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif


int main(int argc, char *argv[])
{
#if defined(MACOS)
	/* support for config file on macos */

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

#if defined(UNIX) || defined(UNIX_X11)
	char scummhome[MAXPATHLEN];
	if(getenv("HOME") != NULL)
		sprintf(scummhome,"%s/%s", getenv("HOME"), DEFAULT_CONFIG_FILE);
	else strcpy(scummhome,DEFAULT_CONFIG_FILE);
	scummcfg = new Config(scummhome, "scummvm");
#else
	scummcfg = new Config(DEFAULT_CONFIG_FILE, "scummvm");
#endif
	scummcfg->set("versioninfo", SCUMMVM_VERSION);
	if (detector.detectMain(argc, argv))
		return (-1);

	OSystem *system = detector.createSystem();

	{
		char *s = detector.getGameName();
		system->property(OSystem::PROP_SET_WINDOW_CAPTION, (long)s);
		free(s);
	}

	/* Simon the Sorcerer? */
	if (detector._gameId >= GID_SIMON_FIRST && detector._gameId <= GID_SIMON_LAST) {
		/* Simon the Sorcerer. Completely different initialization */
		MidiDriver *midi = detector.createMidi();
		
		SimonState *simon = SimonState::create(system, midi);
		simon->_game = detector._gameId - GID_SIMON_FIRST;
		simon->set_volume(detector._sfx_volume);
		simon->_game_path = detector._gameDataPath;
		simon->go();

	} else {
		Scumm *scumm = Scumm::createFromDetector(&detector, system);
		g_scumm = scumm;

		/* bind to Gui */
		scumm->_gui = &gui;
		gui.init(scumm);							/* Reinit GUI after loading a game */

		scumm->go();
	}
	
	delete scummcfg;
	
	return 0;
}
