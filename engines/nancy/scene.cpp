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
#include "engines/nancy/audio.h"

#include "common/memstream.h"
#include "common/rect.h"
#include "common/func.h"

#include "graphics/surface.h"

namespace Nancy {

void SceneManager::process() {
    switch (_state) {
    case kInit:
        init();
        break;
    case kLoad:
        load();
        break;
    case kStartSound:
        _state = kRun;
        if (!doNotStartSound) {
            _engine->sound->stopAllSounds();
            _engine->sound->loadSound(currentScene.audioFile, currentScene.audioID, 0, currentScene.audioVolume);
            _engine->sound->pauseSound(currentScene.audioID, false);
        }
        // fall through
    case kRun:
        run();
        break;
    case kLoadNew:
        _state = kLoad;
        break;
    }
}

void SceneManager::changeScene(uint16 id, uint16 frame, uint16 verticalOffset, bool noSound) {
    _sceneID = id;
    _engine->playState.queuedViewFrame = frame;
    _engine->playState.queuedMaxVerticalScroll = verticalOffset;
    doNotStartSound = noSound;
    _state = kLoadNew;
}

void SceneManager::addObjectToInventory(uint16 id) {
    if (_engine->playState.inventory.heldItem == id) {
        _engine->playState.inventory.heldItem = -1;
    }

    GraphicsManager::InventoryBox &box = _engine->graphics->inventoryBoxDesc;
    _engine->playState.inventory.items[id] = PlayState::kTrue;
    box.itemsOrder.push_back(id);

    // Update the inventory box
    _engine->graphics->updateInvBox();
}

void SceneManager::pickUpObject(uint16 id) {
    Common::Array<uint16> &order = _engine->graphics->inventoryBoxDesc.itemsOrder;
    Common::Array<uint16> temp;

    _engine->playState.inventory.items[id] = PlayState::kFalse;

    // Pop elements from the order array until we find the correct one
    for (int i = order.size() - 1; i >= 0; --i) {
        uint16 thisElem = order.back();
        order.pop_back();
        if (thisElem == id) {
            _engine->playState.inventory.heldItem = id;
            break;
        } else {
            temp.push_back(thisElem);
        }
    }

    // Return all elements in the same order except the one we removed
    for (int i = temp.size() - 1; i >= 0; --i) {
        order.push_back(temp[i]);
    }

    // Update the inventory box
    _engine->graphics->updateInvBox();
}

void SceneManager::init() {
    for (uint i = 0; i < 168; ++i) {
        _engine->playState.eventFlags[i] = PlayState::Flag::kFalse;
    }

    for (uint i = 0; i < 1000; ++i) {
        _engine->playState.sceneHitCount[i] = 0;
    }

    _sceneID = _engine->_firstSceneID;
    _engine->_gameFlow.previousGameState = NancyEngine::kScene;
    
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

    // Load inventory
    inventoryDesc.read(*_engine->getBootChunkStream("INV"));
    if (!_engine->_res->loadImage("ciftree", inventoryDesc.inventoryBoxIconsImageName, _engine->graphics->_inventoryBoxIconsSurface)) {
        error("Failed to load inventory icons (TOOL)");
    }

    // Load the cursors
    if (!_engine->_res->loadImage("ciftree", inventoryDesc.inventoryCursorsImageName, _engine->graphics->_inventoryCursorsSurface)) {
        error("Failed to load cursors (TOOLCUR1)");
    }

    delete[] name;

    _engine->graphics->initSceneZRenderStructs(_ZRenderFilter);

    // Set the scroll bar destinations
    Common::SeekableReadStream *tbox = _engine->getBootChunkStream("TBOX");
    tbox->seek(0x30);
    ZRenderStruct &tbzr = _engine->graphics->getZRenderStruct("CUR TB BAT SLIDER");
    tbzr.destRect.left =  tbox->readUint16LE() - (tbzr.sourceRect.width() / 2); // coords in file are for center position
    tbzr.destRect.top =  tbox->readUint16LE();
    tbzr.destRect.right = tbzr.destRect.left + tbzr.sourceRect.width();
    tbzr.destRect.bottom = tbzr.destRect.top + tbzr.sourceRect.height();

    ZRenderStruct &invzr = _engine->graphics->getZRenderStruct("CUR INV SLIDER");
    invzr.destRect.left = inventoryDesc.sliderDefaultDest.x - (invzr.sourceRect.width() / 2); // coords in file are for center position
    invzr.destRect.top = inventoryDesc.sliderDefaultDest.y;
    invzr.destRect.right = invzr.destRect.left + invzr.sourceRect.width();
    invzr.destRect.bottom = invzr.destRect.top + invzr.sourceRect.height();

    _state = kLoad;
}

void SceneManager::load() {
    clearSceneData();

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

    currentScene = SceneSummary();
    currentScene.read(*sceneSummaryChunk);

    // The check to see if we need to switch the CD is performed here

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

    // Redraw the viewport
    _engine->playState.lastDrawnViewFrame = -1;
    _engine->input->setPointerBitmap(0, 0, false);

    _state = kStartSound;
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

    if (stateChangeRequests & kHelpMenu) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kHelp;
        // _engine->helpMenu->state = HelpMenu::State::kInit;
        stateChangeRequests &= ~kHelpMenu;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (stateChangeRequests & kMainMenu) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kMainMenu;
        // _engine->mainMenu->state = MainMenu::State::kInit;
        stateChangeRequests &= ~kMainMenu;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (stateChangeRequests & kSaveLoad) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kLoadSave;
        // _engine->loadSaveMenu->state = LoadSaveMenu::State::kInit;
        stateChangeRequests &= ~kSaveLoad;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (stateChangeRequests & kReloadSave) {
        // TODO
    }

    if (stateChangeRequests & kSetupMenu) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kSetup;
        // _engine->setupMenu->state = setupMenu::State::kInit;
        stateChangeRequests &= ~kSetupMenu;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (stateChangeRequests & kCredits) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kHelp;
        // _engine->credits->state = CreditsSequence::State::kInit;
        stateChangeRequests &= ~kCredits;
        // _engine->sound->stopSceneSpecificSounds();
        return;
    }

