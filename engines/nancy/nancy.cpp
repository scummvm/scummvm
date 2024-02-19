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
#include "engines/nancy/cif.h"
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
#include "engines/nancy/state/setupmenu.h"
#include "engines/nancy/state/loadsave.h"
#include "engines/nancy/state/savedialog.h"

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
	_graphics = new GraphicsManager();
	_cursor = new CursorManager();
	_resource = new ResourceManager();

	_hasJustSaved = false;
}

NancyEngine::~NancyEngine() {
	destroyState(NancyState::kLogo);
	destroyState(NancyState::kCredits);
	destroyState(NancyState::kMap);
	destroyState(NancyState::kHelp);
	destroyState(NancyState::kScene);
	destroyState(NancyState::kMainMenu);
	destroyState(NancyState::kSetup);
	destroyState(NancyState::kLoadSave);
	destroyState(NancyState::kSaveDialog);

	delete _randomSource;

	delete _graphics;
	delete _cursor;
	delete _input;
	delete _sound;
	delete _resource;

	for (auto &data : _engineData) {
		delete data._value;
	}
}

NancyEngine *NancyEngine::create(GameType type, OSystem *syst, const NancyGameDescription *gd) {
	if (type >= kGameTypeVampire && type <= kGameTypeNancy11) {
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

bool NancyEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return canSaveGameStateCurrently();
}

bool NancyEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return State::Scene::hasInstance() &&
			NancySceneState._state == State::Scene::kRun &&
			NancySceneState.getActiveConversation() == nullptr &&
			NancySceneState.getActiveMovie() == nullptr &&
			!NancySceneState.isRunningAd();
}

void NancyEngine::secondChance() {
	uint secondChanceSlot = getMetaEngine()->getMaximumSaveSlot();
	saveGameState(secondChanceSlot, "SECOND CHANCE", true);
}

void NancyEngine::errorString(const char *buf_input, char *buf_output, int buf_output_size) {
	if (State::Scene::hasInstance()) {
		if (NancySceneState._state == State::Scene::kLoad) {
			// Error while loading scene
			snprintf(buf_output, buf_output_size, "While loading scene S%u, frame %u, action record %u:\n%s",
				NancySceneState._sceneState.currentScene.sceneID,
				NancySceneState._sceneState.currentScene.frameID,
				NancySceneState._actionManager.getActionRecords().size(),
				buf_input);
		} else {
			// Error while running
			snprintf(buf_output, buf_output_size, "In current scene S%u, frame %u:\n%s",
				NancySceneState._sceneState.currentScene.sceneID,
				NancySceneState._sceneState.currentScene.frameID,
				buf_input);
		}
	} else {
		strncpy(buf_output, buf_input, buf_output_size);
		if (buf_output_size > 0)
			buf_output[buf_output_size - 1] = '\0';
	}
}

