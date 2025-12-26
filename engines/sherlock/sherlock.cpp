/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sherlock/sherlock.h"
#include "sherlock/surface.h"
#include "sherlock/scalpel/scalpel_3do_audio_durations.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/text-to-speech.h"
#include "common/file.h"

namespace Sherlock {

SherlockEngine::SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Sherlock") {
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
	_startupAutosave = false;

	// Initialize 3DO talkie support
	_talkieMode = TALKIE_NONE;
	_has3DOAssets = false;
	_3doAssetsPath = Common::Path();
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

	// Detect and configure 3DO talkie support (PC version with 3DO assets only)
	if (getGameID() == GType_SerratedScalpel && getPlatform() != Common::kPlatform3DO) {
		detect3DOAssets();
		registerTalkieSettings();
		loadTalkieConfig();
	}

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

		// Signal startup autosave, if there isn't already a save in
		// that slot.
		SaveStateDescriptor desc = getMetaEngine()->querySaveMetaInfos(
			_targetName.c_str(), getAutosaveSlot());
		if (!desc.isValid())
			_startupAutosave = true;
	}

	_events->showCursor();

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

		// If this is still set from the previous scene, something went wrong.
		// The next scene's path script or a continued script would be
		// incorrectly aborted
		assert(!_talk->_talkToAbort);

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

			if (_startupAutosave) {
				// When the game is first started, create an autosave.
				// This helps with the save dialog to prevent users
				// accidentally saving in the autosave slot
				_startupAutosave = false;
				saveAutosaveIfEnabled();
			}

			_canLoadSave = false;
		}
	}

	_scene->freeScene();
	_people->freeWalk();
}

void SherlockEngine::handleInput() {
	_canLoadSave = (_ui->_menuMode == STD_MODE || _ui->_menuMode == LAB_MODE) && _events->isCursorVisible();
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

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr)
		ttsMan->enable(ConfMan.getBool("tts_narrator"));
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

bool SherlockEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _canLoadSave;
}

bool SherlockEngine::canSaveGameStateCurrently(Common::U32String *msg) {
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

// ===== 3DO Talkie Support Implementation =====

void SherlockEngine::detect3DOAssets() {
	_has3DOAssets = false;
	_3doAssetsPath = Common::Path();

	// Detect 3DO assets by checking for conversation files
	// Randomly sample from the audio durations table to verify files exist
	const Common::HashMap<Common::String, uint32> &durations = Scalpel::get3doAudioDurations();

	// Collect all filenames from the duration table
	Common::Array<Common::String> filenames;
	for (auto it = durations.begin(); it != durations.end(); ++it) {
		filenames.push_back(it->_key);
	}

	if (filenames.empty()) {
		debug(1, "SherlockEngine: No 3DO audio durations available");
		return;
	}

	// Randomly select up to 5 files to check
	const int NUM_SAMPLES = 5;
	Common::Array<Common::String> samples;
	for (int i = 0; i < NUM_SAMPLES && !filenames.empty(); i++) {
		int idx = _randomSource.getRandomNumber(filenames.size() - 1);
		samples.push_back(filenames[idx]);
		filenames.remove_at(idx);
	}

	// Base paths candidates where 3DO assets might be located
	const char *basePaths[] = {
		"",
		"Movies",
		"HolmesData/",
		"HolmesData/videos/",
		"videos/",
		"3DO/",
		nullptr
	};

	Common::File testFile;

	for (int baseIdx = 0; basePaths[baseIdx] != nullptr; baseIdx++) {
		for (uint i = 0; i < samples.size(); i++) {
			// Extract room number from filename (characters at index 3-4)
			// e.g., "afr30aaa" -> room "30", "gar01aaa" -> room "01"
			Common::String filename = samples[i];
			if (filename.size() < 5) continue;

			Common::String roomNum = filename.substr(3, 2);
			Common::String streamPath = Common::String::format("%s/%s.stream",
			                                                    roomNum.c_str(), filename.c_str());
			Common::Path testPath = Common::Path(basePaths[baseIdx]).join(streamPath);

			if (testFile.open(testPath)) {
				testFile.close();
				_has3DOAssets = true;
				_3doAssetsPath = Common::Path(basePaths[baseIdx]);

				debug(1, "SherlockEngine: 3DO assets detected at: %s (verified: %s)",
				      _3doAssetsPath.empty() ? "(game directory)" : _3doAssetsPath.toString().c_str(),
				      streamPath.c_str());
				return;
			}
		}
	}

	debug(1, "SherlockEngine: No 3DO assets detected, talkie features unavailable");
}

void SherlockEngine::registerTalkieSettings() {
	// Default to audio-only mode - if user added 3DO assets, they want talkie
	ConfMan.registerDefault("talkie_mode", (int)TALKIE_AUDIO_ONLY);
}

void SherlockEngine::loadTalkieConfig() {
	// Only enable talkie mode if 3DO assets are available
	if (!_has3DOAssets) {
		_talkieMode = TALKIE_NONE;
		return;
	}

	int configMode = ConfMan.getInt("talkie_mode");
	_talkieMode = (configMode > 0) ? TALKIE_AUDIO_ONLY : TALKIE_NONE;

	debug(1, "SherlockEngine: Talkie mode set to %d (0=NONE, 1=AUDIO_ONLY)", _talkieMode);
}

void SherlockEngine::setTalkieMode(TalkieMode mode) {
	// Only allow setting if 3DO assets are available
	if (!_has3DOAssets && mode != TALKIE_NONE) {
		warning("SherlockEngine::setTalkieMode: Cannot enable talkie mode, 3DO assets not available");
		return;
	}

	_talkieMode = mode;

	// Save to configuration
	ConfMan.setInt("talkie_mode", (int)mode);
	ConfMan.flushToDisk();

	debug(1, "SherlockEngine::setTalkieMode: Talkie mode changed to %d", mode);
}

Common::Path SherlockEngine::get3DOVideoPath(const Common::String &videoFile) const {
	if (_3doAssetsPath.empty()) {
		return Common::Path(videoFile);
	}
	return _3doAssetsPath.join(videoFile);
}

bool SherlockEngine::has3DOVideo(const Common::String &videoFile) const {
	if (!_has3DOAssets) {
		return false;
	}

	Common::File file;
	Common::Path fullPath = get3DOVideoPath(videoFile);

	bool exists = file.open(fullPath);
	if (exists) {
		file.close();
	}

	return exists;
}

// ===== 3DO Talkie Support implementation =====

} // End of namespace Sherlock
