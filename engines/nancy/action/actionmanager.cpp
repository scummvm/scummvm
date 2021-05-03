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

#include "common/serializer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/action/actionmanager.h"
#include "engines/nancy/action/actionrecord.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void ActionManager::handleInput(NancyInput &input) {
	for (auto &rec : _records) {
		if (rec->_isActive) {
			// Send input to all active records
			rec->handleInput(input);
		}

		if (rec->_isActive && rec->_hasHotspot && NancySceneState.getViewport().convertViewportToScreen(rec->_hotspot).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(rec->getHoverCursor());

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.input &= ~NancyInput::kLeftMouseButtonUp;

				bool shouldTrigger = false;
				int16 heldItem = NancySceneState.getHeldItem();
				if (rec->_itemRequired != -1) {
					if (heldItem == -1 && rec->_itemRequired == -2) {
						shouldTrigger = true;
					} else {
						if (rec->_itemRequired <= 100) {
							if (heldItem == rec->_itemRequired) {
								shouldTrigger = true;
							}
						} else if (rec->_itemRequired <= 110 && rec->_itemRequired - 100 != heldItem) {
							// IDs 100 - 110 mean the record will activate when the object is _not_ the specified one
							shouldTrigger = true;
						}
					}

					if (!shouldTrigger) {
						g_nancy->_sound->playSound("CANT");
					}
				} else {
					shouldTrigger = true;
				}
				if (shouldTrigger) {
					rec->_state = ActionRecord::ExecutionState::kActionTrigger;

					if (rec->_itemRequired > 100 && rec->_itemRequired <= 110) {
						rec->_itemRequired -= 100;
					}

					// Re-add the object to the inventory unless it's marked as a one-time use
					if (rec->_itemRequired == heldItem && rec->_itemRequired != -1) {
						if (NancySceneState.getInventoryBox().getItemDescription(heldItem).oneTimeUse != 0) {
							NancySceneState.getInventoryBox().addItem(heldItem);
						}

						NancySceneState.setHeldItem(-1);
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
	char descBuf[0x30];
	inputData.read(descBuf, 0x30);
	descBuf[0x2F] = '\0';
	newRecord->_description = descBuf;

	newRecord->_type = inputData.readByte(); // redundant
	newRecord->_execType = (ActionRecord::ExecutionType)inputData.readByte();

	uint16 localChunkSize = inputData.pos();
	newRecord->readData(inputData);
	localChunkSize = inputData.pos() - localChunkSize;
	localChunkSize += 0x32;

	// If the localChunkSize is less than the total data, there must be dependencies at the end of the chunk
	uint16 depsDataSize = (uint16)inputData.size() - localChunkSize;
	if (depsDataSize > 0) {
		// Each dependency is 0x0C bytes long (in v1)
		uint numDependencies = depsDataSize / 0xC;
		if (depsDataSize % 0xC) {
			error("Action record type %s has incorrect read size", newRecord->getRecordTypeName().c_str());;
		}

		// Initialize the dependencies data
		inputData.seek(localChunkSize);
		newRecord->_dependencies.reserve(numDependencies);
		for (uint16 i = 0; i < numDependencies; ++i) {
			newRecord->_dependencies.push_back(DependencyRecord());
			DependencyRecord &dep = newRecord->_dependencies.back();

			dep.type = (DependencyType)inputData.readByte();
			dep.label = inputData.readByte();
			dep.condition = inputData.readByte();
			dep.orFlag = inputData.readByte();
			dep.hours = inputData.readSint16LE();
			dep.minutes = inputData.readSint16LE();
			dep.seconds = inputData.readSint16LE();
			dep.milliseconds = inputData.readSint16LE();

			if (dep.type != DependencyType::kSceneCount || dep.hours != -1 || dep.minutes != -1 || dep.seconds != -1) {
				dep.timeData = ((dep.hours * 60 + dep.minutes) * 60 + dep.seconds) * 1000 + dep.milliseconds;
			}
		}
	} else {
		// Set new record to active if it doesn't depend on anything
		newRecord->_isActive = true;
	}

	_records.push_back(newRecord);

	debugC(1, kDebugActionRecord, "Loaded action record %i, type %s, typeID %i, description \"%s\", execType == %s",
			_records.size() - 1,
			newRecord->getRecordTypeName().c_str(),
			newRecord->_type,
			newRecord->_description.c_str(),
			newRecord->_execType == ActionRecord::kRepeating ? "kRepeating" : "kOneShot");
	for (uint i = 0; i < newRecord->_dependencies.size(); ++i) {
		debugCN(1, kDebugActionRecord, "\tDependency %i: type ", i);
		DependencyRecord &dep = newRecord->_dependencies[i];
		switch (dep.type) {
		case DependencyType::kNone :
			debugCN(1, kDebugActionRecord, "kNone");
			break;
		case DependencyType::kInventory :
			debugCN(1, kDebugActionRecord, "kInventory, item ID %i %s",
						dep.label,
						dep.condition == kTrue ? "is in possession" : "is not in possession");
			break;
		case DependencyType::kEventFlag :
			debugCN(1, kDebugActionRecord, "kEventFlag, flag ID %i == %s",
						dep.label,
						dep.condition == kTrue ? "true" : "false");
			break;
		case DependencyType::kLogicCondition :
			debugCN(1, kDebugActionRecord, "kLogicCondition, logic condition ID %i == %s",
						dep.label,
						dep.condition == kTrue ? "true" : "false");
			break;
		case DependencyType::kTotalTime :
			debugCN(1, kDebugActionRecord, "kTotalTime, %i hours, %i minutes, %i seconds, %i milliseconds",
						dep.hours,
						dep.minutes,
						dep.seconds,
						dep.milliseconds);
			break;
		case DependencyType::kSceneTime :
			debugCN(1, kDebugActionRecord, "kSceneTime, %i hours, %i minutes, %i seconds, %i milliseconds",
						dep.hours,
						dep.minutes,
						dep.seconds,
						dep.milliseconds);
			break;
		case DependencyType::kPlayerTime :
			debugCN(1, kDebugActionRecord, "kPlayerTime, %i days, %i hours, %i minutes, %i seconds",
						dep.hours,
						dep.minutes,
						dep.seconds,
						dep.milliseconds);
			break;
		case DependencyType::kSceneCount :
			debugCN(1, kDebugActionRecord, "kSceneCount, scene ID %i, hit count %s %i",
						dep.hours,
						dep.milliseconds == 1 ? ">" : dep.milliseconds == 2 ? "<" : "==",
						dep.seconds);
			break;
		case DependencyType::kResetOnNewDay :
			debugCN(1, kDebugActionRecord, "kResetOnNewDay");
			break;
		case DependencyType::kUseItem :
			debugCN(1, kDebugActionRecord, "kUseItem, item ID %i %s",
						dep.label,
						dep.condition == kTrue ? "is held" : "is not held");
			break;
		case DependencyType::kTimeOfDay :
			debugCN(1, kDebugActionRecord, "kTimeOfDay, %s",
						dep.label == 0 ? "day" : dep.label == 1 ? "night" : "dusk/dawn");
			break;
		case DependencyType::kTimerNotDone :
			debugCN(1, kDebugActionRecord, "kTimerNotDone");
			break;
		case DependencyType::kTimerDone :
			debugCN(1, kDebugActionRecord, "kTimerDone");
			break;
		case DependencyType::kDifficultyLevel :
			debugCN(1, kDebugActionRecord, "kDifficultyLevel, level %i", dep.condition);
			break;
		default:
			debugCN(1, kDebugActionRecord, "unknown");
			break;
		}
		debugC(1, kDebugActionRecord, ", orFlag == %s", dep.orFlag == true ? "true" : "false");
	}

	return true;
}

void ActionManager::processActionRecords() {
	for (auto record : _records) {
		if (record->_isDone) {
			continue;
		}

		if (!record->_isActive) {
			for (uint i = 0; i < record->_dependencies.size(); ++i) {
				DependencyRecord &dep = record->_dependencies[i];

				if (!dep.satisfied) {
					switch (dep.type) {
					case DependencyType::kNone:
						dep.satisfied = true;
						break;
					case DependencyType::kInventory:
						switch (dep.condition) {
						case kFalse:
							// Item not in possession or held
							if (NancySceneState._flags.items[dep.label] == kFalse &&
								dep.label != NancySceneState._flags.heldItem) {
								dep.satisfied = true;
							}

							break;
						case kTrue:
							if (NancySceneState._flags.items[dep.label] == kTrue ||
								dep.label == NancySceneState._flags.heldItem) {
								dep.satisfied = true;
							}

							break;
						default:
							break;
						}

						break;
					case DependencyType::kEventFlag:
						if (NancySceneState.getEventFlag(dep.label, (NancyFlag)dep.condition)) {
							// nancy1 has code for some timer array that never gets used
							// and is discarded from nancy2 onward
							dep.satisfied = true;
						}

						break;
					case DependencyType::kLogicCondition:
						if (NancySceneState._flags.logicConditions[dep.label].flag == dep.condition) {
							// Wait for specified time before satisfying dependency condition
							Time elapsed = NancySceneState._timers.lastTotalTime - NancySceneState._flags.logicConditions[dep.label].timestamp;

							if (elapsed >= dep.timeData) {
								dep.satisfied = true;
							}
						}

						break;
					case DependencyType::kTotalTime:
						if (NancySceneState._timers.lastTotalTime >= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kSceneTime:
						if (NancySceneState._timers.sceneTime >= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kPlayerTime:
						// TODO almost definitely wrong, as the original engine treats player time differently
						if (NancySceneState._timers.playerTime >= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kUnknownType7:
						warning("Unknown Dependency type 7");
						break;
					case DependencyType::kUnknownType8:
						warning("Unknown Dependency type 8");
						break;
					case DependencyType::kSceneCount:
						// This dependency type keeps its data in the time variables
						// Also, I'm pretty sure it never gets used
						switch (dep.milliseconds) {
						case 1:
							if (dep.seconds < NancySceneState._flags.sceneHitCount[dep.hours]) {
								dep.satisfied = true;
							}

							break;
						case 2:
							if (dep.seconds > NancySceneState._flags.sceneHitCount[dep.hours]) {
								dep.satisfied = true;
							}

							break;
						case 3:
							if (dep.seconds == NancySceneState._flags.sceneHitCount[dep.hours]) {
								dep.satisfied = true;
							}

							break;
						}

						break;
					case DependencyType::kResetOnNewDay:
						if (record->_days == -1) {
							record->_days = NancySceneState._timers.playerTime.getDays();
							dep.satisfied = true;
							break;
						}

						if (record->_days < NancySceneState._timers.playerTime.getDays()) {
							record->_days = NancySceneState._timers.playerTime.getDays();
							for (uint j = 0; j < record->_dependencies.size(); ++j) {
								if (record->_dependencies[j].type == DependencyType::kPlayerTime) {
									record->_dependencies[j].satisfied = false;
								}
							}
						}

						break;
					case DependencyType::kUseItem: {
						bool hasUnsatisfiedDeps = false;
						for (uint j = 0; j < record->_dependencies.size(); ++j) {
							if (j != i && record->_dependencies[j].satisfied == false) {
								hasUnsatisfiedDeps = true;
							}
						}

						if (hasUnsatisfiedDeps) {
							break;
						}

						record->_itemRequired = dep.label;

						if (dep.condition == 1) {
							record->_itemRequired += 100;
						}

						dep.satisfied = true;
						break;
					}
					case DependencyType::kTimeOfDay:
						if (dep.label == (byte)NancySceneState._timers.timeOfDay) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kTimerNotDone:
						if (NancySceneState._timers.timerTime <= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kTimerDone:
						if (NancySceneState._timers.timerTime > dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kDifficultyLevel:
						if (dep.condition == NancySceneState._difficulty) {
							dep.satisfied = true;
						}

						break;
					default:
						warning("Unknown Dependency type %i", (int)dep.type);
						break;
					}
				}
			}

			// An orFlag marks that its corresponding dependency and the one after it
			// mutually satisfy each other; if one is satisfied, so is the other
			for (uint i = 1; i < record->_dependencies.size(); ++i) {
				if (record->_dependencies[i - 1].orFlag) {
					if (record->_dependencies[i - 1].satisfied)
						record->_dependencies[i].satisfied = true;
					if (record->_dependencies[i].satisfied)
						record->_dependencies[i - 1].satisfied = true;
				}
			}

			// Check if all dependencies have been satisfied, and activate the record if they have
			uint satisfied = 0;
			for (uint i = 0; i < record->_dependencies.size(); ++i) {
				if (record->_dependencies[i].satisfied)
					++satisfied;
			}

			if (satisfied == record->_dependencies.size())
				record->_isActive = true;

		}

		if (record->_isActive) {
			record->execute();
		}
	}
}

void ActionManager::clearActionRecords() {
	for (auto &r : _records) {
		delete r;
	}
	_records.clear();
}

void ActionManager::onPause(bool pause) {
	for (auto &r : _records) {
		if (r->_isActive && !r->_isDone) {
			r->onPause(pause);
		}
	}
}

void ActionManager::synchronize(Common::Serializer &ser) {
	// When loading, the records should already have been initialized by scene
	for (auto &rec : _records) {
		ser.syncAsByte(rec->_isActive);
		ser.syncAsByte(rec->_isDone);
	}
}

} // End of namespace Action
} // End of namespace Nancy
