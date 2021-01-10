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

#include "engines/nancy/logic.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/playstate.h"

#include "common/memstream.h"

namespace Nancy {

bool Logic::addNewActionRecord(Common::SeekableReadStream &inputData) {
    inputData.seek(0x30);
    byte ARType = inputData.readByte();
    ActionRecord *newRecord = createActionRecord(ARType);

    inputData.seek(0);
    char *descBuf = new char[0x30];
    inputData.read(descBuf, 0x30);
    newRecord->description = Common::String(descBuf);
    delete[] descBuf;

    newRecord->type = inputData.readByte(); // redundant
    newRecord->execType = inputData.readByte();

    uint16 localChunkSize = newRecord->readData(inputData);
    localChunkSize += 0x32;

    // If the localChunkSize is less than the total data, there must be dependencies at the end of the chunk
    uint16 depsDataSize = (uint16)inputData.size() - localChunkSize;
    if (depsDataSize > 0) {
        // Each dependency is 0x0C bytes long (in v1)
        newRecord->numDependencies = depsDataSize / 0xC;
        if (depsDataSize % 0xC) {
            error("Invalid dependency data size!");
        }

        newRecord->dependencies = new DependencyRecord[newRecord->numDependencies]();
        newRecord->satisfiedDependencies = new bool[newRecord->numDependencies]();
        newRecord->timers = new Time[newRecord->numDependencies]();
        newRecord->orFlags = new bool[newRecord->numDependencies]();

        // Initialize the dependencies data
        inputData.seek(/*0x32 + */localChunkSize);
        for (uint16 i = 0; i < newRecord->numDependencies; ++i) {
            newRecord->dependencies[i].type = (DependencyType)inputData.readByte();
            newRecord->dependencies[i].label = inputData.readByte();
            newRecord->dependencies[i].condition = inputData.readByte();
            newRecord->dependencies[i].orFlag = inputData.readByte();
            newRecord->dependencies[i].hours = inputData.readSint16LE();
            newRecord->dependencies[i].minutes = inputData.readSint16LE();
            newRecord->dependencies[i].seconds = inputData.readSint16LE();
            newRecord->dependencies[i].milliseconds = inputData.readSint16LE();
        }

        for (uint16 i = 0; i < newRecord->numDependencies; ++i) {
            DependencyRecord &current = newRecord->dependencies[i];
            if (current.type != 9 || current.hours != -1 || current.minutes != -1 || current.seconds != -1) {
                newRecord->timers[i] = ((current.hours * 60 + current.minutes) * 60 + current.seconds) * 1000 + current.milliseconds;
            }
        }

        for (uint16 i = 0; i < newRecord->numDependencies; ++i) {
            if (newRecord->dependencies[i].orFlag == 1) {
                newRecord->orFlags[i] = true;
            } else {
                newRecord->orFlags[i] = false;
            }
        }
    } else {
        // Set new record to active if it doesn't depend on anything
        newRecord->isActive = true;
    }

    _records.push_back(newRecord);
    return true;
}

void Logic::processActionRecords() {
    for (auto record : _records) {
        if (record->isDone) {
            continue;
        }

        if (!record->isActive) {
            if (record->numDependencies > 0) {
                for (uint i = 0; i < record->numDependencies; ++i) {
                    if (record->satisfiedDependencies[i] != 0) {
                        DependencyRecord &dep = record->dependencies[i];
                        switch (record->dependencies[1].type) {
                            case kNone:
                                record->satisfiedDependencies[i] = true;
                            case kInventory:
                                // TODO
                                break;
                            case kEventFlag:
                                if (_engine->playState.eventFlags[dep.label] == dep.condition)
                                    // nancy1 has code for some timer array that never gets used
                                    // and is discarded from nancy2 onward
                                    record->satisfiedDependencies[i] = true;
                                break;
                            case kLogicCondition:
                                if (_engine->playState.logicConditions[dep.label] == dep.condition) {
                                    // Wait for specified time before satisfying dependency condition
                                    Time elapsed = _engine->playState.totalTime - _engine->playState.logicConditionsTimestamps[dep.label];
                                    if (elapsed >= record->timers[i])
                                        record->satisfiedDependencies[i] = true;
                                }
                                break;
                            case kTotalTime:
                                if (_engine->playState.totalTime >= record->timers[i])
                                    record->satisfiedDependencies[i] = true;
                                break;
                            case kSceneTime:
                                if (_engine->playState.sceneTime >= record->timers[i])
                                    record->satisfiedDependencies[i] = true;
                                break;
                            case kPlayerTime:
                                if (_engine->playState.playerTime >= record->timers[i])
                                    record->satisfiedDependencies[i] = true;
                                break;
                            /*case 7:
                                // TODO
                                break;
                            case 8:
                                // TODO
                                break;*/
                            case kSceneCount:
                                // This dependency type keeps its data in the time variables
                                // Also, I'm pretty sure it never gets used
                                switch (dep.milliseconds) {
                                    case 1:
                                        if (dep.seconds < _engine->playState.sceneHitCount[dep.hours])
                                            record->satisfiedDependencies[i] = true;
                                        break;
                                    case 2:
                                        if (dep.seconds > _engine->playState.sceneHitCount[dep.hours])
                                            record->satisfiedDependencies[i] = true;
                                        break;
                                    case 3:
                                        if (dep.seconds == _engine->playState.sceneHitCount[dep.hours])
                                            record->satisfiedDependencies[i] = true;
                                        break;
                                }
                                break;
                            /*case 10:
                                // TODO
                                break;
                            case 11:
                                // TODO
                                break;*/
                            case kTimeOfDay:
                                if (dep.label == (byte)_engine->playState.timeOfDay)
                                    record->satisfiedDependencies[i] = true;
                                break;
                            case kTimerNotDone:
                                if (_engine->playState.timerTime <= record->timers[i])
                                    record->satisfiedDependencies[i] = true;
                                break;
                            case kTimerDone:
                                if (_engine->playState.timerTime > record->timers[i])
                                    record->satisfiedDependencies[i] = true;
                                break;
                            case kDifficultyLevel:
                                if (dep.condition == _engine->playState.difficulty)
                                    record->satisfiedDependencies[i] = true;
                                break;
                            default:
                                break;
                        }
                    }
                }

                // An orFlag marks that its corresponding dependency and the one after it
                // mutually satisfy each other; if one is satisfied, so is the other
                for (int i = 1; i < record->numDependencies; ++i) {
                    if (record->orFlags[i-1]) {
                        if (record->satisfiedDependencies[i-1])
                            record->satisfiedDependencies[i] = true;
                        if (record->satisfiedDependencies[i])
                            record->satisfiedDependencies[i-1] = true;
                    }
                }

                // Check if all dependencies have been satisfied, and activate the record if they have
                uint satisfied = 0;
                for (uint i = 0; i < record->numDependencies; ++i) {
                    if (record->satisfiedDependencies[i])
                        ++satisfied;
                }

                if (satisfied == record->numDependencies)
                    record->isActive = true;
            }
        }

        if (record->isActive) {
            record->execute(_engine);
        }
    }
}

} // End of namespace Nancy