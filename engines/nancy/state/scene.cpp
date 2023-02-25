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

#include "common/serializer.h"
#include "common/config-manager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"
#include "engines/nancy/constants.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/button.h"
#include "engines/nancy/ui/ornaments.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Scene);
}

namespace Nancy {
namespace State {

void Scene::SceneSummary::read(Common::SeekableReadStream &stream) {
	char *buf = new char[0x32];

	stream.seek(0);
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	ser.syncBytes((byte *)buf, 0x32);
	description = Common::String(buf);

	ser.syncBytes((byte *)buf, 10);
	buf[9] = 0;
	videoFile = Common::String(buf);

	// skip 2 unknown bytes
	ser.skip(2);
	videoFormat = stream.readUint16LE();

	// Load the palette data in The Vampire Diaries
	ser.skip(4, kGameTypeVampire, kGameTypeVampire);
	if (ser.getVersion() == kGameTypeVampire) {
		palettes.resize(3);
		readFilename(stream, palettes[0]);
		readFilename(stream, palettes[1]);
		readFilename(stream, palettes[2]);
	}

	sound.read(stream, SoundDescription::kScene);

	ser.skip(6);
	ser.syncAsUint16LE(dontWrap);
	ser.syncAsUint16LE(numberOfVideoFrames);
	ser.syncAsUint16LE(soundPanPerFrame);
	ser.syncAsUint16LE(totalViewAngle);
	ser.syncAsUint16LE(horizontalScrollDelta);
	ser.syncAsUint16LE(verticalScrollDelta);
	ser.syncAsUint16LE(horizontalEdgeSize);
	ser.syncAsUint16LE(verticalEdgeSize);
	ser.syncAsUint16LE((uint32 &)slowMoveTimeDelta);
	ser.syncAsUint16LE((uint32 &)fastMoveTimeDelta);

	if (g_nancy->_overrideMovementTimeDeltas) {
		slowMoveTimeDelta = g_nancy->_slowMovementTimeDelta;
		fastMoveTimeDelta = g_nancy->_fastMovementTimeDelta;
	}

	delete[] buf;
}

Scene::Scene() :
		_state (kInit),
		_lastHint(-1),
		_gameStateRequested(NancyState::kNone),
		_frame(),
		_viewport(),
		_textbox(),
		_inventoryBox(),
		_menuButton(nullptr),
		_helpButton(nullptr),
		_viewportOrnaments(nullptr),
		_textboxOrnaments(nullptr),
		_actionManager(),
		_difficulty(0),
		_activePrimaryVideo(nullptr) {}

Scene::~Scene()  {
	delete _helpButton;
	delete _menuButton;
	delete _viewportOrnaments;
	delete _textboxOrnaments;
}

void Scene::process() {
	switch (_state) {
	case kInit:
		init();

		if (_state != kLoad) {
			break;
		}

		// fall through
	case kLoad:
		load();
		// fall through
	case kStartSound:
		_state = kRun;
		if (!_sceneState.doNotStartSound) {
			g_nancy->_sound->stopAndUnloadSpecificSounds();
			g_nancy->_sound->loadSound(_sceneState.summary.sound);
			g_nancy->_sound->playSound(_sceneState.summary.sound);
		}
		run(); // Extra run() call to fix the single frame with a wrong palette in TVD
		// fall through
	case kRun:
		run();
		break;
	}
}

void Scene::onStateEnter() {
	if (_state != kInit) {
		registerGraphics();
		_actionManager.onPause(false);

		g_nancy->_graphicsManager->redrawAll();

		if (getHeldItem() != -1) {
			g_nancy->_cursorManager->setCursorItemID(getHeldItem());
		}

		// Run once to clear out the previous scene when coming from Map
		process();

		g_nancy->setTotalPlayTime((uint32)_timers.pushedPlayTime);

		unpauseSceneSpecificSounds();
		g_nancy->_sound->stopSound("MSND");
	}
}

void Scene::onStateExit() {
	_timers.pushedPlayTime = g_nancy->getTotalPlayTime();
	_actionManager.onPause(true);
	pauseSceneSpecificSounds();
	_gameStateRequested = NancyState::kNone;
}

void Scene::changeScene(uint16 id, uint16 frame, uint16 verticalOffset, bool noSound) {
	if (id == 9999) {
		return;
	}

	_sceneState.nextScene.sceneID = id;
	_sceneState.nextScene.frameID = frame;
	_sceneState.nextScene.verticalOffset = verticalOffset;
	_sceneState.doNotStartSound = noSound;
	_state = kLoad;
}

void Scene::changeScene(const SceneChangeDescription &sceneDescription) {
	changeScene(sceneDescription.sceneID, sceneDescription.frameID, sceneDescription.verticalOffset, sceneDescription.doNotStartSound);
}

void Scene::pushScene() {
	_sceneState.pushedScene = _sceneState.currentScene;
	_sceneState.isScenePushed = true;
}

void Scene::popScene() {
	changeScene(_sceneState.pushedScene.sceneID, _sceneState.pushedScene.frameID, _sceneState.pushedScene.verticalOffset, true);
	_sceneState.isScenePushed = false;
}

void Scene::pauseSceneSpecificSounds() {
	// TODO missing if, same condition as the one in SoundManager::stopAndUnloadSpecificSounds

	for (uint i = 0; i < 10; ++i) {
		g_nancy->_sound->pauseSound(i, true);
	}
}

void Scene::unpauseSceneSpecificSounds() {
	for (uint i = 0; i < 10; ++i) {
		g_nancy->_sound->pauseSound(i, false);
	}
}

void Scene::addItemToInventory(uint16 id) {
	_flags.items[id] = kTrue;
	if (_flags.heldItem == id) {
		setHeldItem(-1);
	}

	_inventoryBox.addItem(id);
}

void Scene::removeItemFromInventory(uint16 id, bool pickUp) {
	_flags.items[id] = kFalse;

	if (pickUp) {
		setHeldItem(id);
	}

	_inventoryBox.removeItem(id);
}

void Scene::setHeldItem(int16 id)  {
	_flags.heldItem = id; g_nancy->_cursorManager->setCursorItemID(id);
}

void Scene::setEventFlag(int16 label, NancyFlag flag) {
	if (label > -1 && (uint)label < g_nancy->getConstants().numEventFlags) {
		_flags.eventFlags[label] = flag;
	}
}

void Scene::setEventFlag(EventFlagDescription eventFlag) {
	setEventFlag(eventFlag.label, eventFlag.flag);
}

bool Scene::getEventFlag(int16 label, NancyFlag flag) const {
	if (label > -1 && (uint)label < g_nancy->getConstants().numEventFlags) {
		return _flags.eventFlags[label] == flag;
	} else {
		return false;
	}
}

bool Scene::getEventFlag(EventFlagDescription eventFlag) const {
	return getEventFlag(eventFlag.label, eventFlag.flag);
}

void Scene::setLogicCondition(int16 label, NancyFlag flag) {
	if (label > -1) {
		_flags.logicConditions[label].flag = flag;
		_flags.logicConditions[label].timestamp = g_nancy->getTotalPlayTime();
	}
}

bool Scene::getLogicCondition(int16 label, NancyFlag flag) const {
	if (label > -1) {
		return _flags.logicConditions[label].flag == flag;
	} else {
		return false;
	}
}

void Scene::clearLogicConditions() {
	for (auto &cond : _flags.logicConditions) {
		cond.flag = kFalse;
		cond.timestamp = 0;
	}
}

void Scene::useHint(int hintID, int hintWeight) {
	if (_lastHint != hintID) {
		_hintsRemaining[_difficulty] += hintWeight;
		_lastHint = hintID;
	}
}

void Scene::registerGraphics() {
	_frame.registerGraphics();
	_viewport.registerGraphics();
	_textbox.registerGraphics();
	_inventoryBox.registerGraphics();

	if (_menuButton) {
		_menuButton->registerGraphics();
		_menuButton->setVisible(false);
	}

	if (_helpButton) {
		_helpButton->registerGraphics();
		_helpButton->setVisible(false);
	}

	if (_viewportOrnaments) {
		_viewportOrnaments->registerGraphics();
		_viewportOrnaments->setVisible(true);
	}

	if (_textboxOrnaments) {
		_textboxOrnaments->registerGraphics();
		_textboxOrnaments->setVisible(true);
	}

	_textbox.setVisible(!_shouldClearTextbox);
}

void Scene::synchronize(Common::Serializer &ser) {
	if (ser.isSaving()) {
		ser.syncAsUint16LE(_sceneState.currentScene.sceneID);
		ser.syncAsUint16LE(_sceneState.currentScene.frameID);
		ser.syncAsUint16LE(_sceneState.currentScene.verticalOffset);
	} else if (ser.isLoading()) {
		ser.syncAsUint16LE(_sceneState.nextScene.sceneID);
		ser.syncAsUint16LE(_sceneState.nextScene.frameID);
		ser.syncAsUint16LE(_sceneState.nextScene.verticalOffset);
		_sceneState.doNotStartSound = false;

		load();
	}

	ser.syncAsUint16LE(_sceneState.pushedScene.sceneID);
	ser.syncAsUint16LE(_sceneState.pushedScene.frameID);
	ser.syncAsUint16LE(_sceneState.pushedScene.verticalOffset);
	ser.syncAsByte(_sceneState.isScenePushed);

	// hardcoded number of logic conditions, check if there can ever be more/less
	for (uint i = 0; i < 30; ++i) {
		ser.syncAsUint32LE(_flags.logicConditions[i].flag);
	}

	for (uint i = 0; i < 30; ++i) {
		ser.syncAsUint32LE((uint32 &)_flags.logicConditions[i].timestamp);
	}

	// TODO hardcoded inventory size
	auto &order = getInventoryBox()._order;
	uint prevSize = getInventoryBox()._order.size();
	getInventoryBox()._order.resize(g_nancy->getConstants().numItems);

	if (ser.isSaving()) {
		for (uint i = prevSize; i < order.size(); ++i) {
			order[i] = -1;
		}
	}

	ser.syncArray(order.data(), g_nancy->getConstants().numItems, Common::Serializer::Sint16LE);

	while (order.size() && order.back() == -1) {
		order.pop_back();
	}

	if (ser.isLoading()) {
		// Make sure the shades are open if we have items
		getInventoryBox().onReorder();
	}

	// TODO hardcoded inventory size
	ser.syncArray(_flags.items.data(), g_nancy->getConstants().numItems, Common::Serializer::Byte);
	ser.syncAsSint16LE(_flags.heldItem);
	g_nancy->_cursorManager->setCursorItemID(_flags.heldItem);

	ser.syncAsUint32LE((uint32 &)_timers.lastTotalTime);
	ser.syncAsUint32LE((uint32 &)_timers.sceneTime);
	ser.syncAsUint32LE((uint32 &)_timers.playerTime);
	ser.syncAsUint32LE((uint32 &)_timers.pushedPlayTime);
	ser.syncAsUint32LE((uint32 &)_timers.timerTime);
	ser.syncAsByte(_timers.timerIsActive);
	ser.syncAsByte(_timers.timeOfDay);

	g_nancy->setTotalPlayTime((uint32)_timers.lastTotalTime);

	// TODO hardcoded number of event flags
	ser.syncArray(_flags.eventFlags.data(), g_nancy->getConstants().numEventFlags, Common::Serializer::Byte);

	ser.syncArray<uint16>(_flags.sceneHitCount, (uint16)2001, Common::Serializer::Uint16LE);

	ser.syncAsUint16LE(_difficulty);
	ser.syncArray<uint16>(_hintsRemaining.data(), _hintsRemaining.size(), Common::Serializer::Uint16LE);
	ser.syncAsSint16LE(_lastHint);

	// Synchronize SliderPuzzle static data
	ser.syncAsByte(_sliderPuzzleState.playerHasTriedPuzzle);

	byte x = 0, y = 0;

	if (ser.isSaving()) {
		y = _sliderPuzzleState.playerTileOrder.size();
		if (y) {
			x = _sliderPuzzleState.playerTileOrder.back().size();
		} else {
			x = 0;
		}
	}

	ser.syncAsByte(x);
	ser.syncAsByte(y);

	_sliderPuzzleState.playerTileOrder.resize(y);

	for (int i = 0; i < y; ++i) {
		_sliderPuzzleState.playerTileOrder[i].resize(x);
		ser.syncArray(_sliderPuzzleState.playerTileOrder[i].data(), x, Common::Serializer::Sint16LE);
	}
}

void Scene::init() {
	_flags.eventFlags = Common::Array<NancyFlag>(g_nancy->getConstants().numEventFlags, kFalse);

	// Does this ever get used?
	for (uint i = 0; i < 2001; ++i) {
		_flags.sceneHitCount[i] = 0;
	}

	_flags.items = Common::Array<NancyFlag>(g_nancy->getConstants().numItems, kFalse);

	_timers.lastTotalTime = 0;
	_timers.playerTime = g_nancy->_startTimeHours * 3600000;
	_timers.sceneTime = 0;
	_timers.timerTime = 0;
	_timers.timerIsActive = false;
	_timers.playerTimeNextMinute = 0;
	_timers.pushedPlayTime = 0;
	_timers.timeOfDay = Timers::kDay;

	changeScene(g_nancy->_firstScene);

	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("HINT");

	if (chunk) {
		chunk->seek(0);

		_hintsRemaining.clear();

		_hintsRemaining.reserve(3);
		for (uint i = 0; i < 3; ++i) {
			_hintsRemaining.push_back(chunk->readByte());
		}

		_lastHint = -1;
	}

	_sliderPuzzleState.playerHasTriedPuzzle = false;

	initStaticData();

	if (ConfMan.hasKey("save_slot")) {
		// Load savefile directly from the launcher
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= g_nancy->getMetaEngine()->getMaximumSaveSlot()) {
			g_nancy->loadGameState(saveSlot);
		}
	} else {
		// Normal boot, load default first scene
		_state = kLoad;
	}

