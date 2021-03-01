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

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionmanager.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/viewport.h"

#include "common/memstream.h"
#include "common/events.h"
#include "common/str.h"

namespace Nancy {
namespace Action {

void ActionManager::handleInput(NancyInput &input) {
    for (auto &rec : _records) {
        if (rec->isActive) {
            // Send input to all active records
            rec->handleInput(input);
        }

        if (rec->isActive && rec->hasHotspot && _engine->scene->getViewport().convertViewportToScreen(rec->hotspot).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(rec->getHoverCursor());

            if (input.input & NancyInput::kLeftMouseButtonUp) {
                input.input &= ~NancyInput::kLeftMouseButtonUp;

                bool shouldTrigger = false;
                int16 heldItem = _engine->scene->getHeldItem();
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

                    if (!shouldTrigger) {
                        _engine->sound->playSound(17); // Hardcoded by original engine
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
                        if (_engine->scene->getInventoryBox().getItemDescription(heldItem).oneTimeUse != 0) {
                            _engine->scene->getInventoryBox().addItem(heldItem);
                        }

                        _engine->scene->setHeldItem(-1);
                    }
                }

                break;
            }
        }
    }
}

bool ActionManager::addNewActionRecord(Common::SeekableReadStream &inputData) {
    inputData.seek(0x30);
    byte ARType = inputData.readByte();
    ActionRecord *newRecord = createActionRecord(ARType);

    inputData.seek(0);
    char *descBuf = new char[0x30];
    inputData.read(descBuf, 0x30);
    newRecord->description = Common::String(descBuf);
    delete[] descBuf;

    newRecord->type = inputData.readByte(); // redundant
    newRecord->execType = (ActionRecord::ExecutionType)inputData.readByte();

    uint16 localChunkSize = newRecord->readData(inputData);
    localChunkSize += 0x32;

    // If the localChunkSize is less than the total data, there must be dependencies at the end of the chunk
    uint16 depsDataSize = (uint16)inputData.size() - localChunkSize;
    if (depsDataSize > 0) {
        // Each dependency is 0x0C bytes long (in v1)
        uint numDependencies = depsDataSize / 0xC;
        if (depsDataSize % 0xC) {
            error("Invalid dependency data size!");
        }

        // Initialize the dependencies data
        inputData.seek(localChunkSize);
        for (uint16 i = 0; i < numDependencies; ++i) {
            newRecord->dependencies.push_back(DependencyRecord());
            DependencyRecord &dep = newRecord->dependencies.back();

            dep.type = (DependencyType)inputData.readByte();
            dep.label = inputData.readByte();
            dep.condition = inputData.readByte();
            dep.orFlag = inputData.readByte();
            dep.hours = inputData.readSint16LE();
            dep.minutes = inputData.readSint16LE();
            dep.seconds = inputData.readSint16LE();
            dep.milliseconds = inputData.readSint16LE();

            if (dep.type != kSceneCount || dep.hours != -1 || dep.minutes != -1 || dep.seconds != -1) {
                dep.timeData = ((dep.hours * 60 + dep.minutes) * 60 + dep.seconds) * 1000 + dep.milliseconds;
            }
        }
    } else {
        // Set new record to active if it doesn't depend on anything
        newRecord->isActive = true;
    }

    _records.push_back(newRecord);

    debugC(1, kDebugActionRecord, "Loaded action record %i, type %s, typeID %i, description \"%s\", execType == %s",
            _records.size() - 1,
            newRecord->getRecordTypeName().c_str(),
            newRecord->type,
            newRecord->description.c_str(),
            newRecord->execType == ActionRecord::kRepeating ? "kRepeating" : "kOneShot");
    for (uint i = 0; i < newRecord->dependencies.size(); ++i) {
        debugCN(1, kDebugActionRecord, "\tDependency %i: type ", i);
        switch (newRecord->dependencies[i].type) {
            case kNone :debugCN(1, kDebugActionRecord, "kNone"); break;
            case kInventory :
                debugCN(1, kDebugActionRecord, "kInventory, item ID %i %s",
                            newRecord->dependencies[i].label,
                            newRecord->dependencies[i].condition == kTrue ? "is in possession" : "is not in possession");
                break;
            case kEventFlag :
                debugCN(1, kDebugActionRecord, "kEventFlag, flag ID %i == %s",
                            newRecord->dependencies[i].label,
                            newRecord->dependencies[i].condition == kTrue ? "true" : "false");
                break;
            case kLogicCondition :
                debugCN(1, kDebugActionRecord, "kLogicCondition, logic condition ID %i == %s",
                            newRecord->dependencies[i].label,
                            newRecord->dependencies[i].condition == kTrue ? "true" : "false");
                break;
            case kTotalTime :
                debugCN(1, kDebugActionRecord, "kTotalTime, %i hours, %i minutes, %i seconds, %i milliseconds",
                            newRecord->dependencies[i].hours,
                            newRecord->dependencies[i].minutes,
                            newRecord->dependencies[i].seconds,
                            newRecord->dependencies[i].milliseconds);
                break;
            case kSceneTime :
                debugCN(1, kDebugActionRecord, "kSceneTime, %i hours, %i minutes, %i seconds, %i milliseconds",
                            newRecord->dependencies[i].hours,
                            newRecord->dependencies[i].minutes,
                            newRecord->dependencies[i].seconds,
                            newRecord->dependencies[i].milliseconds);
                break;
            case kPlayerTime :
                debugCN(1, kDebugActionRecord, "kPlayerTime, %i days, %i hours, %i minutes, %i seconds",
                            newRecord->dependencies[i].hours,
                            newRecord->dependencies[i].minutes,
                            newRecord->dependencies[i].seconds,
                            newRecord->dependencies[i].milliseconds);
                break;
            case kSceneCount :
                debugCN(1, kDebugActionRecord, "kSceneCount, scene ID %i, hit count %s %i",
                            newRecord->dependencies[i].hours,
                            newRecord->dependencies[i].milliseconds == 1 ? ">" : newRecord->dependencies[i].milliseconds == 2 ? "<" : "==",
                            newRecord->dependencies[i].seconds);
                break;
            case kResetOnNewDay : debugCN(1, kDebugActionRecord, "kResetOnNewDay"); break;
            case kUseItem :
                debugCN(1, kDebugActionRecord, "kUseItem, item ID %i %s",
                            newRecord->dependencies[i].label,
                            newRecord->dependencies[i].condition == kTrue ? "is held" : "is not held");
                break;
            case kTimeOfDay :
                debugCN(1, kDebugActionRecord, "kTimeOfDay, %s",
                            newRecord->dependencies[i].label == 0 ? "day" : newRecord->dependencies[i].label == 1 ? "night" : "dusk/dawn");
                break;
            case kTimerNotDone : debugCN(1, kDebugActionRecord, "kTimerNotDone"); break;
            case kTimerDone : debugCN(1, kDebugActionRecord, "kTimerDone"); break;
            case kDifficultyLevel :
                debugCN(1, kDebugActionRecord, "kDifficultyLevel, level %i", newRecord->dependencies[i].condition);
                break;
            default: debugCN(1, kDebugActionRecord, "unknown"); break;
        }
        debugC(1, kDebugActionRecord, ", orFlag == %s", newRecord->dependencies[i].orFlag == true ? "true" : "false");
    }


    return true;
}

void ActionManager::processActionRecords() {    
    for (auto record : _records) {
        if (record->isDone) {
            continue;
        }

        if (!record->isActive) {
            for (uint i = 0; i < record->dependencies.size(); ++i) {
                DependencyRecord &dep = record->dependencies[i];
                if (!dep.satisfied) {
                    switch (dep.type) {
                        case kNone:
                            dep.satisfied = true;
                            break;
                        case kInventory:
                            switch (dep.condition) {
                                case kFalse:
                                    // Item not in possession or held
                                    if (_engine->scene->_flags.items[dep.label] == kFalse &&
                                        dep.label != _engine->scene->_flags.heldItem) {
                                        dep.satisfied = true;
                                    }
                                    break;
                                case kTrue:
                                    if (_engine->scene->_flags.items[dep.label] == kTrue ||
                                        dep.label == _engine->scene->_flags.heldItem) {
                                        dep.satisfied = true;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case kEventFlag:
                            if (_engine->scene->getEventFlag(dep.label, (NancyFlag)dep.condition))
                                // nancy1 has code for some timer array that never gets used
                                // and is discarded from nancy2 onward
                                dep.satisfied = true;
                            break;
                        case kLogicCondition:
                            if (_engine->scene->_flags.logicConditions[dep.label].flag == dep.condition) {
                                // Wait for specified time before satisfying dependency condition
                                Time elapsed = _engine->scene->_timers.totalTime - _engine->scene->_flags.logicConditions[dep.label].timestamp;
                                if (elapsed >= dep.timeData)
                                    dep.satisfied = true;
                            }
                            break;
                        case kTotalTime:
                            if (_engine->scene->_timers.totalTime >= dep.timeData)
                                dep.satisfied = true;
                            break;
                        case kSceneTime:
                            if (_engine->scene->_timers.sceneTime >= dep.timeData)
                                dep.satisfied = true;
                            break;
                        case kPlayerTime:
                            // TODO almost definitely wrong, as the original engine treats player time differently
                            if (_engine->scene->_timers.playerTime >= dep.timeData)
                                dep.satisfied = true;
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
                                    if (dep.seconds < _engine->scene->_sceneState.sceneHitCount[dep.hours])
                                        dep.satisfied = true;
                                    break;
                                case 2:
                                    if (dep.seconds > _engine->scene->_sceneState.sceneHitCount[dep.hours])
                                        dep.satisfied = true;
                                    break;
                                case 3:
                                    if (dep.seconds == _engine->scene->_sceneState.sceneHitCount[dep.hours])
                                        dep.satisfied = true;
                                    break;
                            }
                            break;
                        case kResetOnNewDay:
                            if (record->days == -1) {
                                record->days = _engine->scene->_timers.playerTime.getDays();
                                dep.satisfied = true;
                                break;
                            }

                            if (record->days < _engine->scene->_timers.playerTime.getDays()) {
                                record->days = _engine->scene->_timers.playerTime.getDays();
                                for (uint j = 0; j < record->dependencies.size(); ++j) {
                                    if (record->dependencies[j].type == kPlayerTime) {
                                        record->dependencies[j].satisfied = false;
                                    }
                                }
                            }
                            break;
                        case kUseItem: {
                            bool hasUnsatisfiedDeps = false;
                            for (uint j = 0; j < record->dependencies.size(); ++j) {
                                if (j != i && record->dependencies[j].satisfied == false) {
                                    hasUnsatisfiedDeps = true;
                                }
                            }

                            if (hasUnsatisfiedDeps) {
                                break;
                            }

                            record->itemRequired = dep.label;

                            if (dep.condition == 1) {
                                record->itemRequired += 100;
                            }
                            
                            dep.satisfied = true;
                            break;
                        }
                        case kTimeOfDay:
                            if (dep.label == (byte)_engine->scene->_timers.timeOfDay)
                                dep.satisfied = true;
                            break;
                        case kTimerNotDone:
                            if (_engine->scene->_timers.timerTime <= dep.timeData)
                                dep.satisfied = true;
                            break;
                        case kTimerDone:
                            if (_engine->scene->_timers.timerTime > dep.timeData)
                                dep.satisfied = true;
                            break;
                        case kDifficultyLevel:
                            if (dep.condition == _engine->scene->_difficulty)
                                dep.satisfied = true;
                            break;
                        default:
                            break;
                    }
                }
            }

            // An orFlag marks that its corresponding dependency and the one after it
            // mutually satisfy each other; if one is satisfied, so is the other
            for (uint i = 1; i < record->dependencies.size(); ++i) {
                if (record->dependencies[i-1].orFlag) {
                    if (record->dependencies[i-1].satisfied)
                        record->dependencies[i].satisfied = true;
                    if (record->dependencies[i].satisfied)
                        record->dependencies[i-1].satisfied = true;
                }
            }

            // Check if all dependencies have been satisfied, and activate the record if they have
            uint satisfied = 0;
            for (uint i = 0; i < record->dependencies.size(); ++i) {
                if (record->dependencies[i].satisfied)
                    ++satisfied;
            }

            if (satisfied == record->dependencies.size())
                record->isActive = true;
        
        }

        if (record->isActive) {
            record->execute(_engine);
        }
    }
}

void ActionManager::clearActionRecords() {
    for (auto &r : _records) {
        delete r;
    }
    _records.clear();
}

} // End of namespace Action
} // End of namespace Nancy
