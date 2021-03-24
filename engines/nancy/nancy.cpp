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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/state/logo.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/help.h"
#include "engines/nancy/state/map.h"
#include "engines/nancy/state/credits.h"

#include "engines/nancy/action/sliderpuzzle.h"
#include "engines/nancy/action/primaryvideo.h"
#include "engines/nancy/action/secondarymovie.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/cheat.h"

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/installshield_cab.h"
#include "common/str.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "graphics/surface.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "engines/util.h"

namespace Nancy {

NancyEngine *g_nancy;

NancyEngine::NancyEngine(OSystem *syst, const NancyGameDescription *gd) : Engine(syst), _gameDescription(gd), _system(syst) {
	g_nancy = this;

	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugActionRecord, "ActionRecord", "Action Record debug level");
	DebugMan.addDebugChannel(kDebugScene, "Scene", "Scene debug level");

	_console = new NancyConsole();
	_randomSource = new Common::RandomSource("Nancy");
	_randomSource->setSeed(_randomSource->getSeed());

	_input = new InputManager();
	_sound = new SoundManager();
	_graphicsManager = new GraphicsManager();
	_cursorManager = new CursorManager();

	_launchConsole = false;
}

NancyEngine::~NancyEngine() {
	clearBootChunks();
	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _randomSource;

	delete _graphicsManager;
	delete _cursorManager;
	delete _input;
	delete _sound;
}

NancyEngine *NancyEngine::create(GameType type, OSystem *syst, const NancyGameDescription *gd) {
	switch (type) {
	case kGameTypeVampire:
		return new NancyEngine(syst, gd);
	case kGameTypeNancy1:
		return new NancyEngine(syst, gd);
	case kGameTypeNancy2:
		return new NancyEngine(syst, gd);
	case kGameTypeNancy3:
		return new NancyEngine(syst, gd);
	default:
		error("Unknown GameType");
	}
}

GUI::Debugger *NancyEngine::getDebugger() {
	return _console;
}

Common::Error NancyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer ser(stream, nullptr);
	return synchronize(ser);
}

Common::Error NancyEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer ser(nullptr, stream);

	return synchronize(ser);
}

bool NancyEngine::canLoadGameStateCurrently()  {
	return canSaveGameStateCurrently();
}

bool NancyEngine::canSaveGameStateCurrently() {
	// TODO also disable during secondary movie
	return Action::PlayPrimaryVideoChan0::_activePrimaryVideo == nullptr;
}

bool NancyEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *NancyEngine::getCopyrightString() const {
	return "Copyright 1989-1997 David P Gray, All Rights Reserved.";
}

uint32 NancyEngine::getGameFlags() const {
	return _gameDescription->desc.flags;
}

const char *NancyEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

GameType NancyEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Platform NancyEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

void NancyEngine::setState(GameState state, GameState overridePrevious) {
	// Handle special cases first
	switch (state) {
	case kBoot:
		bootGameEngine();
		setState(kLogo);
		return;
	case kMainMenu:
		if (_gameFlow.currentState) {
			if (_gameFlow.currentState->onStateExit()) {
				_gameFlow.currentState = nullptr;
			}
		}

		// TODO until the game's own menus are implemented we simply open the GMM
		openMainMenuDialog();

		if (shouldQuit()) {
			return;
		}

		if (_gameFlow.currentState) {
			_gameFlow.currentState->onStateEnter();
		}

		return;
	case kCheat:
		if (_cheatTypeIsEventFlag) {
			EventFlagDialog *dialog = new EventFlagDialog();
			runDialog(*dialog);
			delete dialog;
		} else {
			CheatDialog *dialog = new CheatDialog();
			runDialog(*dialog);
			delete dialog;
		}
		_input->forceCleanInput();
		return;
	default:
		break;
	}

	_graphicsManager->clearObjects();

	_gameFlow.previousState = _gameFlow.currentState;
	_gameFlow.currentState = getStateObject(state);

	if (_gameFlow.previousState) {
		_gameFlow.previousState->onStateExit();
	}

	if (_gameFlow.currentState) {
		_gameFlow.currentState->onStateEnter();
	}

	if (overridePrevious != kNone) {
		_gameFlow.previousState = getStateObject(state);
	}
}