	registerGraphics();
	g_nancy->_graphicsManager->redrawAll();
}

void Scene::setActivePrimaryVideo(Action::PlayPrimaryVideoChan0 *activeVideo) {
	_activePrimaryVideo = activeVideo;
}

Action::PlayPrimaryVideoChan0 *Scene::getActivePrimaryVideo() {
	return _activePrimaryVideo;
}

void Scene::load() {
	clearSceneData();

	// Scene IDs are prefixed with S inside the cif tree; e.g 100 -> S100
	Common::String sceneName = Common::String::format("S%u", _sceneState.nextScene.sceneID);
	IFF sceneIFF(sceneName);

	if (!sceneIFF.load()) {
		error("Faled to load IFF %s", sceneName.c_str());
	}

	Common::SeekableReadStream *sceneSummaryChunk = sceneIFF.getChunkStream("SSUM");

	if (!sceneSummaryChunk) {
		error("Invalid IFF Chunk SSUM");
	}

	_sceneState.summary.read(*sceneSummaryChunk);

	debugC(0, kDebugScene, "Loading new scene %i: description \"%s\", frame %i, vertical scroll %i, doNotStartSound == %s",
				_sceneState.nextScene.sceneID,
				_sceneState.summary.description.c_str(),
				_sceneState.nextScene.frameID,
				_sceneState.nextScene.verticalOffset,
				_sceneState.doNotStartSound == true ? "true" : "false");

	_sceneState.currentScene = _sceneState.nextScene;

	// Search for Action Records, maximum for a scene is 30
	Common::SeekableReadStream *actionRecordChunk = nullptr;

	while (actionRecordChunk = sceneIFF.getChunkStream("ACT", _actionManager._records.size()), actionRecordChunk != nullptr) {
		if (_actionManager._records.size() >= 30) {
			error("Invalid number of Action Records");
		}

		_actionManager.addNewActionRecord(*actionRecordChunk);
	}

	_viewport.loadVideo(_sceneState.summary.videoFile,
						_sceneState.currentScene.frameID,
						_sceneState.currentScene.verticalOffset,
						_sceneState.summary.dontWrap,
						_sceneState.summary.videoFormat,
						_sceneState.summary.palettes.size() ? _sceneState.summary.palettes[_sceneState.currentScene.paletteID] : Common::String());

	if (_viewport.getFrameCount() <= 1) {
		_viewport.disableEdges(kLeft | kRight);
	}

	if (_sceneState.summary.videoFormat == 1) {
		// TODO
	} else if (_sceneState.summary.videoFormat == 2) {
		// always start from the bottom
		_sceneState.currentScene.verticalOffset = _viewport.getMaxScroll();
	} else {
		error("Unrecognized Scene summary chunk video file format");
	}

	if (_sceneState.summary.videoFormat == 1) {
		// TODO
	} else if (_sceneState.summary.videoFormat == 2) {
		if (_viewport.getMaxScroll() == 0) {
			_viewport.disableEdges(kUp | kDown);
		}
	}

	_timers.sceneTime = 0;
	_sceneState.nextScene.paletteID = 0;

	_state = kStartSound;
}

void Scene::run() {
	if (_isComingFromMenu) {
		// TODO
	}

	_isComingFromMenu = false;


	if (_gameStateRequested != NancyState::kNone) {
		g_nancy->setState(_gameStateRequested);

		return;
	}

	Time currentPlayTime = g_nancy->getTotalPlayTime();

	Time deltaTime = currentPlayTime - _timers.lastTotalTime;
	_timers.lastTotalTime = currentPlayTime;

	if (_timers.timerIsActive) {
		_timers.timerTime += deltaTime;
	}

	_timers.sceneTime += deltaTime;

	// Calculate the in-game time (playerTime)
	if (currentPlayTime > _timers.playerTimeNextMinute) {
		_timers.playerTime += 60000; // Add a minute
		_timers.playerTimeNextMinute = currentPlayTime + g_nancy->_playerTimeMinuteLength;
	}

	// Set the time of day according to playerTime
	if (_timers.playerTime.getHours() >= 7 && _timers.playerTime.getHours() < 18) {
		_timers.timeOfDay = Timers::kDay;
	} else if (_timers.playerTime.getHours() >= 19 || _timers.playerTime.getHours() < 6) {
		_timers.timeOfDay = Timers::kNight;
	} else {
		_timers.timeOfDay = Timers::kDuskDawn;
	}

	// Update the UI elements and handle input
	NancyInput input = g_nancy->_input->getInput();
	_viewport.handleInput(input);
	
	_sceneState.currentScene.verticalOffset = _viewport.getCurVerticalScroll();

	if (_sceneState.currentScene.frameID != _viewport.getCurFrame()) {
		_sceneState.currentScene.frameID = _viewport.getCurFrame();
		g_nancy->_sound->calculatePanForAllSounds();
	}

	_actionManager.handleInput(input);
	_menuButton->handleInput(input);
	_helpButton->handleInput(input);
	_textbox.handleInput(input);
	_inventoryBox.handleInput(input);

	if (_menuButton->_isClicked) {
		_menuButton->_isClicked = false;
		g_nancy->_sound->playSound("GLOB");
		requestStateChange(NancyState::kMainMenu);
	}

	if (_helpButton->_isClicked) {
		_helpButton->_isClicked = false;
		g_nancy->_sound->playSound("GLOB");
		requestStateChange(NancyState::kHelp);
	}

	// Handle invisible map button
	for (uint i = 0; i < ARRAYSIZE(g_nancy->getConstants().mapAccessSceneIDs); ++i) {
		if (g_nancy->getConstants().mapAccessSceneIDs[i] == -1) {
			break;
		}

		if ((int)_sceneState.currentScene.sceneID == g_nancy->getConstants().mapAccessSceneIDs[i]) {
			if (_mapHotspot.contains(input.mousePos)) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					requestStateChange(NancyState::kMap);
				}
			}
		}
	}

	_actionManager.processActionRecords();
}

