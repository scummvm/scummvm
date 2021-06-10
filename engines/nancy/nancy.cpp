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

#include "common/system.h"
#include "common/random.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/installshield_cab.h"
#include "common/serializer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/dialogs.h"
#include "engines/nancy/console.h"
#include "engines/nancy/constants.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/primaryvideo.h"

#include "engines/nancy/state/logo.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/help.h"
#include "engines/nancy/state/map.h"
#include "engines/nancy/state/credits.h"
#include "engines/nancy/state/mainmenu.h"

namespace Nancy {

NancyEngine *g_nancy;

NancyEngine::NancyEngine(OSystem *syst, const NancyGameDescription *gd) : Engine(syst), _gameDescription(gd), _system(syst) {
	g_nancy = this;

	_randomSource = new Common::RandomSource("Nancy");
	_randomSource->setSeed(_randomSource->getSeed());

	_input = new InputManager();
	_sound = new SoundManager();
	_graphicsManager = new GraphicsManager();
	_cursorManager = new CursorManager();

	_resource = nullptr;
	_startTimeHours = 0;
	_overrideMovementTimeDeltas = false;
	_cheatTypeIsEventFlag = false;
	_horizontalEdgesSize = 0;
	_verticalEdgesSize = 0;
}

NancyEngine::~NancyEngine() {
	clearBootChunks();
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
	return State::Scene::hasInstance() && NancySceneState.getActivePrimaryVideo() == nullptr;
}

bool NancyEngine::canSaveAutosaveCurrently() {
	if (ConfMan.getBool("second_chance")) {
		return false;
	} else {
		return Engine::canSaveAutosaveCurrently();
	}
}

bool NancyEngine::hasFeature(EngineFeature f) const {
	return  (f == kSupportsReturnToLauncher) ||
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime) ||
			(f == kSupportsChangingOptionsDuringRuntime) ||
			(f == kSupportsSubtitleOptions);
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

const GameConstants &NancyEngine::getConstants() const {
	return gameConstants[getGameType() - 1];
}

void NancyEngine::setState(NancyState::NancyState state, NancyState::NancyState overridePrevious) {
	// Handle special cases first
	switch (state) {
	case NancyState::kBoot:
		bootGameEngine();
		setState(NancyState::kLogo);
		return;
	case NancyState::kMainMenu: {
		if (ConfMan.getBool("original_menus")) {
			break;
		}

		// Do not use the original engine's menus, call the GMM instead
		State::State *s = getStateObject(_gameFlow.curState);
		if (s) {
			s->onStateExit();
		}

		// TODO until the game's own menus are implemented we simply open the GMM
		openMainMenuDialog();

		if (shouldQuit()) {
			return;
		}

		s = getStateObject(_gameFlow.curState);
		if (s) {
			s->onStateEnter();
		}

		_input->forceCleanInput();

		return;
	}
	case NancyState::kCheat:
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

	if (overridePrevious != NancyState::kNone) {
		_gameFlow.prevState = overridePrevious;
	} else {
		_gameFlow.prevState = _gameFlow.curState;
	}

	_gameFlow.curState = state;

	State::State *s = getStateObject(_gameFlow.prevState);
	if (s) {
		s->onStateExit();
	}

	s = getStateObject(_gameFlow.curState);
	if (s) {
		s->onStateEnter();
	}
}

void NancyEngine::setToPreviousState() {
	State::State *s = getStateObject(_gameFlow.curState);
	if (s) {
		s->onStateExit();
	}

	s = getStateObject(_gameFlow.prevState);
	if (s) {
		s->onStateEnter();
	}

	SWAP<NancyState::NancyState>(_gameFlow.curState, _gameFlow.prevState);
}

void NancyEngine::setMouseEnabled(bool enabled) {
	_cursorManager->showCursor(enabled); _input->setMouseInputEnabled(enabled);
}

void NancyEngine::callCheatMenu(bool eventFlags) {
	_cheatTypeIsEventFlag = eventFlags;
	setState(NancyState::kCheat);
}

Common::Error NancyEngine::run() {
	setDebugger(new NancyConsole());

	// Boot the engine
	setState(NancyState::kBoot);

	// Check if we need to load a save state from the launcher
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= getMetaEngine()->getMaximumSaveSlot()) {
			// Set to Scene but do not do the loading yet
			setState(NancyState::kScene);
		}
	}

