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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/resources.h"
#include "mads/sound.h"
#include "mads/sprites.h"

namespace MADS {

MADSEngine::MADSEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _randomSource("MADS") {

	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");

	// Initialize game/engine options
	_easyMouse = true;
	_invObjectsAnimated = true;
	_textWindowStill = false;
	_screenFade = SCREEN_FADE_SMOOTH;
	_musicFlag = true;
	_soundFlag = true;
	_dithering = false;
	_disableFastwalk = false;

	_dialogs = nullptr;
	_events = nullptr;
	_font = nullptr;
	_game = nullptr;
	_gameConv = nullptr;
	_palette = nullptr;
	_resources = nullptr;
	_sound = nullptr;
	_audio = nullptr;
	_screen = nullptr;
}

MADSEngine::~MADSEngine() {
	delete _dialogs;
	delete _events;
	delete _font;
	Font::deinit();
	delete _game;
	delete _gameConv;
	delete _palette;
	delete _resources;
	delete _sound;
	delete _audio;
	//_debugger Debugger is deleted by Engine

	_mixer->stopAll();
}

void MADSEngine::initialize() {
	// Initial sub-system engine references
	MSurface::setVm(this);
	MSprite::setVm(this);

	Resources::init(this);
	Conversation::init(this);
	_debugger = new Debugger(this);
	setDebugger(_debugger);
	_dialogs = Dialogs::init(this);
	_events = new EventsManager(this);
	_palette = new Palette(this);
	Font::init(this);
	_font = new Font();
	_screen = new Screen();
	_sound = new SoundManager(this, _mixer);
	_audio = new AudioPlayer(_mixer, getGameID());
	_game = Game::init(this);
	_gameConv = new GameConversations(this);

	loadOptions();

	_screen->clear();
}

void MADSEngine::loadOptions() {
	if (ConfMan.hasKey("EasyMouse"))
		_easyMouse = ConfMan.getBool("EasyMouse");

	if (ConfMan.hasKey("mute") && ConfMan.getBool("mute")) {
		_soundFlag = false;
		_musicFlag = false;
	} else {
		_soundFlag = !ConfMan.hasKey("sfx_mute") || !ConfMan.getBool("sfx_mute");
		_musicFlag = !ConfMan.hasGameDomain("music_mute") || !ConfMan.getBool("music_mute");
	}

	if (ConfMan.hasKey("ScreenFade"))
		_screenFade = (ScreenFade)ConfMan.getInt("ScreenFade");
	//if (ConfMan.hasKey("GraphicsDithering"))
	//	_dithering = ConfMan.getBool("GraphicsDithering");

	if (getGameID() == GType_RexNebular) {
		if (ConfMan.hasKey("InvObjectsAnimated"))
			_invObjectsAnimated = ConfMan.getBool("InvObjectsAnimated");
		if (ConfMan.hasKey("TextWindowStill"))
			_textWindowStill = !ConfMan.getBool("TextWindowAnimated");
		if (ConfMan.hasKey("NaughtyMode"))
			_game->setNaughtyMode(ConfMan.getBool("NaughtyMode"));
	}

	// Note: MADS is weird in that sfx and music are handled by the same driver,
	// and the game scripts themselves check for music being enabled before playing
	// a "music" sound. Which means we can independantly mute music in ScummVM, but
	// otherwise all sound, music and sfx, is controlled by the SFX volume slider.
	int soundVolume = MIN(255, ConfMan.getInt("sfx_volume"));
	_sound->setVolume(soundVolume);
}

void MADSEngine::saveOptions() {
	ConfMan.setBool("EasyMouse", _easyMouse);
	ConfMan.setInt("ScreenFade", (int)_screenFade);
	//ConfMan.setBool("GraphicsDithering", _dithering);

	ConfMan.setBool("mute", !_soundFlag && !_musicFlag);
	ConfMan.setBool("sfx_mute", !_soundFlag && _musicFlag);
	ConfMan.setBool("music_mute", _soundFlag && !_musicFlag);

	if (getGameID() == GType_RexNebular) {
		ConfMan.setBool("InvObjectsAnimated", _invObjectsAnimated);
		ConfMan.setBool("TextWindowAnimated", !_textWindowStill);
		ConfMan.setBool("NaughtyMode", _game->getNaughtyMode());
	}

	ConfMan.flushToDisk();
}

Common::Error MADSEngine::run() {
	initGraphics(MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT);
	initialize();

	// Run the game
	_game->run();

	return Common::kNoError;
}

int MADSEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

int MADSEngine::getRandomNumber(int minNumber, int maxNumber) {
	int range = maxNumber - minNumber;

	return minNumber + _randomSource.getRandomNumber(range);
}

bool MADSEngine::canLoadGameStateCurrently() {
	return !_game->_winStatus && !_game->globals()[5]
		&& _dialogs->_pendingDialog == DIALOG_NONE
		&& _events->_cursorId != CURSOR_WAIT;
}

bool MADSEngine::canSaveGameStateCurrently() {
	return !_game->_winStatus && !_game->globals()[5]
		&& _dialogs->_pendingDialog == DIALOG_NONE
		&& _events->_cursorId != CURSOR_WAIT;
}

void MADSEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	loadOptions();
}

Common::Error MADSEngine::loadGameState(int slot) {
	_game->_loadGameSlot = slot;
	_game->_scene._currentSceneId = -1;
	_game->_currentSectionNumber = -1;
	return Common::kNoError;
}

Common::Error MADSEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_game->saveGame(slot, desc);
	return Common::kNoError;
}

} // End of namespace MADS