void NancyEngine::setPreviousState() {
	if (_gameFlow.currentState) {
		_gameFlow.currentState->onStateExit();
	}

	if (_gameFlow.previousState) {
		_gameFlow.previousState->onStateEnter();
	}

	SWAP<Nancy::State::State *>(_gameFlow.currentState, _gameFlow.previousState);
}

void NancyEngine::setMouseEnabled(bool enabled) {
	_cursorManager->showCursor(enabled); _input->setMouseInputEnabled(enabled);
}

void NancyEngine::callCheatMenu(bool eventFlags) {
	setState(kCheat), _cheatTypeIsEventFlag = eventFlags;
}

Common::Error NancyEngine::run() {
	// Boot the engine
	setState(kBoot);

	// Check if we need to load a save state from the launcher
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= getMetaEngine().getMaximumSaveSlot()) {
			// Set to Scene but do not do the loading yet
			setState(kScene);
		}
	}

	// Main loop
	while (!shouldQuit()) {
		_cursorManager->setCursorType(CursorManager::kNormalArrow);
		_input->processEvents();

		if (_gameFlow.currentState) {
			_gameFlow.currentState->process();
		}

		_graphicsManager->draw();

		if (_launchConsole) {
			_console->attach();
			_launchConsole = false;
		}
		_console->onFrame();

		_system->updateScreen();
		_system->delayMillis(16);
	}

	NancySceneState.destroy();

	return Common::kNoError;
}

void NancyEngine::bootGameEngine() {
	// Load paths
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	SearchMan.addSubDirectoryMatching(gameDataDir, "datafiles");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdvideo");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdvideo");
	SearchMan.addSubDirectoryMatching(gameDataDir, "iff");
	SearchMan.addSubDirectoryMatching(gameDataDir, "art");
	SearchMan.addSubDirectoryMatching(gameDataDir, "font");

	// Load archive
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("data1.cab");
	if (stream) {
		Common::Archive *cab = Common::makeInstallShieldArchive(stream);

		if (cab) {
			SearchMan.add("data1.hdr", cab);
		}
	}

	_resource = new ResourceManager();
	_resource->initialize();

	// Setup mixer
	syncSoundSettings();

	clearBootChunks();
	IFF *boot = new IFF("boot");
	if (!boot->load())
		error("Failed to load boot script");
	preloadCals(*boot);

	addBootChunk("BSUM", boot->getChunkStream("BSUM"));
	readBootSummary(*boot);

	// Data chunks found in BOOT. These get used in many places in the engine,
	// so we always keep them in memory
	Common::String names[] = {
		"INTR", "HINT", "LOGO", "SPUZ", "INV",
		"FONT", "MENU", "HELP", "CRED", "LOAD",
		"MAP", "CD", "TBOX", "CURS", "VIEW", "MSND",
		"BUOK", "BUDE", "BULS", "GLOB", "SLID",
		"SET", "CURT", "CANT", "TH1", "TH2",
		"QUOT", "TMOD",
		// Used in nancy2
		"FR", "LG", "OB", "CLOK", "SPEC"
	};

	Common::String persistentSounds[] = {
		"BUOK", "BUDE", "BULS", "GLOB", "CURT",
		"CANT"
	};
	
	SoundDescription desc;

	for (auto const &n : names) {
		addBootChunk(n, boot->getChunkStream(n));
	}

	// Persistent sounds that are used across the engine. These originally get loaded inside Logo
	for (auto const &s : persistentSounds) {
		Common::SeekableReadStream *str = g_nancy->getBootChunkStream(s);
		if (str) {
			desc.read(*str, SoundDescription::kNormal);
			g_nancy->_sound->loadSound(desc);
		}
	}

	delete boot;

	_graphicsManager->init();
	_cursorManager->init();
}

State::State *NancyEngine::getStateObject(GameState state) const {
	switch (state) {
	case kLogo:
		return &State::Logo::instance();
	case kCredits:
		return &State::Credits::instance();
	case kMap:
		return &State::Map::instance();
	case kHelp:
		return &State::Help::instance();
	case kScene:
		return &State::Scene::instance();
	default:
		return nullptr;
	}
}

bool NancyEngine::addBootChunk(const Common::String &name, Common::SeekableReadStream *stream) {
	if (!stream)
		return false;
	_bootChunks[name] = stream;
	return true;
}

