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
public:
    SceneManager(NancyEngine *engine) :
        _engine {engine},
        _state {kInit},
        _sceneID {0} { }
    ~SceneManager();

    void process();

private:
    void init();
    void load();
    void run();

    void clearSceneData();

    enum State {
        kInit,
        kLoad,
        kStartSound,
        kRun,
        kLoadNew
    };

public:
    PlayState playState;
    int32 playerTimeMinuteLength;

private:
    NancyEngine *_engine;
    State _state;
    uint16 _sceneID;
    SceneSummary currentScene;
    // TODO these two can be Time
    uint32 _tickCount;
    uint32 _stashedTickCount;
    int16 hovered = -1;

    bool isComingFromMenu = true;
    bool hasLoadedFromSavefile = false;

    bool orderingPuzzleIsActive = false;
    bool rotatingLockPuzzleIsActive = false;
    bool leverPuzzleIsActive = false;
    bool sliderPuzzleIsActive = false;
    bool passwordPuzzleIsActive = false;
    bool telephoneIsActive = false;

    // These could be condensed into an enum
    bool helpMenuRequested = false;
    bool mainMenuRequested = false;
    bool saveLoadRequested = false;
    // not sure
    bool setupMenuRequested = false;
    bool creditsSequenceRequested = false;
    bool mapScreenRequested = false;
    
};

} // End of namespace Nancy

#endif // NANCY_SCENE_H