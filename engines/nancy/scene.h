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

#ifndef NANCY_SCENE_H
#define NANCY_SCENE_H

#include "engines/nancy/time.h"
#include "engines/nancy/playstate.h"
#include "engines/nancy/datatypes.h"

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"

namespace Graphics {
	struct Surface;
}

namespace Common {
    class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;

class SceneManager {
    friend class ActionRecord;
public:
    enum MovementDirection : byte { kUp = 1, kDown = 2, kLeft = 4, kRight = 8 };
    SceneManager(NancyEngine *engine) :
        _engine (engine),
        _state (kInit),
        _sceneID (0),
        movementDirection(0),
        stateChangeRequests(0),
        scrollbarMouse(-1, -1) { }

    void process();

    void changeScene(uint16 id, uint16 frame, uint16 verticalOffset, bool noSound);
    void pushScene();
    void popScene();
    void addObjectToInventory(uint16 id);
    void removeObjectFromInventory(uint16 id, bool pickUp = false);
    float handleScrollbar(uint id, bool reset = false);

private:
    void init();
    void load();
    void run();

    void handleMouse();
    void clearSceneData();


public:
    enum State {
        kInit,
        kLoad,
        kStartSound,
        kRun,
        kLoadNew
    };

    enum GameStateChange : byte {
        kHelpMenu = 1 << 0,
        kMainMenu = 1 << 1,
        kSaveLoad = 1 << 2,
        kReloadSave = 1 << 3,
        kSetupMenu = 1 << 4,
        kCredits = 1 << 5,
        kMap = 1 << 6
    };
    
    byte stateChangeRequests; // GameStateChange

    int32 playerTimeMinuteLength;
    byte movementDirection;
    State _state;
    uint16 _sceneID;
    bool doNotStartSound = false;

    Inventory inventoryDesc;

private:
    NancyEngine *_engine;
    SceneSummary currentScene;
    // TODO these two can be Time
    uint32 _tickCount;
    uint32 _stashedTickCount;
    Time _nextBackgroundMovement;

    uint16 _pushedSceneID = 10000;
    uint16 _pushedFrameID = 0;
    uint16 _pushedVerticalScroll = 0;

    Common::Point scrollbarMouse;

    bool isComingFromMenu = true;
    bool hasLoadedFromSavefile = false;

    bool orderingPuzzleIsActive = false;
    bool rotatingLockPuzzleIsActive = false;
    bool leverPuzzleIsActive = false;
    bool sliderPuzzleIsActive = false;
    bool passwordPuzzleIsActive = false;
    bool telephoneIsActive = false;

    Common::Array<Common::String> _ZRenderFilter;
};

} // End of namespace Nancy

#endif // NANCY_SCENE_H