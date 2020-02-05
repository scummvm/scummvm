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

#include "sherlock/sherlock.h"
#include "sherlock/surface.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

namespace Sherlock {

SherlockEngine::SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Sherlock") {
	DebugMan.addDebugChannel(kDebugLevelScript,      "scripts", "Script debug level");
	DebugMan.addDebugChannel(kDebugLevelAdLibDriver, "AdLib",   "AdLib driver debugging");
	DebugMan.addDebugChannel(kDebugLevelMT32Driver,  "MT32",    "MT32 driver debugging");
	DebugMan.addDebugChannel(kDebugLevelMusic,       "Music",   "Music debugging");

	_animation = nullptr;
	_debugger = nullptr;
	_events = nullptr;
	_fixedText = nullptr;
	_inventory = nullptr;
	_journal = nullptr;
	_map = nullptr;
	_music = nullptr;
	_people = nullptr;
	_res = nullptr;
	_saves = nullptr;
	_scene = nullptr;
	_screen = nullptr;
	_sound = nullptr;
	_talk = nullptr;
	_ui = nullptr;
	_useEpilogue2 = false;
	_loadGameSlot = -1;
	_canLoadSave = false;
	_showOriginalSavesDialog = false;
	_interactiveFl = true;
	_isScreenDoubled = false;
}

SherlockEngine::~SherlockEngine() {
	delete _animation;
	//_debugger is deleted by Engine
	delete _events;
	delete _fixedText;
	delete _journal;
	delete _map;
	delete _people;
	delete _saves;
	delete _scene;
	delete _screen;
	delete _music;
	delete _sound;
	delete _talk;
	delete _ui;
	delete _inventory;
	delete _res;
}

void SherlockEngine::initialize() {
	Fonts::setVm(this);
	ImageFile::setVm(this);
	ImageFile3DO::setVm(this);
	BaseObject::setVm(this);

	if (isDemo()) {
		Common::File f;
		// The interactive demo doesn't have an intro thus doesn't include TITLE.SND
		// At the opposite, the non-interactive demo is only the intro.
		if (f.exists("TITLE.SND"))
			_interactiveFl = false;
	}

	_res = new Resources(this);
	_animation = new Animation(this);
	_debugger = Debugger::init(this);
	setDebugger(_debugger);
	_events = new Events(this);
	_fixedText = FixedText::init(this);
	_inventory = Inventory::init(this);
	_map = Map::init(this);
	_music = new Music(this, _mixer);
	_journal = Journal::init(this);
	_people = People::init(this);
	_saves = SaveManager::init(this, _targetName);
	_scene = Scene::init(this);
	_screen = Screen::init(this);
	_sound = new Sound(this, _mixer);
	_talk = Talk::init(this);
	_ui = UserInterface::init(this);

	// Load game settings
	loadConfig();

	if (getPlatform() == Common::kPlatform3DO) {
		// Disable portraits on 3DO
		// 3DO does not include portrait data
		_people->_portraitsOn = false;
	}
}

Common::Error SherlockEngine::run() {
	// Initialize the engine
	initialize();

	// Flag for whether to show original saves dialog rather than the ScummVM GMM
	_showOriginalSavesDialog = ConfMan.getBool("originalsaveload");

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= MAX_SAVEGAME_SLOTS)
			_loadGameSlot = saveSlot;
	}

	if (_loadGameSlot != -1) {
		_saves->loadGame(_loadGameSlot);
		_loadGameSlot = -1;
	} else {
		do {
			showOpening();
		} while (!shouldQuit() && !_interactiveFl);
	}

	while (!shouldQuit()) {
		// Prepare for scene, and handle any game-specific scenes. This allows
		// for game specific cutscenes or mini-games that aren't standard scenes
		startScene();
		if (shouldQuit())
			break;

		// Clear the screen
		_screen->clear();

		// Reset UI flags
		_ui->reset();

		// Reset the data for the player character (Sherlock)
		_people->reset();

		// Initialize and load the scene.
		_scene->selectScene();

		// Scene handling loop
		sceneLoop();
	}

	return Common::kNoError;
}