    if (stateChangeRequests & kMap) {
        _stashedTickCount = _engine->getTotalPlayTime();
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        _engine->_gameFlow.minGameState = NancyEngine::GameState::kMap;
        // _engine->map->state = Map::State::kInit;
        stateChangeRequests &= ~kMap;
        _engine->sound->stopAllSounds();
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

        // Viewport gets reused, set it back to the correct surface
        ZRenderStruct &zr = _engine->graphics->getZRenderStruct("VIEWPORT AVF");
        zr.sourceSurface = &_engine->graphics->_background;
        zr.sourceRect = _engine->graphics->viewportDesc.source;
        _engine->playState.lastDrawnViewFrame = -1;
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

    if (_engine->input->isClickValidLMB()) {
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
            stateChangeRequests |= kMap;
            return;
        } else if (hovered == InputManager::textBoxID) {
            // TODO
        } else if (hovered == InputManager::inventoryItemTakeID) {
            GraphicsManager::InventoryBox &box = _engine->graphics->inventoryBoxDesc;
            Common::Point mousePos = _engine->input->getMousePosition();
            for (uint i = 0; i < 4; ++i) {
                if (box.onScreenItems[i].dest.contains(mousePos)) {
                    pickUpObject(box.onScreenItems[i].itemId);
                    break;
                }
            }
        } else if (hovered == InputManager::inventoryItemReturnID) {
            addObjectToInventory(_engine->playState.inventory.heldItem);
        } else if (hovered == InputManager::textBoxScrollbarID) {
            handleScrollbar(0);
        } else if (hovered == InputManager::inventoryScrollbarID) {
            handleScrollbar(1);
        } else if (hovered == InputManager::menuButtonID) {
            // TODO
        } else if (hovered == InputManager::helpButtonID) {
            // TODO
        } else if (hovered == InputManager::orderingPuzzleID ||
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
        } else {
            // Not a UI element, ID must be an action record's
            ActionRecord *rec = _engine->logic->getActionRecord(hovered);
            if (rec->isActive /*&& another condition !- 0*/) {
                bool shouldTrigger = false;
                int16 &heldItem = _engine->playState.inventory.heldItem;
                if (rec->itemRequired != -1) {
                    if (heldItem == -1 && rec->itemRequired == -2) {
                        shouldTrigger = true;
                    } else {
                        if (rec->itemRequired <= 100) {
                            if (heldItem == rec->itemRequired) {
                                shouldTrigger = true;
                            }
                        } else if (rec->itemRequired <= 110 && rec->itemRequired - 100 != heldItem) {
                            // IDs 100 - 110 mean the record will activate when the object is _not_ the specified one
                            shouldTrigger = true;
                        }
                    }
                } else {
                    shouldTrigger = true;
                }
                if (shouldTrigger) {
                    rec->state = ActionRecord::ExecutionState::kActionTrigger;
                    
                    if (rec->itemRequired > 100 && rec->itemRequired <= 110) {
                        rec->itemRequired -= 100;
                    }

                    // Re-add the object to the inventory unless it's marked as a one-time use
                    if (rec->itemRequired == heldItem && rec->itemRequired != -1) {
                        if (inventoryDesc.items[heldItem].oneTimeUse != 0) {
                            addObjectToInventory(heldItem);
                        }

                        heldItem = -1;
                    }
                }
                
            }
        }


    } else if (_engine->input->isClickValidRMB()) {
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
    if (_engine->playState.currentViewFrame != _engine->playState.lastDrawnViewFrame ||
        _engine->playState.verticalScroll != _engine->playState.lastVerticalScroll) {
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
    _engine->playState.lastVerticalScroll = _engine->playState.verticalScroll;
    _engine->graphics->renderDisplay(_ZRenderFilter);
}

void SceneManager::handleMouse() {
    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("CUR IMAGE CURSOR");
    Common::Point mousePos = _engine->input->getMousePosition();
    zr.destRect.left = zr.destRect.right = mousePos.x;
    zr.destRect.top = zr.destRect.bottom = mousePos.y;
    movementDirection = 0;
    _engine->input->hoveredElementID = -1;
    bool returnCursorSelected = false;

    View &view = _engine->graphics->viewportDesc;

    // TODO incorrect magic number, figure out where this comes from
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
            _engine->input->setPointerBitmap(-1, 2, 0);
        } else {
            // Go through all action records and find hotspots
            Common::Array<ActionRecord *> &records = _engine->logic->getActionRecords();
            for (uint i = 0; i < records.size(); ++i) {
                ActionRecord *r = records[i];
                if (r->isActive && r->hasHotspot) {
                    // Adjust the hotspot coordinates relative to the viewport
                    // taking into account the vertical scroll as well
                    Common::Rect hotspot = r->hotspot;
                    hotspot.left += view.destination.left;
                    hotspot.top += view.destination.top - _engine->playState.verticalScroll;
                    hotspot.right += view.destination.left;
                    hotspot.bottom += view.destination.top - _engine->playState.verticalScroll;

                    if (hotspot.contains(viewportMouse)) {
                        if (r->type == 0xE || r->type == 0x3D || r->type == 0x3E) {
                            // Set the pointer to the U-shaped arrow if
                            // the type is Hot1FrExitSceneChange, MapCallHot1Fr, or MapCallHotMultiframe
                            _engine->input->setPointerBitmap(0, 3, 0);
                            returnCursorSelected = true;
                        } else {
                            _engine->input->setPointerBitmap(-1, 1, 0);
                        }
                        _engine->input->hoveredElementID = i;
                    }
                }
            }
        }

        // If we're holding an item we need to show it, but only in the viewport
        if (_engine->playState.inventory.heldItem != -1 && !returnCursorSelected) {
            _engine->input->setPointerBitmap(_engine->playState.inventory.heldItem + 3, -1, 1);
        }
    } else {
        // Check if we're hovering above a UI element
        ZRenderStruct *uizr = &_engine->graphics->getZRenderStruct("CUR TB BAT SLIDER");
        if (uizr->destRect.contains(mousePos)) {
            _engine->input->hoveredElementID = InputManager::textBoxScrollbarID;
            _engine->input->setPointerBitmap(1, 2, -1);
            handleScrollbar(0);
        } else if (uizr = &_engine->graphics->getZRenderStruct("CUR INV SLIDER"), uizr->destRect.contains(mousePos)) {
            _engine->input->hoveredElementID = InputManager::inventoryScrollbarID;
            _engine->input->setPointerBitmap(1, 2, -1);
            handleScrollbar(1);
        } else if (_engine->sceneManager->inventoryDesc.shadesDst.contains(mousePos)) {
            if (_engine->playState.inventory.heldItem != -1) {
                _engine->input->hoveredElementID = InputManager::inventoryItemReturnID;
            } else {
                _engine->input->hoveredElementID = InputManager::inventoryItemTakeID;
            }
        } else {
            _engine->input->setPointerBitmap(1, 1, 0);
        }
    }     
}

