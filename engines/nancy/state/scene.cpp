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

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/map.h"

#include "engines/nancy/ui/button.h"
#include "engines/nancy/ui/ornaments.h"
#include "engines/nancy/ui/clock.h"

#include "engines/nancy/misc/lightning.h"

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
	ser.syncAsUint16LE(panningType);
	ser.syncAsUint16LE(numberOfVideoFrames);
	ser.syncAsUint16LE(soundPanPerFrame);
	ser.syncAsUint16LE(totalViewAngle);
	ser.syncAsUint16LE(horizontalScrollDelta);
	ser.syncAsUint16LE(verticalScrollDelta);
	ser.syncAsUint16LE(horizontalEdgeSize);
	ser.syncAsUint16LE(verticalEdgeSize);
	ser.syncAsUint16LE((uint32 &)slowMoveTimeDelta);
	ser.syncAsUint16LE((uint32 &)fastMoveTimeDelta);

	if (g_nancy->_bootSummary->overrideMovementTimeDeltas) {
		slowMoveTimeDelta = g_nancy->_bootSummary->slowMovementTimeDelta;
		fastMoveTimeDelta = g_nancy->_bootSummary->fastMovementTimeDelta;
	}

	delete[] buf;
}

Scene::Scene() :
		_state (kInit),
		_lastHintCharacter(-1),
		_lastHintID(-1),
		_gameStateRequested(NancyState::kNone),
		_frame(),
		_viewport(),
		_textbox(),
		_inventoryBox(),
		_menuButton(nullptr),
		_helpButton(nullptr),
		_viewportOrnaments(nullptr),
		_textboxOrnaments(nullptr),
		_inventoryBoxOrnaments(nullptr),
		_clock(nullptr),
		_actionManager(),
		_difficulty(0),
		_activeConversation(nullptr),
		_lightning(nullptr),
		_sliderPuzzleState(nullptr),
		_rippedLetterPuzzleState(nullptr),
		_towerPuzzleState(nullptr) {}

