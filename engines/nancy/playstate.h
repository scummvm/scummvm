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

#ifndef NANCY_PLAYSTATE_H
#define NANCY_PLAYSTATE_H

#include "engines/nancy/time.h"

namespace Nancy {

// A catch-all struct for storing all player progress and related variables
// TODO split to PlayerState/SceneState
struct PlayState {
    enum Flag { kFalse = 1, kTrue = 2 };
    enum TimeOfDay { kDay, kNight, kDuskDawn };

    Flag inventory[11];
    Flag eventFlags[672];
    // Second array with the same size as EventFlags that never gets used?
    bool sceneHitCount[1000];
    uint16 difficulty; // 0, 1, 2
    Time totalTime;
    Time sceneTime;
    Time timerTime;
    bool timerIsActive = false;
    Time playerTime; // Nancy's in-game time of day, adds a minute every 5 seconds
    Time playerTimeNextMinute; // Stores the next tick count until we add a minute to playerTime
    TimeOfDay timeOfDay = kDay;
    uint16 currentViewFrame = 0;
    uint16 queuedViewFrame = 0;
    uint16 currentMaxVerticalScroll = 0;
    uint16 queuedMaxVerticalScroll = 0;
    uint16 verticalScroll = 0; // This replaces rDisplayed
};

} // End of namespace Nancy

#endif // NANCY_PLAYSTATE_H