	// Main loop
	while (!shouldQuit()) {
		_cursorManager->setCursorType(CursorManager::kNormalArrow);
		_input->processEvents();

		State::State *s = getStateObject(_gameFlow.curState);
		if (s) {
			s->process();
		}

		_graphicsManager->draw();

		_system->updateScreen();
		_system->delayMillis(16);
	}

	if (State::Scene::hasInstance()) {
		NancySceneState.destroy();
	}

	return Common::kNoError;
}

void NancyEngine::bootGameEngine() {
	// Load paths
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	SearchMan.addSubDirectoryMatching(gameDataDir, "datafiles");
	SearchMan.addSubDirectoryMatching(gameDataDir, "ciftree");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdvideo");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdvideo");
	SearchMan.addSubDirectoryMatching(gameDataDir, "iff");
	SearchMan.addSubDirectoryMatching(gameDataDir, "art");
	SearchMan.addSubDirectoryMatching(gameDataDir, "font");

	// Register default settings
	ConfMan.registerDefault("player_speech", true);
	ConfMan.registerDefault("character_speech", true);
	ConfMan.registerDefault("original_menus", false);
	ConfMan.registerDefault("second_chance", false);

	// Load archive if running a compressed variant
	if (isCompressed()) {
		Common::Archive *cabinet = Common::makeInstallShieldArchive("data");
		if (cabinet) {
			SearchMan.add("data1.cab", cabinet);
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
		"CLOK", "SPEC"
	};

	for (auto const &n : names) {
		addBootChunk(n, boot->getChunkStream(n));
	}

	_sound->loadCommonSounds();

	delete boot;

	_graphicsManager->init();
	_cursorManager->init();
}

State::State *NancyEngine::getStateObject(NancyState::NancyState state) const {
	switch (state) {
	case NancyState::kLogo:
		return &State::Logo::instance();
	case NancyState::kCredits:
		return &State::Credits::instance();
	case NancyState::kMap:
		return &State::Map::instance();
	case NancyState::kHelp:
		return &State::Help::instance();
	case NancyState::kScene:
		return &State::Scene::instance();
	case NancyState::kMainMenu:
		return &State::MainMenu::instance();
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
	byte numChunks = 0;
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
	ser.skip(0xA3, kGameTypeNancy1, kGameTypeNancy1);
	ser.skip(0x9D, kGameTypeNancy2, kGameTypeNancy3);
	ser.syncAsUint16LE(_firstScene.sceneID);
	ser.skip(12, kGameTypeVampire, kGameTypeVampire); // Palette
	ser.syncAsUint16LE(_firstScene.frameID);
	ser.syncAsUint16LE(_firstScene.verticalOffset);
	ser.syncAsUint16LE(_startTimeHours);
	ser.syncAsUint16LE(_startTimeMinutes);

	ser.skip(0xA4, kGameTypeVampire, kGameTypeNancy2);

	readChunkList(boot, ser, "FR"); // frames
	readChunkList(boot, ser, "LG"); // logos

	if (ser.getVersion() == kGameTypeNancy3) {
		readChunkList(boot, ser, "PLG"); // partner logos
	}
	
	readChunkList(boot, ser, "OB"); // objects

	ser.skip(0x28, kGameTypeVampire, kGameTypeVampire);
	ser.skip(0x10, kGameTypeNancy1, kGameTypeNancy1);
	ser.skip(0x20, kGameTypeNancy2, kGameTypeNancy3);
	readRect(*bsum, _textboxScreenPosition);

	ser.skip(0x5E, kGameTypeVampire, kGameTypeVampire);
	ser.skip(0x59, kGameTypeNancy1, kGameTypeNancy1);
	ser.skip(0x89, kGameTypeNancy2, kGameTypeNancy3);
	ser.syncAsUint16LE(_horizontalEdgesSize);
	ser.syncAsUint16LE(_verticalEdgesSize);
	ser.skip(0x1C);
	int16 time = 0;
	ser.syncAsSint16LE(time);
	_playerTimeMinuteLength = time;
	ser.skip(2, kGameTypeNancy1, kGameTypeNancy3);
	ser.syncAsByte(_overrideMovementTimeDeltas);

	if (_overrideMovementTimeDeltas) {
		ser.syncAsSint16LE(time);
		_slowMovementTimeDelta = time;
		ser.syncAsSint16LE(time);
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

bool NancyEngine::isCompressed() {
	return getGameFlags() & GF_COMPRESSED;
}

} // End of namespace Nancy
