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

#include "engines/nancy/scene.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/logic.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"

#include "common/memstream.h"
#include "common/rect.h"
#include "common/func.h"

#include "graphics/surface.h"

namespace Nancy {

SceneManager::~SceneManager() {
    clearSceneData();
}

void SceneManager::process() {
    switch (_state) {
    case kInit:
        init();
        break;
    case kLoad:
        load();
        break;
    case kStartSound:
        // TODO
        break;
    case kRun:
        run();
        break;
    case kLoadNew:
        _state = kLoad;
        break;
    }
}

void SceneManager::init() {
    for (uint i = 0; i < 672; ++i) {
        _engine->playState.eventFlags[i] = PlayState::Flag::kFalse;
    }

    for (uint i = 0; i < 1000; ++i) {
        _engine->playState.sceneHitCount[i] = 0;
    }

    _sceneID = _engine->_firstSceneID;
    _engine->_gameFlow.previousGameState = NancyEngine::kScene;

    // TODO init action records
    
    _engine->graphics->clearZRenderStructs();

    // Load the primary frame
    if (!_engine->_res->loadImage("ciftree", _engine->_frames[0].name, _engine->graphics->_primaryFrameSurface)) {
        error("Failed to load %s", _engine->_frames[0].name.c_str());
    }

    char *name = new char[10];

    // Load the Object 0 image
    if (!_engine->_res->loadImage("ciftree", _engine->_objects[0].name, _engine->graphics->_object0Surface)) {
        error("Failed to load %s", _engine->_objects[0].name.c_str());
    }

    // Load inventory icons
    Common::SeekableReadStream *inv = _engine->getBootChunkStream("INV");
    inv->seek(0x1C8);
    inv->read(name, 10); // should be TOOL
    if (!_engine->_res->loadImage("ciftree", Common::String(name), _engine->graphics->_inventoryBoxIconsSurface)) {
        error("Failed to load inventory icons (TOOL)");
    }

    // Load the cursors
    inv->read(name, 10); // should be TOOLCUR1
    if (!_engine->_res->loadImage("ciftree", Common::String(name), _engine->graphics->_inventoryCursorsSurface)) {
        error("Failed to load cursors (TOOLCUR1)");
    }

    delete[] name;

    _engine->graphics->initSceneZRenderStructs();

    _state = kLoad;
}

void SceneManager::load() {
    // Scene IDs are prefixed with S inside the cif tree; e.g 100 -> S100                                                                                    
    Common::String sceneName = Common::String::format("S%u", _sceneID);
    IFF sceneIFF(_engine, sceneName);
	if (!sceneIFF.load()) {
		error("Faled to load IFF %s", sceneName.c_str());
	}

    Common::SeekableReadStream *sceneSummaryChunk = sceneIFF.getChunkStream("SSUM");
    if (!sceneSummaryChunk) {
        error("Invalid IFF Chunk SSUM");
    }

    currentScene = SceneSummary(sceneSummaryChunk);

    // The check to see if we need to switch the CD is performed here

    // Not sure what these do yet
    Common::SeekableReadStream *bsum = _engine->getBootChunkStream("BSUM");
    bsum->seek(0x1F1);
    byte overrideMovementDeltas = bsum->readByte();
    if (overrideMovementDeltas) {
        currentScene.slowMoveTimeDelta = bsum->readUint16LE();
        currentScene.fastMoveTimeDelta = bsum->readUint16LE();
    }

    // Search for Action Records, maximum for a scene is 30
    Common::SeekableReadStream *actionRecordChunk = nullptr;

    while (actionRecordChunk = sceneIFF.getChunkStream("ACT", _engine->logic->_records.size()), actionRecordChunk != nullptr)
    {
        if (_engine->logic->_records.size() >= 30) {
            error("Invalid number of Action Records");
        }

        _engine->logic->addNewActionRecord(*actionRecordChunk);
    }
    _engine->graphics->loadBackgroundVideo(currentScene.videoFile);
    if (_engine->graphics->getBackgroundFrameCount() == 1) {
        currentScene.horizontalEdgeSize = 0;
    }

    View &viewportDesc = _engine->graphics->viewportDesc;

    if (!hasLoadedFromSavefile) {
        _engine->playState.currentMaxVerticalScroll = _engine->playState.queuedMaxVerticalScroll;
        _engine->playState.currentViewFrame = _engine->playState.queuedViewFrame;

        if (currentScene.videoFormat == 1) {
            // TODO not sure this ever gets hit
        } else if (currentScene.videoFormat == 2) {
            // always start from the bottom
            _engine->playState.verticalScroll = _engine->playState.currentMaxVerticalScroll;
            _engine->playState.verticalScrollDelta = currentScene.verticalScrollDelta;
        } else {
            error("Unrecognized Scene summary chunk video file format");
        }

        // Some checks against rFrame

        if (currentScene.videoFormat == 1) {
            // TODO not sure this ever gets hit
        } else if (currentScene.videoFormat == 2) {
            if (_engine->graphics->getBackgroundHeight() == (uint32)viewportDesc.f2Dest.bottom + 1) {
                currentScene.verticalEdgeSize = 0;
            }
        }
    }

    _engine->input->setPointerBitmap(0, 0, false);

    _state = kRun; // TODO temp, is actually StartSound
}

void SceneManager::run() {
    if (isComingFromMenu) {
        // TODO
    }
    isComingFromMenu = false;

    if (orderingPuzzleIsActive)
        _engine->graphics->getZRenderStruct("ORDERING PUZZLE").isActive = true;
    if (rotatingLockPuzzleIsActive)
        _engine->graphics->getZRenderStruct("ROTATING LOCK PUZZLE").isActive = true;
    if (leverPuzzleIsActive)
        _engine->graphics->getZRenderStruct("LEVER PUZZLE").isActive = true;
    if (sliderPuzzleIsActive)
        _engine->graphics->getZRenderStruct("SLIDER PUZZLE").isActive = true;
    if (passwordPuzzleIsActive)
        _engine->graphics->getZRenderStruct("PASSWORD PUZZLE").isActive = true;
    if (telephoneIsActive)
        _engine->graphics->getZRenderStruct("TELEPHONE").isActive = true;

    if (helpMenuRequested) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kHelp;
        // _engine->helpMenu->state = HelpMenu::State::kInit;
        helpMenuRequested = false;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (mainMenuRequested) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kMainMenu;
        // _engine->mainMenu->state = MainMenu::State::kInit;
        mainMenuRequested = false;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (saveLoadRequested) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kLoadSave;
        // _engine->loadSaveMenu->state = LoadSaveMenu::State::kInit;
        saveLoadRequested = false;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (saveReloadRequested) {
        // TODO
    }

    if (setupMenuRequested) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kSetup;
        // _engine->setupMenu->state = setupMenu::State::kInit;
        setupMenuRequested = false;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (creditsSequenceRequested) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kHelp;
        // _engine->credits->state = CreditsSequence::State::kInit;
        creditsSequenceRequested = false;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (mapScreenRequested) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kHelp;
        // _engine->map->state = Map::State::kInit;
        mapScreenRequested = false;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    // Cheat menu, will not implement

    uint32 playTimeThisFrame = _engine->getTotalPlayTime();

    // Do some work if we're coming from a different game state
    if (_engine->_gameFlow.previousGameState != NancyEngine::GameState::kScene) {
        if (hasLoadedFromSavefile) {
            if (playTimeThisFrame > _stashedTickCount) {
                playTimeThisFrame -= _stashedTickCount;
                _engine->playState.totalTime -= playTimeThisFrame;
                _engine->playState.sceneTime -= playTimeThisFrame;
                if (_engine->playState.timerIsActive)
                    _engine->playState.timerTime -= playTimeThisFrame;
            }
        }
            
        _engine->graphics->getZRenderStruct("MENU BTN DN").isActive = false;
        _engine->graphics->getZRenderStruct("HELP BTN DN").isActive = false;
        _engine->input->hoveredElementID = -1;
        // TODO a bunch of function calls
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        return;
    }

    uint32 diff = 0;
    if (_tickCount < playTimeThisFrame) {
        diff = playTimeThisFrame - _tickCount;
        _tickCount = playTimeThisFrame;
    }
    _engine->playState.totalTime += diff;
    if (_engine->playState.timerIsActive)
        _engine->playState.timerTime += diff;
    _engine->playState.sceneTime =+ diff;

    // Calculate the in-game time (playerTime)
    if (playTimeThisFrame > _engine->playState.playerTimeNextMinute) {
        _engine->playState.playerTime += 60000; // Add a minute
        _engine->playState.playerTimeNextMinute = playTimeThisFrame + playerTimeMinuteLength; // Set when we're going to add the next minute
    }

    // Set the time of day according to playerTime
    if (_engine->playState.playerTime.getHours_alt() >= 7 && _engine->playState.playerTime.getHours_alt() < 18) {
        _engine->playState.timeOfDay = _engine->playState.kDay;
    } else if ((_engine->playState.playerTime.getHours_alt() >= 19 || _engine->playState.playerTime.getHours_alt() < 6)) {
        _engine->playState.timeOfDay = _engine->playState.kNight;
    } else {
        _engine->playState.timeOfDay = _engine->playState.kDuskDawn;
    }

    if (_engine->input->isClickValidLMB) {
        if (orderingPuzzleIsActive) {
            // TODO
        }

        if (sliderPuzzleIsActive) {
            // TODO
        }

        if (orderingPuzzleIsActive) {
            // TODO
        }

        if (telephoneIsActive) {
            // TODO
        }

        if (leverPuzzleIsActive) {
            // TODO   
        }

        if (passwordPuzzleIsActive) {

        }

        int16 &hovered = _engine->input->hoveredElementID;

        if (hovered == InputManager::mapButtonID) {
            // TODO another if
            mapScreenRequested = true;
            return;
        }

        if (hovered == InputManager::textBoxID) {
            // TODO
        }

        if (hovered == InputManager::inventoryItemTakeID) {
            // TODO
        }

        if (hovered == InputManager::inventoryItemReturnID) {
            // TODO
        }

        if (hovered == InputManager::textBoxScrollbarID) {
            // TODO
        }

        if (hovered == InputManager::inventoryScrollbarID) {
            // TODO
        }

        if (hovered == InputManager::menuButtonID) {
            // TODO
        }

        if (hovered == InputManager::helpButtonID) {
            // TODO
        }

        if (    hovered == InputManager::orderingPuzzleID ||
                hovered == InputManager::orderingPuzzleEndID ||
                hovered == InputManager::rotatingLockPuzzleUpID ||
                hovered == InputManager::rotatingLockPuzzleDownID ||
                hovered == InputManager::rotatingLockPuzzleEndID ||
                hovered == InputManager::leverPuzzleID ||
                hovered == InputManager::leverPuzzleEndID ||
                hovered == InputManager::telephoneID ||
                hovered == InputManager::telephoneEndID ||
                hovered == InputManager::sliderPuzzleID ||
                hovered == InputManager::sliderPuzzleEndID ||
                hovered == InputManager::passwordPuzzleEndID) {
            // TODO
        }

    } else if (_engine->input->isClickValidRMB) {
        if (_engine->input->hoveredElementID == InputManager::textBoxScrollbarID) {
            // TODO, moves scrollbar one line up
        } else if (_engine->input->hoveredElementID == InputManager::inventoryScrollbarID) {
            // TODO, moves scrollbar one line up
        } else if (_engine->input->hoveredElementID == InputManager::textBoxID) {
            // TODO
        }
    } else {
        // Perform movement
        byte inputs = _engine->input->getInput();
        if  ( ( (
                    (inputs & InputManager::kLeftMouseButtonDown) != (inputs & InputManager::kRightMouseButtonDown) 
                ) ||
                (inputs & (InputManager::kMoveUp | InputManager::kMoveDown | InputManager::kMoveLeft | InputManager::kMoveRight) )
              ) &&
                movementDirection != 0 &&
                playTimeThisFrame > _nextBackgroundMovement
            ) {
            switch(movementDirection) {
                case kLeft:
                    _engine->playState.currentViewFrame += 1;
                    if (_engine->playState.currentViewFrame >= (int16)_engine->graphics->getBackgroundFrameCount()) {
                        _engine->playState.currentViewFrame = 0;
                    }
                    break;
                case kRight:
                    _engine->playState.currentViewFrame -= 1;
                    if (_engine->playState.currentViewFrame < 0) {
                        _engine->playState.currentViewFrame = (int16)_engine->graphics->getBackgroundFrameCount() -1;
                    }
                    break;
                case kUp:
                    if (_engine->playState.verticalScroll != 0) {
                        int16 newScroll = _engine->playState.verticalScroll - _engine->playState.verticalScrollDelta;
                        _engine->playState.verticalScroll = MAX(newScroll, (int16)0);
                    }
                    break;
                case kDown:
                    if (_engine->playState.verticalScroll != _engine->playState.currentMaxVerticalScroll) {
                        uint16 newScroll = _engine->playState.verticalScroll + _engine->playState.verticalScrollDelta;
                        _engine->playState.verticalScroll = MIN(newScroll, _engine->playState.currentMaxVerticalScroll);
                    }
                    break;
                case kUp | kLeft:
                    if (_engine->playState.verticalScroll != 0) {
                        int16 newScroll = _engine->playState.verticalScroll - _engine->playState.verticalScrollDelta;
                        _engine->playState.verticalScroll = MAX(newScroll, (int16)0);
                    }
                    _engine->playState.currentViewFrame += 1;
                    if (_engine->playState.currentViewFrame >= (int16)_engine->graphics->getBackgroundFrameCount()) {
                        _engine->playState.currentViewFrame = 0;
                    }
                    break;
                case kUp | kRight:
                    if (_engine->playState.verticalScroll != 0) {
                        int16 newScroll = _engine->playState.verticalScroll - _engine->playState.verticalScrollDelta;
                        _engine->playState.verticalScroll = MAX(newScroll, (int16)0);
                    }
                    _engine->playState.currentViewFrame -= 1;
                    if (_engine->playState.currentViewFrame < 0) {
                        _engine->playState.currentViewFrame = _engine->graphics->getBackgroundFrameCount() - 1;
                    }
                    break;
                case kDown | kLeft:
                    if (_engine->playState.verticalScroll != _engine->playState.currentMaxVerticalScroll) {
                        uint16 newScroll = _engine->playState.verticalScroll + _engine->playState.verticalScrollDelta;
                        _engine->playState.verticalScroll = MIN(newScroll, _engine->playState.currentMaxVerticalScroll);
                    }
                    _engine->playState.currentViewFrame += 1;
                    if (_engine->playState.currentViewFrame >= (int16)_engine->graphics->getBackgroundFrameCount()) {
                        _engine->playState.currentViewFrame = 0;
                    }
                    break;
                case kDown | kRight:
                    if (_engine->playState.verticalScroll != _engine->playState.currentMaxVerticalScroll) {
                        uint16 newScroll = _engine->playState.verticalScroll + _engine->playState.verticalScrollDelta;
                        _engine->playState.verticalScroll = MIN(newScroll, _engine->playState.currentMaxVerticalScroll);
                    }
                    _engine->playState.currentViewFrame -= 1;
                    if (_engine->playState.currentViewFrame < 0) {
                        _engine->playState.currentViewFrame = _engine->graphics->getBackgroundFrameCount() -1;
                    }
                    break;
            }
            if (_engine->input->getInput(InputManager::kMoveFastModifier) ||
                _engine->input->getInput(InputManager::kRightMouseButtonDown)) {
                _nextBackgroundMovement = playTimeThisFrame + currentScene.fastMoveTimeDelta;

            } else {
                _nextBackgroundMovement = playTimeThisFrame + currentScene.slowMoveTimeDelta;
            }
        }
    }

    // Redraw the Background surface if we've moved
    if (_engine->playState.currentViewFrame != _engine->playState.lastDrawnViewFrame) {
        if (currentScene.videoFormat == 1) {
            // TODO if it ever gets hit
        } else if (currentScene.videoFormat == 2) {
            _engine->graphics->_background.copyRectToSurface(
            *_engine->graphics->getBackgroundFrame(_engine->playState.currentViewFrame),
            0, 0,
            Common::Rect(0, _engine->playState.verticalScroll, _engine->graphics->getBackgroundWidth() - 1,
                _engine->playState.verticalScroll + _engine->graphics->viewportDesc.source.bottom));
        }
        // TODO some if related to PlaySoundPanFrameAnchorAndDie
        _engine->playState.lastDrawnViewFrame = _engine->playState.currentViewFrame;
        // TODO function call that sets a val to 1 and 3 others to 0
    }

    _engine->logic->processActionRecords();

    // code that skips rendering for the first 12 frames??? why

    handleMouse();

    // TODO
    _engine->graphics->renderDisplay();
}

void SceneManager::handleMouse() {
    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("CUR IMAGE CURSOR");
    Common::Point mousePos = _engine->input->getMousePosition();
    zr.destRect.left = zr.destRect.right = mousePos.x;
    zr.destRect.top = zr.destRect.bottom = mousePos.y;
    movementDirection = 0;
    _engine->input->hoveredElementID = -1;

    View &view = _engine->graphics->viewportDesc;

    // TODO hotpoints for scene movement are not correct, figure out how they're actually calculated
    Common::Point viewportMouse = mousePos + Common::Point(10, 10);
    
    // Check if the mouse is within the viewport
    if (view.destination.contains(viewportMouse)){
        _engine->input->setPointerBitmap(0, 0, 0);
        // We can scroll left and right
        if (currentScene.horizontalEdgeSize > 0) {
            if (viewportMouse.x < view.destination.left + currentScene.horizontalEdgeSize) {
                movementDirection |= kLeft;
            } else if (viewportMouse.x > view.destination.right - currentScene.horizontalEdgeSize) {
                movementDirection |= kRight;
            }
        }
        if (currentScene.verticalEdgeSize > 0) {
            if (viewportMouse.y < view.destination.top + currentScene.verticalEdgeSize) {
                movementDirection |= kUp;
            } else if (viewportMouse.y > view.destination.bottom - currentScene.verticalEdgeSize) {
                movementDirection |= kDown;
            }
        }

        if (movementDirection != 0) {
            _engine->input->setPointerBitmap(0, 2, 0);
        } else {
            // Go through all action records and find hotspots
            Common::Array<ActionRecord *> &records = _engine->logic->getActionRecords();
            for (uint i = 0; i < records.size(); ++i) {
                ActionRecord *r = records[i];
                if (r->isActive && r->hasHotspot) {
                    // Adjust the hotspot coordinates relative to the viewport
                    Common::Rect hotspot = r->hotspot;
                    hotspot.left += view.destination.left;
                    hotspot.top += view.destination.top;
                    hotspot.right += view.destination.left;
                    hotspot.bottom += view.destination.top;

                    if (hotspot.contains(viewportMouse)) {
                        _engine->input->setPointerBitmap(-1, 1, 0);
                        _engine->input->hoveredElementID = i;
                    }
                }
            }
        }
    } else {
        
        // pointers are out of order from the one on Object0
        _engine->input->setPointerBitmap(1, 1, 0);
        
    }
        
}

void SceneManager::clearSceneData() {
    // TODO these shouldn't be here
    _engine->graphics->_primaryFrameSurface.free();
    _engine->graphics->_object0Surface.free();
    _engine->graphics->_inventoryBoxIconsSurface.free();
    _engine->graphics->_inventoryCursorsSurface.free();
}

} // End of namespace Nancy