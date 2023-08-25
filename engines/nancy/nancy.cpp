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
#include "engines/nancy/console.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/conversation.h"

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
		_datFileMajorVersion(1),
		_datFileMinorVersion(0),
		_false(gd->gameType <= kGameTypeNancy2 ? 1 : 0),
		_true(gd->gameType <= kGameTypeNancy2 ? 2 : 1) {

	g_nancy = this;

	_randomSource = new Common::RandomSource("Nancy");
	_randomSource->setSeed(Common::RandomSource::generateNewSeed());

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
	_sliderPuzzleData = nullptr;
	_clockData = nullptr;
	_specialEffectData = nullptr;
	_raycastPuzzleData = nullptr;
	_raycastPuzzleLevelBuilderData = nullptr;
}

NancyEngine::~NancyEngine() {
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
	delete _sliderPuzzleData;
	delete _clockData;
	delete _specialEffectData;
	delete _raycastPuzzleData;
	delete _raycastPuzzleLevelBuilderData;
}

NancyEngine *NancyEngine::create(GameType type, OSystem *syst, const NancyGameDescription *gd) {
	if (type >= kGameTypeVampire && type <= kGameTypeNancy6) {
		return new NancyEngine(syst, gd);
	}

	error("Unknown GameType");
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
	return State::Scene::hasInstance() &&
			NancySceneState._state == State::Scene::kRun &&
			NancySceneState.getActiveConversation() == nullptr;
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

	if (overridePrevious != NancyState::kNone) {
		_gameFlow.prevState = overridePrevious;
	} else {
		_gameFlow.prevState = _gameFlow.curState;
	}

	_gameFlow.curState = state;
	_gameFlow.changingState = true;
}

void NancyEngine::setToPreviousState() {
	setState(_gameFlow.prevState);
}

void NancyEngine::setMouseEnabled(bool enabled) {
	_cursorManager->showCursor(enabled); _input->setMouseInputEnabled(enabled);
}

void NancyEngine::addDeferredLoader(Common::SharedPtr<DeferredLoader> &loaderPtr) {
	_deferredLoaderObjects.push_back(Common::WeakPtr<DeferredLoader>(loaderPtr));
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
		uint32 frameEndTime = _system->getMillis() + 16;

		_cursorManager->setCursorType(CursorManager::kNormalArrow);
		_input->processEvents();

		State::State *s;

		if (_gameFlow.changingState) {
			s = getStateObject(_gameFlow.curState);
			if (s) {
				s->onStateEnter(_gameFlow.curState);
			}

			_gameFlow.changingState = false;
		}

		s = getStateObject(_gameFlow.curState);
		if (s) {
			s->process();
		}

		_graphicsManager->draw();

		if (_gameFlow.changingState) {
			_graphicsManager->clearObjects();

			s = getStateObject(_gameFlow.prevState);
			if (s) {
				if(s->onStateExit(_gameFlow.prevState)) {
					destroyState(_gameFlow.prevState);
				}
			}
		}

		_system->updateScreen();

		// Use the spare time until the next frame to load larger data objects
		// Some loading is guaranteed to happen even with no time left, to ensure
		// slower systems won't be stuck waiting forever
		if (_deferredLoaderObjects.size()) {
			uint i = _deferredLoaderObjects.size() - 1;
			int32 timePerObj = (frameEndTime - g_system->getMillis()) / _deferredLoaderObjects.size();

			if (timePerObj < 0) {
				timePerObj = 0;
			}

			for (auto *iter = _deferredLoaderObjects.begin(); iter < _deferredLoaderObjects.end(); ++iter) {
				if (iter->expired()) {
					iter = _deferredLoaderObjects.erase(iter);
				} else {
					auto objectPtr = iter->lock();
					if (objectPtr) {
						if (objectPtr->load(frameEndTime - (i * timePerObj))) {
							iter = _deferredLoaderObjects.erase(iter);
						}
						--i;
					}

					if (_system->getMillis() > frameEndTime) {
						break;
					}
				}
			}
		}

		uint32 frameFinishTime = _system->getMillis();
		if (frameFinishTime < frameEndTime) {
			_system->delayMillis(frameEndTime - frameFinishTime);
		}
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

	if (s) {
		if (pause) {
			s->onStateExit(NancyState::kPause);
		} else {
			s->onStateEnter(NancyState::kPause);
		}
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
	_menuData = new MENU(boot->getChunkStream("MENU"));

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

	chunkStream = boot->getChunkStream("SPUZ");
	if (chunkStream) {
		_sliderPuzzleData = new SPUZ(chunkStream);
	}

	chunkStream = boot->getChunkStream("CLOK");
	if (chunkStream) {
		_clockData = new CLOK(chunkStream);
	}

	chunkStream = boot->getChunkStream("SPEC");
	if (chunkStream) {
		_specialEffectData = new SPEC(chunkStream);
	}

	chunkStream = boot->getChunkStream("RCPR");
	if (chunkStream) {
		_raycastPuzzleData = new RCPR(chunkStream);
	}

	chunkStream = boot->getChunkStream("RCLB");
	if (chunkStream) {
		_raycastPuzzleLevelBuilderData = new RCLB(chunkStream);
	}

	_sound->initSoundChannels();
	_sound->loadCommonSounds(boot);

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

void NancyEngine::destroyState(NancyState::NancyState state) const {
	switch (state) {
	case NancyState::kLogo:
		if (State::Logo::hasInstance()) {
			State::Logo::instance().destroy();
		}
		break;
	case NancyState::kCredits:
		if (State::Credits::hasInstance()) {
			State::Credits::instance().destroy();
		}
		break;
	case NancyState::kMap:
		if (State::Map::hasInstance()) {
			State::Map::instance().destroy();
		}
		break;
	case NancyState::kHelp:
		if (State::Help::hasInstance()) {
			State::Help::instance().destroy();
		}
		break;
	case NancyState::kScene:
		if (State::Scene::hasInstance()) {
			State::Scene::instance().destroy();
		}
		break;
	case NancyState::kMainMenu:
		if (State::MainMenu::hasInstance()) {
			State::MainMenu::instance().destroy();
		}
		break;
	default:
		break;
	}
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
	if (major != _datFileMajorVersion) {
		error("Incorrect nancy.dat version. Expected '%d.%d', found %d.%d",
			_datFileMajorVersion, _datFileMinorVersion, major, minor);
	} else {
		if (minor != _datFileMinorVersion) {
			warning("Incorrect nancy.dat version. Expected '%d.%d', found %d.%d. Game may still work, but expect bugs",
			_datFileMajorVersion, _datFileMinorVersion, major, minor);
		}
	}

	uint16 numGames = datFile->readUint16LE();
	uint16 gameType = getGameType();
	if (gameType > numGames) {
		// Fallback for when no data is present for the current game:
		// throw a warning and use the last available game data
		warning("Data for game type %d is not in nancy.dat", getGameType());
		gameType = numGames;
	}

	// Seek to offset containing current game
	datFile->skip((gameType - 1) * 4);
	uint32 thisGameOffset = datFile->readUint32LE();
	uint32 nextGameOffset = gameType == numGames ? datFile->size() : datFile->readUint32LE();
	datFile->seek(thisGameOffset);

	_staticData.readData(*datFile, _gameDescription->desc.language, nextGameOffset);
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