void Scene::initStaticData() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MAP");
	chunk->seek(0x8A);
	readRect(*chunk, _mapHotspot);

	chunk = g_nancy->getBootChunkStream("FR0");
	chunk->seek(0);

	_frame.init(chunk->readString());
	_viewport.init();
	_textbox.init();
	_inventoryBox.init();

	// Init menu and help buttons
	chunk = g_nancy->getBootChunkStream("BSUM");
	chunk->seek(0x184);
	Common::Rect menuSrc, helpSrc, menuDest, helpDest;
	readRect(*chunk, menuSrc);
	readRect(*chunk, helpSrc);
	readRect(*chunk, menuDest);
	readRect(*chunk, helpDest);
	_menuButton = new UI::Button(5, g_nancy->_graphicsManager->_object0, menuSrc, menuDest);
	_helpButton = new UI::Button(5, g_nancy->_graphicsManager->_object0, helpSrc, helpDest);
	_menuButton->init();
	_helpButton->init();
	g_nancy->_cursorManager->showCursor(true);

	// Init ornaments (TVD only)
	if (g_nancy->getGameType() == Nancy::GameType::kGameTypeVampire) {
		_viewportOrnaments = new UI::ViewportOrnaments(9);
		_viewportOrnaments->init();
		 
		_textboxOrnaments = new UI::TextboxOrnaments(9);
		_textboxOrnaments->init();
	}

	_state = kLoad;
}

void Scene::clearSceneData() {
	// only clear select flags
	for (uint i = 0; i < ARRAYSIZE(g_nancy->getConstants().eventFlagsToClearOnSceneChange); ++i) {
		if (g_nancy->getConstants().eventFlagsToClearOnSceneChange[i] == -1) {
			break;
		}

		_flags.eventFlags[g_nancy->getConstants().eventFlagsToClearOnSceneChange[i]] = kFalse;
	}

	clearLogicConditions();
	_actionManager.clearActionRecords();
}

} // End of namespace State
} // End of namespace Nancy
