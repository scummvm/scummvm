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

#include "common/str.h"
#include "common/stream.h"

namespace Nancy {

class NancyEngine;
    
enum DependencyType : byte {
    kNone               = 0,
    kInventory          = 1,
    kEventFlag          = 2,
    // ...
    kPlayTime           = 4,
    kSceneTime          = 5,
    kSceneCount         = 9,
    // ...
    kTimerNotDone       = 13,
    kTimerDone          = 14,
    kDifficultyLevel    = 15
};

struct DependencyRecord {
    DependencyType type;    // 0x00
    byte label;             // 0x01
    byte condition;         // 0x02
    bool orFlag;            // 0x03
    int16 hours;            // 0x04
    int16 minutes;          // 0x06
    int16 seconds;          // 0x08
    int16 milliseconds;     // 0x0A
};

class ActionRecord {
public:
    enum ExecutionState { Start }; // TODO has 4 states
    ActionRecord() :
        type(0),
        execType(0),
        dependencies(nullptr),
        numDependencies(0),
        hasNoDependencies(0),
        hasSatisfiedCondition(false),
        isDone(false),
        state(ExecutionState::Start) {}
    virtual ~ActionRecord() { delete[] dependencies; delete rawData; }

    virtual uint16 readData(Common::SeekableReadStream &stream) =0;
    virtual void execute(NancyEngine *engine) {};

protected:
    // TODO these are temporary until every data class is figured out
    uint16 readRaw(Common::SeekableReadStream &stream, uint16 bytes) {
        rawData = new byte[bytes];
        stream.read(rawData, bytes);
        return bytes;
    }
    byte *rawData = nullptr;

public:
    Common::String description;     // 0x00
    byte type;                      // 0x30
    byte execType;                  // 0x31
    // 0x32 data
    DependencyRecord *dependencies; // 0x36
    byte numDependencies;           // 0x3A
    bool hasNoDependencies;         // 0x3B, not sure abt this one
    bool hasSatisfiedCondition;     // 0x3C
    int32 timers[12];               // 0x48, not sure how many there can be
    bool isDone;                    // 0x84
    ExecutionState state;           // 0x91
    };

} // End of namespace Nancy

#endif // NANCY_ACTION_ACTIONRECORD_H