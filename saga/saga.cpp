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

#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/rscfile_mod.h"
#include "saga/render.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/console.h"
#include "saga/cvar_mod.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/game_mod.h"
#include "saga/game.h"
#include "saga/interface.h"
#include "saga/isomap.h"
#include "saga/script.h"
#include "saga/scene.h"
#include "saga/sdata.h"
#include "saga/sndres.h"
#include "saga/sprite.h"
#include "saga/sound.h"
#include "saga/music.h"
#include "saga/game_mod.h"
#include "saga/palanim.h"

GameList Engine_SAGA_gameList() {
	return Saga::GAME_GameList();
}

DetectedGameList Engine_SAGA_detectGames(const FSList &fslist) {
	return Saga::GAME_ProbeGame(fslist);
}

Engine *Engine_SAGA_create(GameDetector *detector, OSystem *syst) {
	return new Saga::SagaEngine(detector, syst);
}

REGISTER_PLUGIN("SAGA Engine", Engine_SAGA_gameList, Engine_SAGA_create, Engine_SAGA_detectGames)

namespace Saga {

#define R_MAX_TIME_DELTA 100

static void CF_quitfunc(int argc, char *argv[], void *refCon);
static void CF_testfunc(int argc, char *argv[], void *refCon);

SagaEngine *_vm = NULL;

SagaEngine::SagaEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	// The Linux version of Inherit the Earth puts all data files in an
	// 'itedata' sub-directory, except for voices.rsc
	File::addDefaultDirectory(_gameDataPath + "itedata/");

	// The Windows version of Inherit the Earth puts various data files in
	// other subdirectories.
	File::addDefaultDirectory(_gameDataPath + "graphics/");
	File::addDefaultDirectory(_gameDataPath + "music/");
	File::addDefaultDirectory(_gameDataPath + "sound/");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);

	_vm = this;
}

SagaEngine::~SagaEngine() {
}

void SagaEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SagaEngine::go() {
	int msec = 0;

	_soundEnabled = 1;
	_musicEnabled = 1;

	_console = new Console(this);

	CVAR_RegisterFunc(CF_testfunc, "testfunc", "foo [ optional foo ]", R_CVAR_NONE, 0, -1, this);

	CVAR_Register_I(&_soundEnabled, "sound", NULL, R_CVAR_CFG, 0, 1);

	CVAR_Register_I(&_musicEnabled, "music", NULL, R_CVAR_CFG, 0, 1);

	CVAR_RegisterFunc(CF_quitfunc, "quit", NULL, R_CVAR_NONE, 0, 0, this);

	// Process config file
	// FIXME
	/*
	if (CFG_Read(NULL) != R_SUCCESS) {
		warning("Couldn't read configuration file");
	}
	*/

	// Process command line

	// Detect game and open resource files
	if (GAME_Init() != R_SUCCESS) {
		return;
	}

	// Initialize engine modules
	_sndRes = new SndRes(this);
	_events = new Events(this);
	_font = new Font(this);
	_sprite = new Sprite(this);
	_anim = new Anim(this);
	_script = new Script();
	_sdata = new SData();
	_interface = new Interface(this); // requires script module
	_actor = new Actor(this);
	_palanim = new PalAnim(this);
	_scene = new Scene(this);

	if (!_scene->initialized()) {
		warning("Couldn't initialize scene module");
		return;
	}

	// System initialization

	_previousTicks = _system->getMillis();

	// On some platforms, graphics initialization also initializes sound
	// ( Win32 DirectX )... Music must be initialized before sound for 
	// native midi support
	MidiDriver *driver = GameDetector::createMidi(GameDetector::detectMusicDriver(MDT_NATIVE | MDT_ADLIB | MDT_PREFER_NATIVE));
	if (!driver)
		driver = MidiDriver_ADLIB_create(_mixer);
	else if (ConfMan.getBool("native_mt32"))
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_music = new Music(driver, _musicEnabled);
	_music->hasNativeMT32(ConfMan.getBool("native_mt32"));

	if (!_musicEnabled) {
		debug(0, "Music disabled.");
	}

	// Initialize graphics
	R_GAME_DISPLAYINFO disp_info;
	GAME_GetDisplayInfo(&disp_info);
	_gfx = new Gfx(_system, disp_info.logical_w, disp_info.logical_h);

	_isoMap = new IsoMap(_gfx);
	
	_render = new Render(this, _system);
	if (!_render->initialized()) {
		return;
	}

	// Initialize system specific sound
	_sound = new Sound(this, _mixer, _soundEnabled);
	if (!_soundEnabled) {
		debug(0, "Sound disabled.");
	}

	// Register engine modules
	_console->reg(); // Register console cvars first
	GAME_Register();
	_scene->reg();
	_actor->reg();
	_script->reg();
	_render->reg();
	_anim->reg();

	_previousTicks = _system->getMillis();

	// Begin Main Engine Loop

	_scene->startScene();
	uint32 currentTicks;

	for (;;) {
		if (_render->getFlags() & RF_RENDERPAUSE) {
			// Freeze time while paused
			_previousTicks = _system->getMillis();
		} else {
			currentTicks = _system->getMillis();
			// Timer has rolled over after 49 days
			if (currentTicks < _previousTicks)
				msec = 0;
			else {
				msec = currentTicks - _previousTicks;
				_previousTicks = currentTicks;
			}
			if (msec > R_MAX_TIME_DELTA) {
				msec = R_MAX_TIME_DELTA;
			}
			_actor->direct(msec);
			_events->handleEvents(msec);
			_script->SThreadExecThreads(msec);
		}
		// Per frame processing
		_render->drawScene();
		_system->delayMillis(10);
	}
}

void SagaEngine::shutdown() {
	delete _scene;
	delete _actor;
	delete _script;
	delete _sprite;
	delete _font;
	delete _console;
	CVAR_Shutdown();
	delete _events;
	delete _palanim;

	delete _interface;
	delete _render;
	delete _isoMap;
	delete _sndRes;
	delete _sdata;
	// Shutdown system modules */
	delete _music;
	delete _sound;
	delete _anim;

	_system->quit();
}

static void CF_quitfunc(int argc, char *argv[], void *refCon) {
	((SagaEngine *)refCon)->shutdown();
	exit(0);
}

static void CF_testfunc(int argc, char *argv[], void *refCon) {
	int i;

	_vm->_console->print("Test function invoked: Got %d arguments.", argc);

	for (i = 0; i < argc; i++) {
		_vm->_console->print("Arg %d: %s", i, argv[i]);
	}
}

} // End of namespace Saga