void SherlockEngine::sceneLoop() {
	while (!shouldQuit() && _scene->_goToScene == -1) {
		// See if a script needs to be completed from either a goto room code,
		// or a script that was interrupted by another script
		if (_talk->_scriptMoreFlag == 1 || _talk->_scriptMoreFlag == 3)
			_talk->talkTo(_talk->_scriptName);
		else
			_talk->_scriptMoreFlag = 0;

		// Handle any input from the keyboard or mouse
		handleInput();

		if (_people->_savedPos.x == -1) {
			_canLoadSave = true;
			_scene->doBgAnim();
			_canLoadSave = false;
		}
	}

	_scene->freeScene();
	_people->freeWalk();
}

void SherlockEngine::handleInput() {
	_canLoadSave = _ui->_menuMode == STD_MODE || _ui->_menuMode == LAB_MODE;
	_events->pollEventsAndWait();
	_canLoadSave = false;

	// See if a key or mouse button is pressed
	_events->setButtonState();

	_ui->handleInput();
}

bool SherlockEngine::readFlags(int flagNum) {
	bool value = _flags[ABS(flagNum)];
	if (flagNum < 0)
		value = !value;

	return value;
}

void SherlockEngine::setFlags(int flagNum) {
	_flags[ABS(flagNum)] = flagNum >= 0;

	_scene->checkSceneFlags(true);
}

void SherlockEngine::setFlagsDirect(int flagNum) {
	_flags[ABS(flagNum)] = flagNum >= 0;
}

void SherlockEngine::loadConfig() {
	// Load sound settings
	syncSoundSettings();

	ConfMan.registerDefault("font", getGameID() == GType_SerratedScalpel ? 1 : 4);

	_screen->setFont(ConfMan.getInt("font"));
	if (getGameID() == GType_SerratedScalpel)
		_screen->_fadeStyle = ConfMan.getBool("fade_style");

	_ui->_helpStyle = ConfMan.getBool("help_style");
	_ui->_slideWindows = ConfMan.getBool("window_style");
	_people->_portraitsOn = ConfMan.getBool("portraits_on");
}

void SherlockEngine::saveConfig() {
	ConfMan.setBool("mute", !_sound->_digitized);
	ConfMan.setBool("music_mute", !_music->_musicOn);
	ConfMan.setBool("speech_mute", !_sound->_speechOn);
	ConfMan.setInt("music_volume", _music->_musicVolume);
	ConfMan.setInt("sfx_volume", _sound->_soundVolume);
	ConfMan.setInt("speech_volume", _sound->_soundVolume);

	ConfMan.setInt("font", _screen->fontNumber());
	ConfMan.setBool("fade_style", _screen->_fadeStyle);
	ConfMan.setBool("help_style", _ui->_helpStyle);
	ConfMan.setBool("window_style", _ui->_slideWindows);
	ConfMan.setBool("portraits_on", _people->_portraitsOn);

	ConfMan.flushToDisk();
}

void SherlockEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	// Load sound-related settings
	_sound->syncSoundSettings();
	_music->syncMusicSettings();
}

void SherlockEngine::synchronize(Serializer &s) {
	for (uint idx = 0; idx < _flags.size(); ++idx)
		s.syncAsByte(_flags[idx]);
}

bool SherlockEngine::canLoadGameStateCurrently() {
	return _canLoadSave;
}

bool SherlockEngine::canSaveGameStateCurrently() {
	return _canLoadSave;
}

Common::Error SherlockEngine::loadGameState(int slot) {
	_saves->loadGame(slot);
	return Common::kNoError;
}

Common::Error SherlockEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_saves->saveGame(slot, desc);
	return Common::kNoError;
}

} // End of namespace Sherlock
