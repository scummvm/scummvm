/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "stdafx.h"

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "sound/mixer.h"
#include "common/file.h"
#include "common/config-manager.h"

#include "saga.h"

#include "reinherit.h"

#include "rscfile_mod.h"
#include "render_mod.h"
#include "actor_mod.h"
#include "animation_mod.h"
#include "console_mod.h"
#include "cvar_mod.h"
#include "events_mod.h"
#include "actionmap_mod.h"
#include "font_mod.h"
#include "game_mod.h"
#include "interface_mod.h"
#include "isomap_mod.h"
#include "script_mod.h"
#include "scene_mod.h"
#include "sndres_mod.h"
#include "sprite_mod.h"
#include "text_mod.h"
#include "objectmap_mod.h"

struct SAGAGameSettings {
	const char *name;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const SAGAGameSettings saga_settings[] = {
	/* Inherit the Earth - Original floppy version */
  	{ "ite", "Inherit the Earth (DOS)", Saga::GID_ITE,
	 MDT_ADLIB, "ite.rsc" },
	/* Inherit the Earth - CD version */
	{ "itecd", "Inherit the Earth (DOS CD Version)", Saga::GID_ITECD,
	 MDT_ADLIB, "sounds.rsc" },
	/* I Have No Mouth and I Must Scream */
	{ "ihnm", "I Have No Mouth and I Must Scream (DOS)", Saga::GID_IHNM,
	 MDT_ADLIB, "scream.res" },

	{ NULL, NULL, 0, 0, NULL }
};

GameList Engine_SAGA_gameList() {
	const SAGAGameSettings *g = saga_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_SAGA_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const SAGAGameSettings *g;
	
	for (g = saga_settings; g->name; ++g) {
		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(g->toGameSettings());
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SAGA_create(GameDetector *detector, OSystem *syst) {
	return new Saga::SagaEngine(detector, syst);
}

REGISTER_PLUGIN("SAGA Engine", Engine_SAGA_gameList, Engine_SAGA_create, Engine_SAGA_detectGames)

namespace Saga {

#define R_MAX_TIME_DELTA 100

struct R_MAIN_DATA {

	int sound_enabled;
	int music_enabled;

};

static void CF_quitfunc(int argc, char *argv[]);
static void CF_testfunc(int argc, char *argv[]);

static R_MAIN_DATA MainData;

SagaEngine::SagaEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	GAME_setGameDirectory(getGameDataPath());

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);

	// Initialize backend
	//syst->initSize(320, 240);
}

SagaEngine::~SagaEngine() {

}

void SagaEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SagaEngine::go() {
	int msec = 0;

	/* Register engine modules
	 * \*------------------------------------------------------------- */
	CON_Register();		/* Register console cvars first */

	RENDER_Register();
	GAME_Register();

	ANIM_Register();
	ACTIONMAP_Register();
	OBJECTMAP_Register();
	SCRIPT_Register();
	ACTOR_Register();
	SCENE_Register();

	MainData.sound_enabled = 1;

	CVAR_RegisterFunc(CF_testfunc,
	    "testfunc", "foo [ optional foo ]", R_CVAR_NONE, 0, -1);

	CVAR_Register_I(&MainData.sound_enabled,
	    "sound", NULL, R_CVAR_CFG, 0, 1);

	CVAR_Register_I(&MainData.music_enabled,
	    "music", NULL, R_CVAR_CFG, 0, 1);

	CVAR_RegisterFunc(CF_quitfunc, "quit", NULL, R_CVAR_NONE, 0, 0);

	/* Process config file
	 * \*------------------------------------------------------------- */
	// FIXME
	/*
	if (CFG_Read(NULL) != R_SUCCESS) {
		R_printf(R_STDERR, "Couldn't read configuration file.\n");
	}
	*/

	/* Process command line
	 * \*------------------------------------------------------------- */

	/* Detect game and open resource files
	 * \*------------------------------------------------------------- */
	if (GAME_Init() != R_SUCCESS) {

		R_printf(R_STDERR,
		    "Couldn't start the game: %s\n", GAME_GetErrS());

		return;
	}

	/* Initialize engine modules
	 * \*------------------------------------------------------------- */
	SND_Init();
	EVENT_Init();
	FONT_Init();
	SPRITE_Init();
	ANIM_Init();
	ACTIONMAP_Init();
	OBJECTMAP_Init();
	ISOMAP_Init();
	SCRIPT_Init();
	INTERFACE_Init();	/* requires script module */
	ACTOR_Init();

	if (SCENE_Init() != R_SUCCESS) {

		R_printf(R_STDERR, "Couldn't initialize scene module.\n");
		return;
	}

	/* System initialization
	 * \*------------------------------------------------------------- */

	if (SYSIO_Init() != R_SUCCESS) {

		return;
	}

	/* Must initialize system timer module first */
	if (SYSTIMER_InitMSCounter() != R_SUCCESS) {

		return;
	}

	/* On some platforms, graphics initialization also initializes sound
	 * ( Win32 DirectX )... Music must be initialized before sound for 
	 * native midi support */
	SYSMUSIC_Init(MainData.music_enabled);
	if (!MainData.music_enabled) {
		R_printf(R_STDOUT, "Music disabled.\n");
	}

	/* Initialize graphics */
	if (RENDER_Init() != R_SUCCESS) {
		return;
	}

	/* Initialize system specific sound */
	SYSSOUND_Init(MainData.sound_enabled);
	if (!MainData.sound_enabled) {
		R_printf(R_STDOUT, "Sound disabled.\n");
	}

	SYSINPUT_Init();

	SYSTIMER_ResetMSCounter();

	/* Begin Main Engine Loop
	 * \*------------------------------------------------------------- */

	SCENE_Start();

	for (;;) {

#ifdef R_USE_CUSTOM_WININIT

		if (ITESYS_CheckSignal()) {
			break;
		}
#endif

		if (RENDER_GetFlags() & RF_RENDERPAUSE) {
			/* Freeze time while paused */
			SYSTIMER_ResetMSCounter();
		} else {
			msec = SYSTIMER_ReadMSCounter();

			if (msec > R_MAX_TIME_DELTA) {
				msec = R_MAX_TIME_DELTA;
			}

			ACTOR_Direct(msec);
			EVENT_HandleEvents(msec);
			STHREAD_ExecThreads(msec);
		}

		/* Per frame processing
		 * \*--------------------------------------------------------- */
		RENDER_DrawScene();

		SYSTIMER_Sleep(0);

	}			/* end main game engine loop */

	R_Shutdown(0);

	return;
}

void SagaEngine::shutdown() {
	_system->quit();
}

void R_Shutdown(int param) {
	SCENE_Shutdown();
	ACTOR_Shutdown();
	SCRIPT_Shutdown();
	ANIM_Shutdown();
	SPRITE_Shutdown();
	OBJECTMAP_Shutdown();

	FONT_Shutdown();

	CON_Shutdown();
	CVAR_Shutdown();
	EVENT_Shutdown();

	/* Shutdown system modules */
	SYSMUSIC_Shutdown();
	SYSSOUND_Shutdown();

	SYSIO_Shutdown();

	/*  exit(param); */
}

static void CF_quitfunc(int argc, char *argv[])
{
	R_Shutdown(0);
	exit(0);
}

static void CF_testfunc(int argc, char *argv[])
{
	int i;

	CON_Print("Test function invoked: Got %d arguments.", argc);

	for (i = 0; i < argc; i++) {
		CON_Print("Arg %d: %s", i, argv[i]);
	}

	return;
}

} // End of namespace Saga
