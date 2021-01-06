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
        playState.eventFlags[i] = PlayState::Flag::kFalse;
    }

    for (uint i = 0; i < 1000; ++i) {
        playState.sceneHitCount[i] = 0;
    }

    _sceneID = _engine->_firstSceneID;
    _engine->_gameFlow.previousGameState = NancyEngine::kScene;

    // TODO init action records
    
    for (uint i = 0; i <= 60; ++i) {
		_engine->graphics->clearGenericZRenderStruct(i);
	}

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

    // TODO init various rects for the ZRender structs

    // Init the ZRender structs themselves:
    // TODO most of these are wrong and/or incomplete
    #define READ_SOURCE_RECT(x) chunk->seek(x); source = new Common::Rect(chunk->readUint32LE(), chunk->readUint32LE(), chunk->readUint32LE(), chunk->readUint32LE());
    #define READ_DEST_POINT(x) chunk->seek(x); dest = new Common::Point(chunk->readUint32LE(), chunk->readUint32LE());

    Common::SeekableReadStream *chunk;
    Common::Rect *source;
    Common::Point *dest;

    // CUR: current
    // RES: redraws the background of a moved element so it doesnt get doubled
    // VP: viewport

    chunk = _engine->getBootChunkStream("MENU");
    READ_SOURCE_RECT(16)
    READ_DEST_POINT(16)
    _engine->graphics->initGenericZRenderStruct(0, "FRAME", 1, true, ZRenderStruct::BltType::kNone, &_engine->graphics->_primaryFrameSurface,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderFrame),
                                                source, dest);
    _engine->graphics->initGenericZRenderStruct(2, "CUR IMAGE CURSOR", 11, false, ZRenderStruct::BltType::kTrans,
                                                &_engine->graphics->_object0Surface, nullptr, new Common::Rect(), new Common::Point());
    _engine->graphics->initGenericZRenderStruct(3, "RES IMAGE CURSOR - FRAME", 2, false, ZRenderStruct::BltType::kNoTrans,
                                                &_engine->graphics->_primaryFrameSurface, nullptr, new Common::Rect(), new Common::Point());
    _engine->graphics->initGenericZRenderStruct(4, "RES IMAGE CURSOR - VP", 3, false, ZRenderStruct::BltType::kNoTrans,
                                                &_engine->graphics->_background, nullptr, new Common::Rect(), new Common::Point());
    // Skip DIAGNOSTICS and VERSION

    chunk = _engine->getBootChunkStream("TBOX");
    READ_SOURCE_RECT(0)
    _engine->graphics->initGenericZRenderStruct(9, "CUR TB BAT SLIDER", 9, false, ZRenderStruct::BltType::kTrans,
                                                &_engine->graphics->_object0Surface, nullptr, source, new Common::Point());           

    _engine->graphics->initGenericZRenderStruct(10, "RES TB BAT SLIDER", 3, false, ZRenderStruct::BltType::kNoTrans, &_engine->graphics->_primaryFrameSurface,
                                               new RenderFunction(_engine->graphics, &GraphicsManager::renderResTBBatSlider),
                                               new Common::Rect(), new Common::Point());

    chunk = _engine->getBootChunkStream("BSUM");
    READ_DEST_POINT(356)
    _engine->graphics->initGenericZRenderStruct(8, "FRAME TB SURF", 6, false, ZRenderStruct::BltType::kNoTrans,
                                                &_engine->graphics->_frameTextBox, nullptr, source, dest);

    READ_SOURCE_RECT(356)
    READ_DEST_POINT(356)
    _engine->graphics->initGenericZRenderStruct(11, "TB FRAME RES", 2, false, ZRenderStruct::BltType::kNoTrans,
                                                &_engine->graphics->_primaryFrameSurface, nullptr, source, dest);  

    READ_SOURCE_RECT(388)
    READ_DEST_POINT(420)
    _engine->graphics->initGenericZRenderStruct(12, "MENU BUT DN", 5, false, ZRenderStruct::BltType::kTrans,
                                                &_engine->graphics->_object0Surface, nullptr, source, dest);
    READ_SOURCE_RECT(404)
    READ_DEST_POINT(436)
    _engine->graphics->initGenericZRenderStruct(13, "HELP BUT DN", 5, false, ZRenderStruct::BltType::kTrans,
                                                &_engine->graphics->_object0Surface, nullptr, source, dest);
    
    chunk = _engine->getBootChunkStream("INV");
    READ_SOURCE_RECT(0)
    _engine->graphics->initGenericZRenderStruct(14, "CUR INV SLIDER", 9, false, ZRenderStruct::BltType::kTrans,
                                                &_engine->graphics->_object0Surface, nullptr, source, new Common::Point());

    _engine->graphics->initGenericZRenderStruct(15, "RES INV SLIDER", 3, false, ZRenderStruct::BltType::kNoTrans,
                                                &_engine->graphics->_primaryFrameSurface, nullptr, new Common::Rect(), new Common::Point());

    _engine->graphics->initGenericZRenderStruct(16, "FRAME INV BOX", 6, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderFrameInvBox),
                                                nullptr, nullptr);

    _engine->graphics->initGenericZRenderStruct(5, "INV BITMAP", 9, false, ZRenderStruct::BltType::kNoTrans,
                                                nullptr, nullptr, new Common::Rect(), new Common::Point());

    _engine->graphics->initGenericZRenderStruct(17, "PRIMARY VIDEO", 8, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderPrimaryVideo),
                                                new Common::Rect(), new Common::Point());

    _engine->graphics->initGenericZRenderStruct(18, "SEC VIDEO 0", 8, false, ZRenderStruct::BltType::kTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderSecVideo0),
                                                new Common::Rect(), new Common::Point());

    _engine->graphics->initGenericZRenderStruct(19, "SEC VIDEO 1", 8, false, ZRenderStruct::BltType::kTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderSecVideo1),
                                                new Common::Rect(), new Common::Point());
    
    _engine->graphics->initGenericZRenderStruct(20, "SEC MOVIE", 8, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderSecMovie),
                                                new Common::Rect(), new Common::Point());
    
    _engine->graphics->initGenericZRenderStruct(21, "ORDERING PUZZLE", 7, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderOrderingPuzzle),
                                                nullptr);
    
    _engine->graphics->initGenericZRenderStruct(22, "ROTATING LOCK PUZZLE", 7, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderRotatingLockPuzzle),
                                                nullptr, nullptr);

    _engine->graphics->initGenericZRenderStruct(23, "LEVER PUZZLE", 7, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderLeverPuzzle),
                                                nullptr, nullptr);

    _engine->graphics->initGenericZRenderStruct(24, "TELEPHONE", 7, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderTelephone),
                                                nullptr, nullptr);
    
    _engine->graphics->initGenericZRenderStruct(25, "SLIDER PUZZLE", 7, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderSliderPuzzle),
                                                nullptr, nullptr);
    
    _engine->graphics->initGenericZRenderStruct(26, "PASSWORD PUZZLE", 7, false, ZRenderStruct::BltType::kNoTrans, nullptr,
                                                new RenderFunction(_engine->graphics, &GraphicsManager::renderPasswordPuzzle),
                                                nullptr, nullptr);
    #undef READ_SOURCE_RECT
    #undef READ_DEST_POINT

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
    byte unknown = bsum->readByte();
    if (unknown) {
        currentScene.unknown78 = bsum->readUint16LE();
        currentScene.unknown7A = bsum->readUint16LE();
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
        currentScene.sceneHasMovement = 0;
    }

    View &viewportDesc = _engine->graphics->viewportDesc;

    if (!hasLoadedFromSavefile) {
        playState.currentMaxVerticalScroll = playState.queuedMaxVerticalScroll;
        playState.currentViewFrame = playState.queuedViewFrame;

        if (currentScene.videoFormat == 1) {
            // TODO not sure this ever gets hit
        } else if (currentScene.videoFormat == 2) {
            // always start from the bottom
            playState.verticalScroll = playState.currentMaxVerticalScroll;
        } else {
            error("Unrecognized Scene summary chunk video file format");
        }

        // Some checks against rFrame

        if (currentScene.videoFormat == 1) {
            // TODO not sure this ever gets hit
        } else if (currentScene.videoFormat == 2) {
            if (_engine->graphics->getBackgroundHeight() == viewportDesc.f2Bottom) {
                currentScene.sceneHasMovement = false;
            }
        }
    }

    delete sceneSummaryChunk;

    Common::Point *dest = new Common::Point(viewportDesc.destLeft, viewportDesc.destTop);
    Common::Rect *source = new Common::Rect(viewportDesc.srcLeft, viewportDesc.srcTop, viewportDesc.srcRight, viewportDesc.srcBottom);
   _engine->graphics->initGenericZRenderStruct(2, "VIEWPORT AVF", 6, true, ZRenderStruct::BltType::kNoTrans,
                                                &_engine->graphics->_background, nullptr, source, dest);

    _state = kRun; // TODO temp, is actually StartSound
}

