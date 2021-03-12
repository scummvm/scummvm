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

#ifndef NANCY_ACTION_ACTIONRECORD_H
#define NANCY_ACTION_ACTIONRECORD_H

#include "engines/nancy/input.h"

#include "engines/nancy/time.h"
#include "engines/nancy/cursor.h"

#include "common/str.h"
#include "common/stream.h"
#include "common/rect.h"

namespace Nancy {

class NancyEngine;

namespace Action {
    
enum DependencyType : byte {
    kNone               = 0,
    kInventory          = 1,
    kEventFlag          = 2,
    kLogicCondition     = 3,
    kTotalTime          = 4,
    kSceneTime          = 5,
    kPlayerTime         = 6,
    // ...
    kSceneCount         = 9,
    kResetOnNewDay      = 10,
    kUseItem            = 11,
    kTimeOfDay          = 12,
    kTimerNotDone       = 13,
    kTimerDone          = 14,
    kDifficultyLevel    = 15
};

// Describes a condition that needs to be fulfilled before the
// action record can be executed
struct DependencyRecord {
    DependencyType type;    // 0x00
    byte label;             // 0x01
    byte condition;         // 0x02
    bool orFlag;            // 0x03
    int16 hours;            // 0x04
    int16 minutes;          // 0x06
    int16 seconds;          // 0x08
    int16 milliseconds;     // 0x0A

    bool satisfied;
    Time timeData;
};

// Describes a single action that will be performed on every update.
// Supports conditional execution (via dependencies) and can have
// clickable hotspots on screen.
// Does _not_ support drawing to screen, records that need this functionality
// will have to also subclass RenderObject.
class ActionRecord {
    friend class ActionManager;
public:
    enum ExecutionState { kBegin, kRun, kActionTrigger };
    enum ExecutionType { kOneShot = 1, kRepeating = 2 };
    ActionRecord() :
        type(0),
        execType(kOneShot),
        isActive(false),
        isDone(false),
        hasHotspot(false),
        state(ExecutionState::kBegin),
        days(-1),
        itemRequired(-1) {}
    virtual ~ActionRecord() {}

    virtual void readData(Common::SeekableReadStream &stream) =0;
    virtual void execute() {}
    virtual void onPause(bool pause) {}

    virtual CursorManager::CursorType getHoverCursor() const { return CursorManager::kHotspot; }
    virtual void handleInput(NancyInput &input) {}

protected:   
    void finishExecution() {
        switch (execType) {
        case kOneShot:
            isDone = true;
            state = kBegin;
            break;
        case kRepeating:
            isDone = false;
            isActive = false;
            state = kBegin;

            for (uint i = 0; i < dependencies.size(); ++i) {
                dependencies[i].satisfied = false;
            }

            break;
        default:
            state = kBegin;
            break;
        }
    }

    // Used for debugging
    virtual Common::String getRecordTypeName() const =0;

public:
    Common::String description;                     // 0x00
    byte type;                                      // 0x30
    ExecutionType execType;                         // 0x31
    // 0x32 data
    Common::Array<DependencyRecord> dependencies;   // 0x36
    // 0x3A numDependencies
    bool isActive;                                  // 0x3B
    // 0x3C satisfiedDependencies[] 
    // 0x48 timers[]
    // 0x78 orFlags[]
    bool isDone;                                    // 0x84
    bool hasHotspot;                                // 0x85
    Common::Rect hotspot;                           // 0x89
    ExecutionState state;                           // 0x91
    int16 days;                                     // 0x95
    int8 itemRequired;                              // 0x97
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ACTIONRECORD_H
