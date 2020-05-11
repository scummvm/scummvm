/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "engines/util.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "cge2/cge2.h"
#include "cge2/bitmap.h"
#include "cge2/vga13h.h"
#include "cge2/sound.h"
#include "cge2/text.h"
#include "cge2/hero.h"
#include "cge2/general.h"
#include "cge2/spare.h"
#include "cge2/talk.h"
#include "cge2/cge2_main.h"
#include "cge2/map.h"

namespace CGE2 {

CGE2Engine::CGE2Engine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _gameDescription(gameDescription), _randomSource("cge2") {

	// Debug/console setup
	DebugMan.addDebugChannel(kCGE2DebugOpcode, "opcode", "CGE2 opcode debug channel");

	_resman = nullptr;
	_vga = nullptr;
	_midiPlayer = nullptr;
	_fx = nullptr;
	_sound = nullptr;
	_text = nullptr;
	for (int i = 0; i < 2; i++)
		_heroTab[i] = nullptr;
	_eye = nullptr;
	for (int i = 0; i < kSceneMax; i++)
		_eyeTab[i] = nullptr;
	_spare = nullptr;
	_commandHandler = nullptr;
	_commandHandlerTurbo = nullptr;
	_font = nullptr;
	_infoLine = nullptr;
	_mouse = nullptr;
	_keyboard = nullptr;
	_talk = nullptr;
	for (int i = 0; i < kMaxPoint; i++)
		_point[i] = nullptr;
	_sys = nullptr;
	_busyPtr = nullptr;
	for (int i = 0; i < 2; i++)
		_vol[i] = nullptr;
	_eventManager = nullptr;
	_map = nullptr;
	_quitFlag = false;
	_bitmapPalette = nullptr;
	_gamePhase = kPhaseIntro;
	_now = 1;
	_sex = 1;
	_mouseTop = kWorldHeight / 3;
	_dark = false;
	_lastFrame = 0;
	_lastTick = 0;
	_waitSeq = 0;
	_waitRef = 0;
	_soundStat._wait = nullptr;
	_soundStat._ref[0] = 0;
	_soundStat._ref[1] = 0;
	_taken = false;
	_endGame = false;
	_req = 1;
	_midiNotify = nullptr;
	_spriteNotify = nullptr;
	_startGameSlot = 0;

	_sayCap = ConfMan.getBool("subtitles");
	_sayVox = !ConfMan.getBool("speech_mute");
	_muteAll = ConfMan.getBool("mute");
	if (_muteAll) {
		_oldMusicVolume = _oldSfxVolume = 0;
		_music = _sayVox = false;
	} else {
		_oldMusicVolume = ConfMan.getInt("music_volume");
		_oldSfxVolume = ConfMan.getInt("sfx_volume");
		_music = _oldMusicVolume != 0;
	}
}

void CGE2Engine::init() {
	// Create debugger console
	setDebugger(new CGE2Console(this));
	_resman = new ResourceManager();
	_vga = new Vga(this);
	_fx = new Fx(this, 16);
	_sound = new Sound(this);
	_midiPlayer = new MusicPlayer(this);
	_text = new Text(this, "CGE");

	for (int i = 0; i < 2; i++)
		_heroTab[i] = new HeroTab(this);

	_eye = new V3D();
	for (int i = 0; i < kSceneMax; i++)
		_eyeTab[i] = new V3D();

	_spare = new Spare(this);
	_commandHandler = new CommandHandler(this, false);
	_commandHandlerTurbo = new CommandHandler(this, true);
	_font = new Font(this);
	_infoLine = new InfoLine(this, kInfoW);
	_mouse = new Mouse(this);
	_keyboard = new Keyboard(this);

	for (int i = 0; i < kMaxPoint; i++)
		_point[i] = new V3D();

	_sys = new System(this);
	_eventManager = new EventManager(this);
	_map = new Map(this);
	_startGameSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
}

void CGE2Engine::deinit() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _spare;
	delete _resman;
	delete _vga;
	delete _fx;
	delete _sound;
	delete _midiPlayer;
	delete _text;

	for (int i = 0; i < 2; i++)
		delete _heroTab[i];

	for (int i = 0; i < kSceneMax; i++)
		delete _eyeTab[i];

	delete _eye;
	delete _commandHandler;
	delete _commandHandlerTurbo;
	delete _font;
	delete _infoLine;
	delete _mouse;
	delete _keyboard;

	if (_talk != nullptr)
		delete _talk;

	for (int i = 0; i < kMaxPoint; i++)
		delete _point[i];

	delete _sys;
	delete _eventManager;
	delete _map;
}

bool CGE2Engine::hasFeature(EngineFeature f) const {
	return (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime)
		|| (f == kSupportsReturnToLauncher);
}

Common::Error CGE2Engine::run() {
	syncSoundSettings();
	initGraphics(kScrWidth, kScrHeight);

	init();
	cge2_main();
	deinit();

	ConfMan.setBool("subtitles", _sayCap);
	ConfMan.setBool("speech_mute", !_sayVox);
	ConfMan.flushToDisk();

	return Common::kNoError;
}

} // End of namespace CGE2
