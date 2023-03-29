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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/random.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/compression/installshield_cab.h"
#include "common/serializer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/dialogs.h"
#include "engines/nancy/console.h"
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

NancyEngine::NancyEngine(OSystem *syst, const NancyGameDescription *gd) :
		Engine(syst),
		_gameDescription(gd),
		_system(syst),
		_datFileMajorVersion(0),
		_datFileMinorVersion(1) {

	g_nancy = this;

	_randomSource = new Common::RandomSource("Nancy");
	_randomSource->setSeed(_randomSource->getSeed());

	_input = new InputManager();
	_sound = new SoundManager();
	_graphicsManager = new GraphicsManager();
	_cursorManager = new CursorManager();

	_resource = nullptr;

	_bootSummary = nullptr;
	_viewportData = nullptr;
	_inventoryData = nullptr;
	_textboxData = nullptr;
	_mapData = nullptr;
	_helpData = nullptr;
	_creditsData = nullptr;
	_hintData = nullptr;
	_clockData = nullptr;
}

NancyEngine::~NancyEngine() {
	clearBootChunks();
	delete _randomSource;

	delete _graphicsManager;
	delete _cursorManager;
	delete _input;
	delete _sound;

	delete _bootSummary;
	delete _viewportData;
	delete _inventoryData;
	delete _textboxData;
	delete _mapData;
	delete _helpData;
	delete _creditsData;
	delete _hintData;
	delete _clockData;
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

bool NancyEngine::canLoadGameStateCurrently() {
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

void NancyEngine::secondChance() {
	SaveStateList saves = getMetaEngine()->listSaves(_targetName.c_str());
	Common::String name = "SECOND CHANCE";

	// Overwrite an existing second chance if possible
	for (auto &save : saves) {
		if (save.getDescription() == name) {
			saveGameState(save.getSaveSlot(), name, true);
			return;
		}
	}

	// If no second chance slot exists, create a new one
	saveGameState(saves.size(), name, true);
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

const StaticData &NancyEngine::getStaticData() const {
	return _staticData;
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
		openMainMenuDialog();

		if (shouldQuit()) {
			return;
		}

		_input->forceCleanInput();

		return;
	}
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

	if (State::Logo::hasInstance())
		State::Logo::instance().destroy();
	if (State::Credits::hasInstance())
		State::Credits::instance().destroy();
	if (State::Map::hasInstance())
		State::Map::instance().destroy();
	if (State::Help::hasInstance())
		State::Help::instance().destroy();
	if (State::Scene::hasInstance())
		State::Scene::instance().destroy();
	if (State::MainMenu::hasInstance())
		State::MainMenu::instance().destroy();

	return Common::kNoError;
}

void NancyEngine::pauseEngineIntern(bool pause) {
	State::State *s = getStateObject(_gameFlow.curState);
	if (pause) {
		s->onStateExit();
	} else {
		s->onStateEnter();
	}
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

	// Load archive if running a compressed variant
	if (isCompressed()) {
		Common::Archive *cabinet = Common::makeInstallShieldArchive("data");
		if (cabinet) {
			SearchMan.add("data1.cab", cabinet);
		}
	}

	_resource = new ResourceManager();
	_resource->initialize();

	// Read nancy.dat
	readDatFile();

	// Setup mixer
	syncSoundSettings();

	clearBootChunks();
	IFF *boot = new IFF("boot");
	if (!boot->load())
		error("Failed to load boot script");
	preloadCals(*boot);

	// Load BOOT chunks data
	_bootSummary = new BSUM(boot->getChunkStream("BSUM"));
	_viewportData = new VIEW(boot->getChunkStream("VIEW"));
	_inventoryData = new INV(boot->getChunkStream("INV"));
	_textboxData = new TBOX(boot->getChunkStream("TBOX"));
	_helpData = new HELP(boot->getChunkStream("HELP"));
	_creditsData = new CRED(boot->getChunkStream("CRED"));

	// For now we ignore the potential for more than one of each of these
	_imageChunks.setVal("OB0", boot->getChunkStream("OB0"));
	_imageChunks.setVal("FR0", boot->getChunkStream("FR0"));
	_imageChunks.setVal("LG0", boot->getChunkStream("LG0"));

	_cursorManager->init(boot->getChunkStream("CURS"));

	_graphicsManager->init();
	_graphicsManager->loadFonts(boot->getChunkStream("FONT"));

	auto *chunkStream = boot->getChunkStream("MAP");
	if (chunkStream) {
		_mapData = new MAP(chunkStream);
	}

	chunkStream = boot->getChunkStream("HINT");
	if (chunkStream) {
		_hintData = new HINT(chunkStream);
	}

	chunkStream = boot->getChunkStream("CLOK");
	if (chunkStream) {
		_clockData = new CLOK(chunkStream);
	}

	// Load all data chunks found in BOOT. These get used in a lot of places
	// across the engine, so we always keep them in memory
	Common::Array<Common::String> bootChunkNames;
	boot->list(bootChunkNames);

	for (auto const &n : bootChunkNames) {
		addBootChunk(n, boot->getChunkStream(n));
	}

	_sound->loadCommonSounds();

	delete boot;
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

void NancyEngine::readDatFile() {
	Common::SeekableReadStream *datFile = SearchMan.createReadStreamForMember("nancy.dat");
	if (!datFile) {
		error("Unable to find nancy.dat");
	}

	if (datFile->readUint32BE() != MKTAG('N', 'N', 'C', 'Y')) {
		error("nancy.dat is invalid");
	}

	byte major = datFile->readByte();
	byte minor = datFile->readByte();
	if (major != _datFileMajorVersion || minor != _datFileMinorVersion) {
		error("Incorrect nancy.dat version. Expected '%d.%d', found %d.%d",
			_datFileMajorVersion, _datFileMinorVersion, major, minor);
	}

	uint16 numGames = datFile->readUint16LE();
	if (getGameType() > numGames) {
		warning("Data for game type %d is not in nancy.dat", numGames);
		return;
	}

	// Seek to offset containing current game
	datFile->skip((getGameType() - 1) * 4);
	datFile->seek(datFile->readUint32LE());

	_staticData.readData(*datFile, _gameDescription->desc.language);
}

Common::Error NancyEngine::synchronize(Common::Serializer &ser) {
	assert(_bootSummary);

	// Sync boot summary header, which includes full game title
	ser.syncVersion(kSavegameVersion);
	ser.matchBytes((char *)_bootSummary->header, 90);

	// Sync scene and action records
	NancySceneState.synchronize(ser);
	NancySceneState._actionManager.synchronize(ser);

	return Common::kNoError;
}

bool NancyEngine::isCompressed() {
	return getGameFlags() & GF_COMPRESSED;
}

} // End of namespace Nancy
