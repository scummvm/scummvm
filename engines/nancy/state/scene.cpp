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

#include "engines/nancy/state/scene.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/action/actionmanager.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/time.h"
#include "engines/nancy/util.h"

#include "common/memstream.h"
#include "common/rect.h"
#include "common/func.h"

#include "graphics/surface.h"

namespace Nancy {
namespace State{

void Scene::process() {
    switch (_state) {
    case kInit:
        init();
        _state = kLoad;
        // fall through
    case kLoadNew:
        _state = kLoad;
        // fall through
    case kLoad:
        load();
        // fall through
    case kStartSound:
        _state = kRun;
        if (!_sceneState._doNotStartSound) {
            _engine->stopAndUnloadSpecificSounds();
            _engine->sound->loadSound(_sceneState.summary.sound);
            _engine->sound->playSound(_sceneState.summary.sound);
        }
        // fall through
    case kRun:
        run();
        break;
    }
}

void Scene::changeScene(uint16 id, uint16 frame, uint16 verticalOffset, bool noSound) {
    if (id == 9999) {
        return;
    }

    _sceneState.nextScene.sceneID = id;
    _sceneState.nextScene.frameID = frame;
    _sceneState.nextScene.verticalOffset = verticalOffset;
    _sceneState._doNotStartSound = noSound;
    _state = kLoadNew;
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
    // TODO missing if, same condition as the one in NancyEngine::stopAndUnloadSpecificSounds

    for (uint i = 0; i < 10; ++i) {
		_engine->sound->pauseSound(i, true);
	}
}

void Scene::unpauseSceneSpecificSounds() {
    for (uint i = 0; i < 10; ++i) {
		_engine->sound->pauseSound(i, false);
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

void Scene::registerGraphics() {
    _frame.registerGraphics();
    _viewport.registerGraphics();
    _textbox.registerGraphics();
    _inventoryBox.registerGraphics();
    _menuButton.registerGraphics();

    _engine->graphicsManager->redrawAll();

    // Used to clear the map label
    _textbox.setVisible(false);
}

void Scene::init() {
    for (uint i = 0; i < 168; ++i) {
        _flags.eventFlags[i] = kFalse;
    }

    // Does this ever get used?
    for (uint i = 0; i < 1000; ++i) {
        _sceneState.sceneHitCount[i] = 0;
    }

    for (uint i = 0; i < 11; ++i) {
        _flags.items[i] = kFalse;
    }

    _timers.playerTime = _engine->startTimeHours * 3600000;
    _timers.sceneTime = 0;
    _timers.timerTime = 0;
    _timers.timerIsActive = false;
    _timers.tickCount = 0;
    _timers.playerTimeNextMinute = 0;
    _timers.pushedPlayTime = 0;
    _timers.timeOfDay = Timers::kDay;

    _sceneState.nextScene.sceneID = _engine->firstSceneID;

    Common::SeekableReadStream *chunk = _engine->getBootChunkStream("HINT");
    chunk->seek(0);

    for (uint i = 0; i < 3; ++i) {
        _hintsRemaining.push_back(chunk->readByte());
    }

    _lastHint = -1;

    chunk = _engine->getBootChunkStream("MAP");
    chunk->seek(0x8A);
    readRect(*chunk, _mapHotspot);

    // Hardcoded by original engine
    _mapAccessSceneIDs.clear();
    _mapAccessSceneIDs.push_back(9);
    _mapAccessSceneIDs.push_back(10);
    _mapAccessSceneIDs.push_back(11);
    _mapAccessSceneIDs.push_back(0x4B0);
    _mapAccessSceneIDs.push_back(0x378);
    _mapAccessSceneIDs.push_back(0x29A);
    _mapAccessSceneIDs.push_back(0x4E2);
    _mapAccessSceneIDs.push_back(0x682);

    _frame.init("FRAME"); // TODO should be extracted from BSUM
    _viewport.init();
    _textbox.init();
    _inventoryBox.init();
    _menuButton.init();
    _helpButton.init();
    _engine->cursorManager->showCursor(true);

    _state = kLoad;
}

void Scene::load() {
    clearSceneData();

    // Scene IDs are prefixed with S inside the cif tree; e.g 100 -> S100                                                                                    
    Common::String sceneName = Common::String::format("S%u", _sceneState.nextScene.sceneID);
    IFF sceneIFF(_engine, sceneName);

	if (!sceneIFF.load()) {
		error("Faled to load IFF %s", sceneName.c_str());
	}

    Common::SeekableReadStream *sceneSummaryChunk = sceneIFF.getChunkStream("SSUM");

    if (!sceneSummaryChunk) {
        error("Invalid IFF Chunk SSUM");
    }

    readSceneSummary(*sceneSummaryChunk);

    debugC(0, kDebugScene, "Loading new scene %i: description \"%s\", frame %i, vertical scroll %i, doNotStartSound == %s",
                _sceneState.nextScene.sceneID,
                _sceneState.summary.description.c_str(),
                _sceneState.nextScene.frameID,
                _sceneState.nextScene.verticalOffset,
                _sceneState._doNotStartSound == true ? "true" : "false");

    // Search for Action Records, maximum for a scene is 30
    Common::SeekableReadStream *actionRecordChunk = nullptr;

    while (actionRecordChunk = sceneIFF.getChunkStream("ACT", _actionManager._records.size()), actionRecordChunk != nullptr) {
        if (_actionManager._records.size() >= 30) {
            error("Invalid number of Action Records");
        }

        _actionManager.addNewActionRecord(*actionRecordChunk);
    }

    _viewport.loadVideo(_sceneState.summary.videoFile, _sceneState.nextScene.frameID, _sceneState.nextScene.verticalOffset);

    // TODO TEMPORARY
    _viewport.setEdgesSize(25, 25, 25, 25);

    if (_viewport.getFrameCount() <= 1) {
        _viewport.disableEdges(kLeft | kRight);
    }

    if (!hasLoadedFromSavefile) {
        _sceneState.currentScene.verticalOffset = _sceneState.nextScene.verticalOffset;
        _sceneState.currentScene.frameID = _sceneState.nextScene.frameID;

        if (_sceneState.summary.videoFormat == 1) {
            // TODO not sure this ever gets hit
        } else if (_sceneState.summary.videoFormat == 2) {
            // always start from the bottom
            _sceneState.currentScene.verticalOffset = _viewport.getMaxScroll();
        } else {
            error("Unrecognized Scene summary chunk video file format");
        }

        // Some checks against rFrame

        if (_sceneState.summary.videoFormat == 1) {
            // TODO not sure this ever gets hit
        } else if (_sceneState.summary.videoFormat == 2) {
            if (_viewport.getMaxScroll() == 0) {
                _viewport.disableEdges(kUp | kDown);
            }
        }
    }

    _sceneState.currentScene = _sceneState.nextScene;
    _timers.sceneTime = 0;

    _state = kStartSound;
}

void Scene::run() {
    if (isComingFromMenu) {
        // TODO
    }

    isComingFromMenu = false;

    Time playTime = _engine->getTotalPlayTime();

    if (changeGameState()) {
        return;
    }

    // Do some work if we're coming from a different game state
    if (_engine->getGameState() != _engine->getPreviousGameState()) {
        if (hasLoadedFromSavefile) {
            if (playTime > _timers.pushedPlayTime) {
                _engine->setTotalPlayTime((uint32)_timers.pushedPlayTime);
                playTime = _timers.pushedPlayTime;
            }
        }

        // If the GMM was on we shouldn't reregister graphics
        if (_engine->getPreviousGameState() != Nancy::NancyEngine::kPause) {
            registerGraphics();
        }

        unpauseSceneSpecificSounds();
        _menuButton.setVisible(false);
        _helpButton.setVisible(false);

        return;
    }

    Time deltaTime = 0;

    if (_timers.tickCount < playTime) {
        deltaTime = playTime - _timers.tickCount;
        _timers.tickCount = playTime;
    }

    _timers.totalTime += deltaTime;

    if (_timers.timerIsActive)
        _timers.timerTime += deltaTime;
    _timers.sceneTime += deltaTime;

    // Calculate the in-game time (playerTime)
    if (playTime > _timers.playerTimeNextMinute) {
        _timers.playerTime += 60000; // Add a minute
        _timers.playerTimeNextMinute = playTime + playerTimeMinuteLength; // Set when we're going to add the next minute
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
    NancyInput input = _engine->input->getInput();
    _viewport.handleInput(input);
    _menuButton.handleInput(input);
    _helpButton.handleInput(input);
    _textbox.handleInput(input);
    _inventoryBox.handleInput(input);
    _actionManager.handleInput(input);

    _sceneState.currentScene.frameID = _viewport.getCurFrame();
    _sceneState.currentScene.verticalOffset = _viewport.getCurVerticalScroll();

    // Handle invisible map button
    for (uint i = 0; i < _mapAccessSceneIDs.size(); ++i) {
        if (_sceneState.currentScene.sceneID == _mapAccessSceneIDs[i]) {
            if (_mapHotspot.contains(input.mousePos)) {
                _engine->cursorManager->setCursorType(CursorManager::kHotspotArrow);

                if (input.input & NancyInput::kLeftMouseButtonUp) {
                    requestStateChange(NancyEngine::kMap);
                }
            }
        }
    }

    _actionManager.processActionRecords();
}

void Scene::readSceneSummary(Common::SeekableReadStream &stream) {
    char *buf = new char[0x32];

    stream.seek(0);
    stream.read(buf, 0x31);
    buf[32] = 0;
    _sceneState.summary.description = Common::String(buf);

    stream.seek(1, SEEK_CUR);
    stream.read(buf, 9);
    buf[9] = 0;
    _sceneState.summary.videoFile = Common::String(buf);

    // skip 1 extra byte & 2 unknown bytes
    stream.seek(3, SEEK_CUR);
    _sceneState.summary.videoFormat = stream.readUint16LE();

    _sceneState.summary.sound.read(stream, SoundDescription::kScene);

    stream.seek(0x72);
    _sceneState.summary.verticalScrollDelta = stream.readUint16LE();
    _sceneState.summary.horizontalEdgeSize = stream.readUint16LE();
    _sceneState.summary.verticalEdgeSize = stream.readUint16LE();
    _sceneState.summary.slowMoveTimeDelta = stream.readUint16LE();
    _sceneState.summary.fastMoveTimeDelta = stream.readUint16LE();

    if (_engine->overrideMovementTimeDeltas) {
        _sceneState.summary.slowMoveTimeDelta = _engine->slowMovementTimeDelta;
        _sceneState.summary.fastMoveTimeDelta = _engine->fastMovementTimeDelta;
    }

    delete[] buf;
}

bool Scene::changeGameState(bool keepGraphics) {
    if (_gameStateRequested != NancyEngine::kScene) {
        _timers.pushedPlayTime = _engine->getTotalPlayTime();
        _engine->setGameState(_gameStateRequested, keepGraphics);
        _gameStateRequested = NancyEngine::kScene;
        pauseSceneSpecificSounds();

        return true;
    }

    return false;
}

void Scene::clearSceneData() {
    // only clear select flags
    for (uint i = 44; i < 54; ++i) {
        _flags.eventFlags[i] = kFalse;
    }

    for (uint i = 63; i < 74; ++i) {
        _flags.eventFlags[i] = kFalse;
    }
    
    for (uint i = 75; i < 85; ++i) {
        _flags.eventFlags[i] = kFalse;
    }

    clearLogicConditions();
    _actionManager.clearActionRecords();
}

void Scene::setEventFlag(int16 label, NancyFlag flag) {
    if (label > -1) {
        _flags.eventFlags[label] = flag;
    }
}

void Scene::setEventFlag(EventFlagDescription eventFlag) {
    setEventFlag(eventFlag.label, eventFlag.flag);
}

bool Scene::getEventFlag(int16 label, NancyFlag flag) const {
    if (label > -1) {
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
        _flags.logicConditions[label].timestamp = _engine->getTotalPlayTime();
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

} // End of namespace State
} // End of namespace Nancy