void SceneManager::run() {
    if (isComingFromMenu) {
        // TODO
    }
    isComingFromMenu = false;

    if (orderingPuzzleIsActive)
        _engine->graphics->getGenericZRenderStruct(21)->isActive = true;
    if (rotatingLockPuzzleIsActive)
        _engine->graphics->getGenericZRenderStruct(22)->isActive = true;
    if (leverPuzzleIsActive)
        _engine->graphics->getGenericZRenderStruct(23)->isActive = true;
    if (sliderPuzzleIsActive)
        _engine->graphics->getGenericZRenderStruct(25)->isActive = true;
    if (passwordPuzzleIsActive)
        _engine->graphics->getGenericZRenderStruct(26)->isActive = true;
    if (telephoneIsActive)
        _engine->graphics->getGenericZRenderStruct(24)->isActive = true;

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

    // Unknown if, have never triggered it

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

    // Do some work if we're coming from a different game state
    if (_engine->_gameFlow.previousGameState != NancyEngine::GameState::kScene) {
        uint32 t = _engine->getTotalPlayTime();
        if (hasLoadedFromSavefile) {
            if (t > _stashedTickCount) {
                t -= _stashedTickCount;
                playState.totalTime -= t;
                playState.sceneTime -= t;
                if (playState.timerIsActive)
                    playState.timerTime -= t;
            }
        }
            
        _engine->graphics->getGenericZRenderStruct(12)->isActive = false; // MENU BTN DN
        _engine->graphics->getGenericZRenderStruct(13)->isActive = false; // HELP BTN DN
        hovered = -1;
        // TODO a bunch of function calls
        _engine->_gameFlow.previousGameState = NancyEngine::GameState::kScene;
        return;
    }

    uint32 t = _engine->getTotalPlayTime();
    uint32 diff = 0;
    if (_tickCount < t) {
        diff = t - _tickCount;
        _tickCount = t;
    }
    playState.totalTime += diff;
    if (playState.timerIsActive)
        playState.timerTime += diff;
    playState.sceneTime =+ diff;

    // Calculate the in-game time (playerTime)
    if (t > playState.playerTimeNextMinute) {
        playState.playerTime += 60000; // Add a minute
        playState.playerTimeNextMinute = t + playerTimeMinuteLength; // Set when we're going to add the next minute
    }

    // Set the time of day according to playerTime
    if (playState.playerTime.getHours_alt() >= 7 && playState.playerTime.getHours_alt() < 18) {
        playState.timeOfDay = playState.kDay;
    } else if ((playState.playerTime.getHours_alt() >= 19 || playState.playerTime.getHours_alt() < 6)) {
        playState.timeOfDay = playState.kNight;
    } else {
        playState.timeOfDay = playState.kDuskDawn;
    }
    
    // TODO we're skipping a lot of things like mouse handling and scene movement
    // so we can get _something_ on screen

    // TODO dont call every frame
    _engine->graphics->_background.copyRectToSurface(
        *_engine->graphics->getBackgroundFrame(playState.currentViewFrame),
        0, 0,
        Common::Rect(0, playState.verticalScroll, _engine->graphics->getBackgroundWidth() - 1,
            playState.verticalScroll + _engine->graphics->viewportDesc.srcBottom)); // TODO fix magic number
    _engine->graphics->renderDisplay(25);
}

void SceneManager::clearSceneData() {
    // TODO these shouldn't be here
    _engine->graphics->_primaryFrameSurface.free();
    _engine->graphics->_object0Surface.free();
    _engine->graphics->_inventoryBoxIconsSurface.free();
    _engine->graphics->_inventoryCursorsSurface.free();
}

} // End of namespace Nancy