Scene::~Scene()  {
	delete _helpButton;
	delete _menuButton;
	delete _viewportOrnaments;
	delete _textboxOrnaments;
	delete _inventoryBoxOrnaments;
	delete _clock;
	delete _lightning;
	delete _sliderPuzzleState;
	delete _rippedLetterPuzzleState;
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
		if (_sceneState.continueSceneSound == kLoadSceneSound) {
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

void Scene::onStateEnter(const NancyState::NancyState prevState) {
	if (_state != kInit) {
		registerGraphics();

		if (prevState != NancyState::kPause) {
			g_nancy->setTotalPlayTime((uint32)_timers.pushedPlayTime);
		}

		_actionManager.onPause(false);

		g_nancy->_graphicsManager->redrawAll();

		if (getHeldItem() != -1) {
			g_nancy->_cursorManager->setCursorItemID(getHeldItem());
		}

		unpauseSceneSpecificSounds();
		g_nancy->_sound->stopSound("MSND");
	}
}

bool Scene::onStateExit(const NancyState::NancyState nextState) {
	if (nextState != NancyState::kPause) {
		_timers.pushedPlayTime = g_nancy->getTotalPlayTime();
	}
	
	_actionManager.onPause(true);
	pauseSceneSpecificSounds();
	_gameStateRequested = NancyState::kNone;

	// Re-register the clock so the open/close animation can continue playing inside Map
	if (nextState == NancyState::kMap && g_nancy->getGameType() == kGameTypeVampire) {
		_clock->registerGraphics();
	}
	
	return false;
}

void Scene::changeScene(uint16 id, uint16 frame, uint16 verticalOffset, byte continueSceneSound, int8 paletteID) {
	if (id == 9999) {
		return;
	}

	_sceneState.nextScene.sceneID = id;
	_sceneState.nextScene.frameID = frame;
	_sceneState.nextScene.verticalOffset = verticalOffset;
	_sceneState.continueSceneSound = continueSceneSound;

	if (paletteID != -1) {
		_sceneState.nextScene.paletteID = paletteID;
	}

	_state = kLoad;
}

void Scene::changeScene(const SceneChangeDescription &sceneDescription) {
	changeScene(sceneDescription.sceneID,
				sceneDescription.frameID,
				sceneDescription.verticalOffset,
				sceneDescription.continueSceneSound,
				sceneDescription.paletteID);
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
	if (g_nancy->getGameType() == kGameTypeVampire && Nancy::State::Map::hasInstance() && g_nancy->getState() != NancyState::kMap) {
		uint currentScene = _sceneState.currentScene.sceneID;
		if (currentScene == 0 || (currentScene >= 15 && currentScene <= 27)) {
			g_nancy->_sound->pauseSound(NancyMapState.getSound(), true);
		}
	}

	for (uint i = 0; i < 10; ++i) {
		g_nancy->_sound->pauseSound(i, true);
	}
}

void Scene::unpauseSceneSpecificSounds() {
	if (g_nancy->getGameType() == kGameTypeVampire && Nancy::State::Map::hasInstance()) {
		uint currentScene = _sceneState.currentScene.sceneID;
		if (currentScene == 0 || (currentScene >= 15 && currentScene <= 27)) {
			g_nancy->_sound->pauseSound(NancyMapState.getSound(), false);
		}
	}

	for (uint i = 0; i < 10; ++i) {
		g_nancy->_sound->pauseSound(i, false);
	}
}

void Scene::setPlayerTime(Time time, byte relative) {
	if (relative == kRelativeClockBump) {
		// Relative, add the specified time to current playerTime
		_timers.playerTime += time;
	} else {
		// Absolute, maintain days but replace hours and minutes
		_timers.playerTime = _timers.playerTime.getDays() * 86400000 + time;
	}

	_timers.playerTimeNextMinute = g_nancy->getTotalPlayTime() + g_nancy->_bootSummary->playerTimeMinuteLength;
}

byte Scene::getPlayerTOD() const {
	if (_timers.playerTime.getHours() >= 7 && _timers.playerTime.getHours() < 18) {
		return kPlayerDay;
	} else if (_timers.playerTime.getHours() >= 19 || _timers.playerTime.getHours() < 6) {
		return kPlayerNight;
	} else {
		return kPlayerDuskDawn;
	}
}

void Scene::addItemToInventory(uint16 id) {
	_flags.items[id] = kInvHolding;
	if (_flags.heldItem == id) {
		setHeldItem(-1);
	}

	_inventoryBox.addItem(id);
}

void Scene::removeItemFromInventory(uint16 id, bool pickUp) {
	_flags.items[id] = kInvEmpty;

	if (pickUp) {
		setHeldItem(id);
	}

	_inventoryBox.removeItem(id);
}

void Scene::setHeldItem(int16 id)  {
	_flags.heldItem = id; g_nancy->_cursorManager->setCursorItemID(id);
}

void Scene::setEventFlag(int16 label, byte flag) {
	if (label > kEvNoEvent && (uint)label < g_nancy->getStaticData().numEventFlags) {
		_flags.eventFlags[label] = flag;
	}
}

void Scene::setEventFlag(FlagDescription eventFlag) {
	setEventFlag(eventFlag.label, eventFlag.flag);
}

bool Scene::getEventFlag(int16 label, byte flag) const {
	if (label > kEvNoEvent && (uint)label < g_nancy->getStaticData().numEventFlags) {
		return _flags.eventFlags[label] == flag;
	} else {
		return false;
	}
}

bool Scene::getEventFlag(FlagDescription eventFlag) const {
	return getEventFlag(eventFlag.label, eventFlag.flag);
}

void Scene::setLogicCondition(int16 label, byte flag) {
	if (label > kEvNoEvent) {
		_flags.logicConditions[label].flag = flag;
		_flags.logicConditions[label].timestamp = g_nancy->getTotalPlayTime();
	}
}

bool Scene::getLogicCondition(int16 label, byte flag) const {
	if (label > kEvNoEvent) {
		return _flags.logicConditions[label].flag == flag;
	} else {
		return false;
	}
}

void Scene::clearLogicConditions() {
	for (auto &cond : _flags.logicConditions) {
		cond.flag = kLogNotUsed;
		cond.timestamp = 0;
	}
}

void Scene::useHint(uint16 characterID, uint16 hintID) {
	if (_lastHintID != hintID || _lastHintCharacter != characterID) {
		_hintsRemaining[_difficulty] += g_nancy->getStaticData().hints[characterID][hintID].hintWeight;
		_lastHintCharacter = characterID;
		_lastHintID = hintID;
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

	if (_inventoryBoxOrnaments) {
		_inventoryBoxOrnaments->registerGraphics();
		_inventoryBoxOrnaments->setVisible(true);
	}

	if (_clock) {
		_clock->registerGraphics();
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
		_sceneState.continueSceneSound = kLoadSceneSound;

		g_nancy->_sound->stopAllSounds();

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

	auto &order = getInventoryBox()._order;
	uint prevSize = getInventoryBox()._order.size();
	getInventoryBox()._order.resize(g_nancy->getStaticData().numItems);

	if (ser.isSaving()) {
		for (uint i = prevSize; i < order.size(); ++i) {
			order[i] = -1;
		}
	}

	ser.syncArray(order.data(), g_nancy->getStaticData().numItems, Common::Serializer::Sint16LE);

	while (order.size() && order.back() == -1) {
		order.pop_back();
	}

	if (ser.isLoading()) {
		// Make sure the shades are open if we have items
		getInventoryBox().onReorder();
	}

	ser.syncArray(_flags.items.data(), g_nancy->getStaticData().numItems, Common::Serializer::Byte);
	ser.syncAsSint16LE(_flags.heldItem);
	g_nancy->_cursorManager->setCursorItemID(_flags.heldItem);

	ser.syncAsUint32LE((uint32 &)_timers.lastTotalTime);
	ser.syncAsUint32LE((uint32 &)_timers.sceneTime);
	ser.syncAsUint32LE((uint32 &)_timers.playerTime);
	ser.syncAsUint32LE((uint32 &)_timers.pushedPlayTime);
	ser.syncAsUint32LE((uint32 &)_timers.timerTime);
	ser.syncAsByte(_timers.timerIsActive);
	ser.skip(1); // timeOfDay; To be removed on next savefile version bump

	g_nancy->setTotalPlayTime((uint32)_timers.lastTotalTime);

	ser.syncArray(_flags.eventFlags.data(), g_nancy->getStaticData().numEventFlags, Common::Serializer::Byte);

	ser.syncArray<uint16>(_flags.sceneHitCount, (uint16)2001, Common::Serializer::Uint16LE);

	ser.syncAsUint16LE(_difficulty);
	ser.syncArray<uint16>(_hintsRemaining.data(), _hintsRemaining.size(), Common::Serializer::Uint16LE);

	ser.syncAsSint16LE(_lastHintCharacter);
	ser.syncAsSint16LE(_lastHintID);

	switch (g_nancy->getGameType()) {
	case kGameTypeVampire:
		// Fall through to avoid having to bump the savegame version
		// fall through
	case kGameTypeNancy1: {
		// Synchronize SliderPuzzle static data
		if (!_sliderPuzzleState) {
			return;
		}

		ser.syncAsByte(_sliderPuzzleState->playerHasTriedPuzzle);

		byte x = 0, y = 0;

		if (ser.isSaving()) {
			y = _sliderPuzzleState->playerTileOrder.size();
			if (y) {
				x = _sliderPuzzleState->playerTileOrder.back().size();
			} else {
				x = 0;
			}
		}

		ser.syncAsByte(x);
		ser.syncAsByte(y);

		_sliderPuzzleState->playerTileOrder.resize(y);

		for (int i = 0; i < y; ++i) {
			_sliderPuzzleState->playerTileOrder[i].resize(x);
			ser.syncArray(_sliderPuzzleState->playerTileOrder[i].data(), x, Common::Serializer::Sint16LE);
		}

		break;
	}
	case kGameTypeNancy2 :
		if (!_rippedLetterPuzzleState || !_towerPuzzleState) {
			break;
		}

		ser.syncAsByte(_rippedLetterPuzzleState->playerHasTriedPuzzle);

		if (ser.isLoading()) {
			_rippedLetterPuzzleState->order.resize(24);
			_rippedLetterPuzzleState->rotations.resize(24);
		}

		ser.syncArray(_rippedLetterPuzzleState->order.data(), 24, Common::Serializer::Byte);
		ser.syncArray(_rippedLetterPuzzleState->rotations.data(), 24, Common::Serializer::Byte);

		ser.syncAsByte(_towerPuzzleState->playerHasTriedPuzzle);

		if (ser.isLoading()) {
			_towerPuzzleState->order.resize(3, Common::Array<int8>(6, -1));
		}

		for (uint i = 0; i < 3; ++i) {
			ser.syncArray(_towerPuzzleState->order[i].data(), 6, Common::Serializer::Byte);
		}

		break;
	default:
		break;
	}
}

void Scene::init() {
	_flags.eventFlags.resize(g_nancy->getStaticData().numEventFlags, kEvNotOccurred);

	// Does this ever get used?
	for (uint i = 0; i < 2001; ++i) {
		_flags.sceneHitCount[i] = 0;
	}

	_flags.items.resize(g_nancy->getStaticData().numItems, kInvEmpty);

	_timers.lastTotalTime = 0;
	_timers.playerTime = g_nancy->_bootSummary->startTimeHours * 3600000;
	_timers.sceneTime = 0;
	_timers.timerTime = 0;
	_timers.timerIsActive = false;
	_timers.playerTimeNextMinute = 0;
	_timers.pushedPlayTime = 0;

	changeScene(g_nancy->_bootSummary->firstScene);

	if (g_nancy->_hintData) {
		_hintsRemaining.clear();

		_hintsRemaining = g_nancy->_hintData->numHints;

		_lastHintCharacter = _lastHintID = -1;
	}

	// Initialize game-specific data
	switch (g_nancy->getGameType()) {
	case kGameTypeVampire:
		// Fall through to avoid having to bump the savefile version
		// fall through
	case kGameTypeNancy1:
		delete _sliderPuzzleState;
		_sliderPuzzleState = new SliderPuzzleState();
		_sliderPuzzleState->playerHasTriedPuzzle = false;
		break;
	case kGameTypeNancy2:
		delete _rippedLetterPuzzleState;
		_rippedLetterPuzzleState = new RippedLetterPuzzleState();
		_rippedLetterPuzzleState->playerHasTriedPuzzle = false;
		_rippedLetterPuzzleState->order.resize(24, 0);
		_rippedLetterPuzzleState->rotations.resize(24, 0);

		delete _towerPuzzleState;
		_towerPuzzleState = new TowerPuzzleState();
		_towerPuzzleState->playerHasTriedPuzzle = false;
		_towerPuzzleState->order.resize(3, Common::Array<int8>(6, -1));
		break;
	default:
		break;
	}

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

	if (g_nancy->getGameType() == kGameTypeVampire) {
		_lightning = new Misc::Lightning();
	}

	registerGraphics();
	g_nancy->_graphicsManager->redrawAll();
}

void Scene::setActiveConversation(Action::ConversationSound *activeConversation) {
	_activeConversation = activeConversation;
}

Action::ConversationSound *Scene::getActiveConversation() {
	return _activeConversation;
}

void Scene::beginLightning(int16 distance, uint16 pulseTime, int16 rgbPercent) {
	if (_lightning) {
		_lightning->beginLightning(distance, pulseTime, rgbPercent);
	}
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

	delete sceneSummaryChunk;

	debugC(0, kDebugScene, "Loading new scene %i: description \"%s\", frame %i, vertical scroll %i, %s",
				_sceneState.nextScene.sceneID,
				_sceneState.summary.description.c_str(),
				_sceneState.nextScene.frameID,
				_sceneState.nextScene.verticalOffset,
				_sceneState.continueSceneSound == kContinueSceneSound ? "kContinueSceneSound" : "kLoadSceneSound");

	_sceneState.currentScene = _sceneState.nextScene;

	// Search for Action Records, maximum for a scene is 30
	Common::SeekableReadStream *actionRecordChunk = nullptr;

	while (actionRecordChunk = sceneIFF.getChunkStream("ACT", _actionManager._records.size()), actionRecordChunk != nullptr) {
		if (_actionManager._records.size() >= 30) {
			error("Invalid number of Action Records");
		}

		_actionManager.addNewActionRecord(*actionRecordChunk);
		delete actionRecordChunk;
	}

	_viewport.loadVideo(_sceneState.summary.videoFile,
						_sceneState.currentScene.frameID,
						_sceneState.currentScene.verticalOffset,
						_sceneState.summary.panningType,
						_sceneState.summary.videoFormat,
						_sceneState.summary.palettes.size() ? _sceneState.summary.palettes[(byte)_sceneState.currentScene.paletteID] : Common::String());

	if (_viewport.getFrameCount() <= 1) {
		_viewport.disableEdges(kLeft | kRight);
	}

	if (_sceneState.summary.videoFormat == kSmallVideoFormat) {
		// TODO
	} else if (_sceneState.summary.videoFormat == kLargeVideoFormat) {
		// always start from the bottom
		_sceneState.currentScene.verticalOffset = _viewport.getMaxScroll();
	} else {
		error("Unrecognized Scene summary chunk video file format");
	}

	if (_sceneState.summary.videoFormat == kSmallVideoFormat) {
		// TODO
	} else if (_sceneState.summary.videoFormat == kLargeVideoFormat) {
		if (_viewport.getMaxScroll() == 0) {
			_viewport.disableEdges(kUp | kDown);
		}
	}

	_timers.sceneTime = 0;

	_state = kStartSound;
}

void Scene::run() {
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
		_timers.playerTimeNextMinute = currentPlayTime + g_nancy->_bootSummary->playerTimeMinuteLength;
	}

	handleInput();

	_actionManager.processActionRecords();

	if (_lightning) {
		_lightning->run();
	}

	if (_state == kLoad) {
		g_nancy->_graphicsManager->suppressNextDraw();
	}
}

void Scene::handleInput() {
	NancyInput input = g_nancy->_input->getInput();

	// Warp the mouse below the inactive zone during dialogue scenes
	if (_activeConversation != nullptr) {
		const Common::Rect &inactiveZone = g_nancy->_cursorManager->getPrimaryVideoInactiveZone();
		const Common::Point cursorHotspot = g_nancy->_cursorManager->getCurrentCursorHotspot();
		Common::Point adjustedMousePos = input.mousePos;
		adjustedMousePos.y -= cursorHotspot.y;

		if (inactiveZone.bottom > adjustedMousePos.y) {
			input.mousePos.y = inactiveZone.bottom + cursorHotspot.y;
			g_system->warpMouse(input.mousePos.x, input.mousePos.y);
		}
	}

	// Handle invisible map button
	// We do this first since TVD's map button overlaps the viewport's right hotspot
	for (uint16 id : g_nancy->getStaticData().mapAccessSceneIDs) {
		if ((int)_sceneState.currentScene.sceneID == id) {
			if (_mapHotspot.contains(input.mousePos)) {
				g_nancy->_cursorManager->setCursorType(g_nancy->getGameType() == kGameTypeVampire ? CursorManager::kHotspot : CursorManager::kHotspotArrow);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					requestStateChange(NancyState::kMap);

					if (g_nancy->getGameType() == kGameTypeVampire) {
						g_nancy->setMouseEnabled(false);
					}
				}

				input.eatMouseInput();
			}

			break;
		}
	}
	
	// Handle clock before viewport since it overlaps the left hotspot in TVD
	if (_clock) {
		_clock->handleInput(input);
	}

	_viewport.handleInput(input);

	_sceneState.currentScene.verticalOffset = _viewport.getCurVerticalScroll();

	if (_sceneState.currentScene.frameID != _viewport.getCurFrame()) {
		_sceneState.currentScene.frameID = _viewport.getCurFrame();
		g_nancy->_sound->calculatePanForAllSounds();
	}

	_actionManager.handleInput(input);
	_textbox.handleInput(input);
	_inventoryBox.handleInput(input);

	if (_menuButton) {
		_menuButton->handleInput(input);

		if (_menuButton->_isClicked) {
			if (_buttonPressActivationTime == 0) {
				g_nancy->_sound->playSound("BUOK");
				_buttonPressActivationTime = g_system->getMillis() + g_nancy->_bootSummary->buttonPressTimeDelay;
			} else if (g_system->getMillis() > _buttonPressActivationTime) {
				_menuButton->_isClicked = false;
				requestStateChange(NancyState::kMainMenu);
				_buttonPressActivationTime = 0;
			}
		}
	}
	
	if (_helpButton) {
		_helpButton->handleInput(input);

		if (_helpButton->_isClicked) {
			if (_buttonPressActivationTime == 0) {
				g_nancy->_sound->playSound("BUOK");
				_buttonPressActivationTime = g_system->getMillis() + g_nancy->_bootSummary->buttonPressTimeDelay;
			} else if (g_system->getMillis() > _buttonPressActivationTime) {
				_helpButton->_isClicked = false;
				requestStateChange(NancyState::kHelp);
				_buttonPressActivationTime = 0;
			}
		}
	}
}

void Scene::initStaticData() {
	_frame.init(g_nancy->_imageChunks["FR0"].imageName);
	_viewport.init();
	_textbox.init();
	_inventoryBox.init();

	// Init buttons
	BSUM *bsum = g_nancy->_bootSummary;
	assert(bsum);
	
	if (g_nancy->getGameType() == kGameTypeVampire) {
		_mapHotspot = bsum->mapButtonHotspot;
	} else if (g_nancy->_mapData) {
		_mapHotspot = g_nancy->_mapData->buttonDest;
	}

	_menuButton = new UI::Button(5, g_nancy->_graphicsManager->_object0, bsum->menuButtonSrc, bsum->menuButtonDest, bsum->menuButtonHighlightSrc);
	_helpButton = new UI::Button(5, g_nancy->_graphicsManager->_object0, bsum->helpButtonSrc, bsum->helpButtonDest, bsum->helpButtonHighlightSrc);
	g_nancy->setMouseEnabled(true);
	
	// Init ornaments and clock (TVD only)
	if (g_nancy->getGameType() == kGameTypeVampire) {
		_viewportOrnaments = new UI::ViewportOrnaments(9);
		_viewportOrnaments->init();

		_textboxOrnaments = new UI::TextboxOrnaments(9);
		_textboxOrnaments->init();

		_inventoryBoxOrnaments = new UI::InventoryBoxOrnaments(9);
		_inventoryBoxOrnaments->init();

		_clock = new UI::Clock();
		_clock->init();
	}

	if (g_nancy->getGameType() >= kGameTypeNancy2) {
		_clock = new UI::Clock();
		_clock->init();
	}

	_state = kLoad;
}

void Scene::clearSceneData() {
	// Clear generic flags only
	for (uint16 id : g_nancy->getStaticData().genericEventFlags) {
		_flags.eventFlags[id] = kEvNotOccurred;
	}

	clearLogicConditions();
	_actionManager.clearActionRecords();

	if (_lightning) {
		_lightning->endLightning();
	}
}

} // End of namespace State
} // End of namespace Nancy