bool NancyEngine::hasFeature(EngineFeature f) const {
	return	(f == kSupportsReturnToLauncher) ||
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

Common::Language NancyEngine::getGameLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform NancyEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

const StaticData &NancyEngine::getStaticData() const {
	return _staticData;
}

const EngineData *NancyEngine::getEngineData(const Common::String &name) const {
	if (_engineData.contains(name)) {
		return _engineData[name];
	}

	return nullptr;
}

void NancyEngine::setState(NancyState::NancyState state, NancyState::NancyState overridePrevious) {
	// Handle special cases first
	switch (state) {
	case NancyState::kBoot:
		bootGameEngine();
		setState(NancyState::kLogo);
		return;
	case NancyState::kMainMenu: {
		if (!ConfMan.hasKey("original_menus") || ConfMan.getBool("original_menus")) {
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

	_gameFlow.nextState = state;
	_gameFlow.changingState = true;
}

void NancyEngine::setToPreviousState() {
	setState(_gameFlow.prevState);
}

void NancyEngine::setMouseEnabled(bool enabled) {
	_cursor->showCursor(enabled); _input->setMouseInputEnabled(enabled);
}

void NancyEngine::addDeferredLoader(Common::SharedPtr<DeferredLoader> &loaderPtr) {
	_deferredLoaderObjects.push_back(Common::WeakPtr<DeferredLoader>(loaderPtr));
}

Common::Error NancyEngine::run() {
	setDebugger(new NancyConsole());

	// Set the default number of saves for earlier games
	if (!ConfMan.hasKey("nancy_max_saves", ConfMan.getActiveDomainName())) {
		if (getGameType() <= kGameTypeNancy7) {
			ConfMan.setInt("nancy_max_saves", 8, ConfMan.getActiveDomainName());
		}
	}

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

	bool graphicsWereSuppressed = false;

	// Main loop
	while (true) {
		_input->processEvents();
		if (shouldQuit()) {
			break;
		}

		uint32 frameEndTime = _system->getMillis() + 16;

		if (!graphicsWereSuppressed) {
			_cursor->setCursorType(CursorManager::kNormalArrow);
		}

		State::State *s;

		if (_gameFlow.changingState) {
			_gameFlow.curState = _gameFlow.nextState;
			_gameFlow.nextState = NancyState::kNone;

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

		graphicsWereSuppressed = _graphics->_isSuppressed;

		_graphics->draw();

		if (_gameFlow.changingState) {
			_graphics->clearObjects();

			s = getStateObject(_gameFlow.curState);
			if (s) {
				if (s->onStateExit(_gameFlow.nextState)) {
					destroyState(_gameFlow.curState);
				}
			}
		}

		_system->updateScreen();

		// In cases where the graphics were not drawn for a frame, we want to make sure the next
		// frame is processed as fast as possible. Thus, we skip deferred loaders and the time
		// delay that normally maintains 60fps
		if (!graphicsWereSuppressed) {
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
	}

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

	Engine::pauseEngineIntern(pause);
}

void NancyEngine::bootGameEngine() {
	// Load paths
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
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

	_resource->readCifTree("ciftree", "dat", 1);
	_resource->readCifTree("promotree", "dat", 1);

	// Read nancy.dat
	readDatFile();

	// Setup mixer
	syncSoundSettings();

	if (getGameType() >= kGameTypeNancy10) {
		error("Game not supported; Use console to inspect game data");
	}

	IFF *iff = _resource->loadIFF("boot");
	if (!iff)
		error("Failed to load boot script");

	// Load BOOT chunks data
	Common::SeekableReadStream *chunkStream = nullptr;
	#define LOAD_BOOT_L(t, s) if (chunkStream = iff->getChunkStream(s), chunkStream) {	\
								_engineData.setVal(s, new t(chunkStream));				\
								delete chunkStream;										\
							}
	#define LOAD_BOOT(t) LOAD_BOOT_L(t, #t)

	LOAD_BOOT_L(ImageChunk, "OB0")
	LOAD_BOOT_L(ImageChunk, "FR0")
	LOAD_BOOT_L(ImageChunk, "LG0")

	// One weird version of nancy3 has a partner logo implemented the same way as the other image chunks
	LOAD_BOOT_L(ImageChunk, "PLG0")

	// For all other games (starting with nancy4) the partner logo is a larger struct,
	// containing video and sound data as well. Those go unused, however, so we still
	// treat is as a simple image. Note the O instead of the 0 above.
	LOAD_BOOT_L(ImageChunk, "PLGO")

	LOAD_BOOT(BSUM) // This checks for PLG0, do NOT reorder
	LOAD_BOOT(VIEW)
	LOAD_BOOT(PCAL)
	LOAD_BOOT(INV)
	LOAD_BOOT(TBOX)
	LOAD_BOOT(HELP)
	LOAD_BOOT(CRED)
	LOAD_BOOT(MENU)
	LOAD_BOOT(SET)
	LOAD_BOOT(SDLG)
	LOAD_BOOT(MAP)
	LOAD_BOOT(HINT)
	LOAD_BOOT(SPUZ)
	LOAD_BOOT(CLOK)
	LOAD_BOOT(SPEC)
	LOAD_BOOT(RCPR)
	LOAD_BOOT(RCLB)
	LOAD_BOOT(TABL)
	LOAD_BOOT(MARK)

	if (g_nancy->getGameType() <= kGameTypeNancy7) {
		LOAD_BOOT(LOAD)
	} else {
		// nancy8 has a completely new save/load screen
		LOAD_BOOT_L(LOAD_v2, "LOAD")
	}

	_cursor->init(iff->getChunkStream("CURS"));

	_graphics->init();
	_graphics->loadFonts(iff->getChunkStream("FONT"));

	preloadCals();

	_sound->initSoundChannels();
	_sound->loadCommonSounds(iff);

	delete iff;

	// Load convo texts and autotext
	auto *bsum = GetEngineData(BSUM);
	if (bsum && !bsum->conversationTextsFilename.empty() && !bsum->autotextFilename.empty()) {
		iff = _resource->loadIFF(bsum->conversationTextsFilename);
		if (!iff) {
			error("Could not load CONVO IFF");
		}

		if (chunkStream = iff->getChunkStream("CVTX"), chunkStream) {
			_engineData.setVal("CONVO", new CVTX(chunkStream));
			delete chunkStream;
		}

		delete iff;

		iff = _resource->loadIFF(bsum->autotextFilename);
		if (!iff) {
			error("Could not load AUTOTEXT IFF");
		}

		if (chunkStream = iff->getChunkStream("CVTX"), chunkStream) {
			_engineData.setVal("AUTOTEXT", new CVTX(chunkStream));
			delete chunkStream;
		}

		delete iff;
	}

	#undef LOAD_BOOT_L
	#undef LOAD_BOOT
}

State::State *NancyEngine::getStateObject(NancyState::NancyState state) const {
	switch (state) {
	case NancyState::kLogo:
		return &State::Logo::instance();
	case NancyState::kCredits:
		return &State::Credits::instance();
	case NancyState::kMap:
		return &State::Map::instance();
	case NancyState::kSetup:
		return &State::SetupMenu::instance();
	case NancyState::kHelp:
		return &State::Help::instance();
	case NancyState::kScene:
		return &State::Scene::instance();
	case NancyState::kMainMenu:
		return &State::MainMenu::instance();
	case NancyState::kLoadSave:
		return &State::LoadSaveMenu::instance();
	case NancyState::kSaveDialog:
		return &State::SaveDialog::instance();
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
	case NancyState::kSetup:
		if (State::SetupMenu::hasInstance()) {
			State::SetupMenu::instance().destroy();
		}
		break;
	case NancyState::kLoadSave:
		if (State::LoadSaveMenu::hasInstance()) {
			State::LoadSaveMenu::instance().destroy();
		}
		break;
	case NancyState::kSaveDialog:
		if (State::SaveDialog::hasInstance()) {
			State::SaveDialog::instance().destroy();
		}
		break;
	default:
		break;
	}
}

void NancyEngine::preloadCals() {
	auto *pcal = GetEngineData(PCAL);
	if (!pcal) {
		// CALs only appeared in nancy2 so a PCAL chunk may not exist
		return;
	}

	for (const Common::String &name : pcal->calNames) {
		if (!_resource->readCifTree(name, "cal", 2)) {
			error("Failed to preload CAL '%s'", name.c_str());
		}
	}
}

void NancyEngine::readDatFile() {
	Common::SeekableReadStream *datFile = SearchMan.createReadStreamForMember("nancy.dat");
	if (!datFile) {
		error("Unable to find nancy.dat");
	}

	if (datFile->readUint32BE() != MKTAG('N', 'N', 'C', 'Y')) {
		error("nancy.dat is invalid");
	}

	int8 major = datFile->readSByte();
	int8 minor = datFile->readSByte();
	if (major != _datFileMajorVersion) {
		error("Incorrect nancy.dat version. Expected '%d.%d', found %d.%d",
			_datFileMajorVersion, _datFileMinorVersion, major, minor);
	} else {
		if (minor < _datFileMinorVersion) {
			warning("Incorrect nancy.dat version. Expected at least '%d.%d', found %d.%d. Game may still work, but expect bugs",
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

	_staticData.readData(*datFile, _gameDescription->desc.language, nextGameOffset, major, minor);

	delete datFile;
}

Common::Error NancyEngine::synchronize(Common::Serializer &ser) {
	auto *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	// Sync boot summary header, which includes full game title
	ser.syncVersion(kSavegameVersion);
	ser.matchBytes((const char *)bootSummary->header, 90);

	// Sync scene and action records
	NancySceneState.synchronize(ser);
	NancySceneState._actionManager.synchronize(ser);

	return Common::kNoError;
}

bool NancyEngine::isCompressed() {
	return getGameFlags() & GF_COMPRESSED;
}

} // End of namespace Nancy