void SceneManager::clearSceneData() {
    // only clear select flags
    for (uint i = 44; i < 54; ++i) {
        _engine->playState.eventFlags[i] = PlayState::kFalse;
    }
    for (uint i = 63; i < 74; ++i) {
        _engine->playState.eventFlags[i] = PlayState::kFalse;
    }
    for (uint i = 75; i < 85; ++i) {
        _engine->playState.eventFlags[i] = PlayState::kFalse;
    }

    _engine->logic->clearActionRecords();

    _engine->graphics->getZRenderStruct("SEC VIDEO 0").isActive = false;
    _engine->graphics->getZRenderStruct("SEC VIDEO 1").isActive = false;
    _engine->graphics->getZRenderStruct("SEC MOVIE").isActive = false;
    _engine->graphics->getZRenderStruct("STATIC BITMAP ANIMATION").isActive = false;
}

// 0 is textbox, 1 is inventory, returns -1 when movement is stopped/disabled
float SceneManager::handleScrollbar(uint id) {
    Common::SeekableReadStream *chunk;
    ZRenderStruct *zr;
    if (id == 0) {
        chunk = _engine->getBootChunkStream("TBOX");
        chunk->seek(0x30);
        zr = &_engine->graphics->getZRenderStruct("CUR TB BAT SLIDER");
    } else if (id == 1) {
        chunk = _engine->getBootChunkStream("INV");
        chunk->seek(0x10);
        zr = &_engine->graphics->getZRenderStruct("CUR INV SLIDER");
    }
    Common::Point origDest;
    origDest.x = chunk->readUint16LE() - (zr->sourceRect.width() / 2); // coords in file are for center position
    origDest.y = chunk->readUint16LE();

    Common::Rect &tboxRect = _engine->graphics->getZRenderStruct("FRAME TB SURF").destRect;
    Common::Rect &scrollRect = zr->destRect;
    Common::Point newMousePos = _engine->input->getMousePosition();

    if (_engine->input->getInput() & InputManager::kLeftMouseButtonDown) {
        if (scrollbarMouse.x == -1 && scrollbarMouse.y == -1) {
            scrollbarMouse = newMousePos - Common::Point(scrollRect.left, scrollRect.top);
        }
        newMousePos -= Common::Point(scrollRect.left, scrollRect.top);

        uint16 minY = origDest.y;
        uint16 maxY = tboxRect.bottom - scrollRect.height() + tboxRect.top - origDest.y; // TODO goes a little out of bounds
        uint16 newTop = CLIP((uint16)(scrollRect.top + newMousePos.y - scrollbarMouse.y), minY, maxY);
        scrollRect.bottom += newTop - scrollRect.top;
        scrollRect.top = newTop;

        return (float)(maxY - minY) / newTop;
    } else {
        scrollbarMouse.x = -1;
        scrollbarMouse.y = -1;
    }

    return -1;
}

} // End of namespace Nancy