Common::SeekableReadStream *NancyEngine::getBootChunkStream(const Common::String &name) const {
	if (_bootChunks.contains(name)) {
		return _bootChunks[name];
	} else {
		return nullptr;
	}
}

void NancyEngine::clearBootChunks() {
	for (auto const& i : _bootChunks) {
		delete i._value;
	}
	_bootChunks.clear();
}

void NancyEngine::preloadCals(const IFF &boot) {
	const byte *buf;
	uint size;
	buf = boot.getChunk(ID_PCAL, size);

	if (buf) {
		Common::MemoryReadStream stream(buf, size);
		uint16 count = stream.readUint16LE();
		debugC(1, kDebugEngine, "Preloading %d CALs", count);
		int nameLen = size / count;

		char *name = new char[nameLen];

		for (uint i = 0; i < count; i++) {
			stream.read(name, nameLen);
			name[nameLen - 1] = 0;
			debugC(1, kDebugEngine, "Preloading CAL '%s'", name);
			if (!_resource->loadCifTree(name, "cal"))
				error("Failed to preload CAL '%s'", name);
		}

		delete[] name;

		if (stream.err())
			error("Error reading PCAL chunk");
	} else
		debugC(1, kDebugEngine, "No PCAL chunk found");
}

void NancyEngine::readChunkList(const IFF &boot, Common::Serializer &ser, const Common::String &prefix) {
	byte numChunks;
	ser.syncAsByte(numChunks);
	for (byte i = 0; i < numChunks; ++ i) {
		Common::String name = Common::String::format("%s%d", prefix.c_str(), i);
		addBootChunk(name, boot.getChunkStream(name));
	}
}

void NancyEngine::readBootSummary(const IFF &boot) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	bsum->seek(0);

	// Use a serializer to handle several games' BSUMs in the same function
	Common::Serializer ser(bsum, nullptr);
	ser.setVersion(_gameDescription->gameType);

	ser.skip(0x71, kGameTypeVampire, kGameTypeVampire);
	ser.skip(0xA3, kGameTypeNancy1, kGameTypeNancy2);
	ser.syncAsUint16LE(_firstSceneID);
	ser.skip(4, kGameTypeNancy1, kGameTypeNancy2);
	ser.syncAsUint16LE(_startTimeHours, kGameTypeNancy1, kGameTypeNancy2);

	ser.skip(0xB8, kGameTypeVampire, kGameTypeVampire);
	ser.skip(0xA6, kGameTypeNancy1, kGameTypeNancy1);
	ser.skip(0xA0, kGameTypeNancy2, kGameTypeNancy2);

	// nancy3 has not been looked into, skip straight to images
	ser.skip(0xA7, kGameTypeNancy3, kGameTypeNancy3);

	readChunkList(boot, ser, "FR");
	readChunkList(boot, ser, "LG");

	if (ser.getVersion() < kGameTypeNancy3) {
		readChunkList(boot, ser, "OB");
	}

	ser.skip(0x99, kGameTypeNancy1, kGameTypeNancy1);
	int16 time = 0;
	ser.syncAsSint16LE(time, kGameTypeNancy1, kGameTypeNancy1);
	_playerTimeMinuteLength = time;
	ser.skip(2, kGameTypeNancy1, kGameTypeNancy1);
	ser.syncAsByte(_overrideMovementTimeDeltas, kGameTypeNancy1, kGameTypeNancy1);

	if (_overrideMovementTimeDeltas) {
		ser.syncAsSint16LE(time, kGameTypeNancy1, kGameTypeNancy1);
		_slowMovementTimeDelta = time;
		ser.syncAsSint16LE(time, kGameTypeNancy1, kGameTypeNancy1);
		_fastMovementTimeDelta = time;
	}
}

Common::Error NancyEngine::synchronize(Common::Serializer &ser) {
	Common::SeekableReadStream *bsum = getBootChunkStream("BSUM");
	bsum->seek(0);

	// Sync boot summary header, which includes full game title
	ser.syncVersion(kSavegameVersion);
	char buf[90];
	bsum->read(buf, 90);
	ser.matchBytes(buf, 90);

	// Sync scene and action records
	NancySceneState.synchronize(ser);
	NancySceneState._actionManager.synchronize(ser);

	return Common::kNoError;
}

} // End of namespace Nancy
