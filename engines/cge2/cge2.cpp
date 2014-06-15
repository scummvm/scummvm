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
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "engines/util.h"

#include "cge2/cge2.h"
#include "cge2/bitmap.h"
#include "cge2/vga13h.h"
#include "cge2/sound.h"
#include "cge2/text.h"
#include "cge2/hero.h"
#include "cge2/general.h"
#include "cge2/spare.h"
#include "cge2/events.h"
#include "cge2/talk.h"
#include "cge2/cge2_main.h"
#include "cge2/map.h"

namespace CGE2 {

CGE2Engine::CGE2Engine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _gameDescription(gameDescription), _randomSource("cge") {
	_resman = nullptr;
	_vga = nullptr;
	_sprite = nullptr;
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
	_blinkSprite = nullptr;
	_map = nullptr;
	
	_quitFlag = false;
	_bitmapPalette = nullptr;
	_mode = 0;
	_music = true;
	_startupMode = 1;
	_now = 1;
	_sex = 1;
	_mouseTop = kWorldHeight / 3;
	_dark = false;
	_lastFrame = 0;
	_lastTick = 0;
	_waitSeq = 0;
	_waitRef = 0;
	_commandStat._wait = nullptr;
	_commandStat._ref[0] = 0;
	_commandStat._ref[1] = 0;
	_taken = false;
	_endGame = false;
	for (int i = 0; i < 4; i++)
		_flag[i] = false;
	_sayCap = true;
	_sayVox = true;
}

void CGE2Engine::init() {
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
}

void CGE2Engine::deinit() {
	delete _resman;
	delete _vga;
	delete _fx;
	delete _sound;
	delete _midiPlayer;
	delete _text;
	for (int i = 0; i < 2; i++)
		delete _heroTab[i];
	for (int i = 0; i < kSceneMax; i++) {
		delete _eyeTab[i];
	}
	delete _eye;
	delete _spare;
	delete _sprite;
	delete _commandHandler;
	delete _commandHandlerTurbo;
	delete _font;
	delete _infoLine;
	delete _mouse;
	delete _keyboard;
	if (_talk != nullptr)
		delete _talk;
	for (int i = 0; i < kMaxPoint; i++) {
		delete _point[i];
	}
	delete _sys;
	delete _eventManager;
	if (_blinkSprite != nullptr)
		delete _blinkSprite;
	delete _map;
}

bool CGE2Engine::hasFeature(EngineFeature f) const {
	return false;
}

bool CGE2Engine::canLoadGameStateCurrently() {
	return false;
}
bool CGE2Engine::canSaveGameStateCurrently() {
	return false;
}

Common::Error CGE2Engine::loadGameState(int slot) {
	warning("STUB: CGE2Engine::loadGameState()");
	return Common::kNoError;
}

Common::Error CGE2Engine::saveGameState(int slot, const Common::String &desc) {
	warning("STUB: CGE2Engine::saveGameState()");
	return Common::kNoError;
}

Common::Error CGE2Engine::run() {
	warning("STUB: CGE2Engine::run()");

	initGraphics(kScrWidth, kScrHeight, false);

	init();

	cge2_main();
	
	deinit();
	return Common::kNoError;
}

} // End of namespace CGE2
