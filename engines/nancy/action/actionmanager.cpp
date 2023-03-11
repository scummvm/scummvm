/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
						if (NancySceneState.getInventoryBox().getItemDescription(heldItem).keepItem == kInvItemKeepAlways) {
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
						case kInvEmpty:
							// Item not in possession or held
							if (NancySceneState._flags.items[dep.label] == kInvEmpty &&
								dep.label != NancySceneState._flags.heldItem) {
								dep.satisfied = true;
							}

							break;
						case kInvHolding:
							if (NancySceneState._flags.items[dep.label] == kInvHolding ||
								dep.label == NancySceneState._flags.heldItem) {
								dep.satisfied = true;
							}

							break;
						default:
							break;
						}

						break;
					case DependencyType::kEvent:
						if (NancySceneState.getEventFlag(dep.label, dep.condition)) {
							// nancy1 has code for some timer array that never gets used
							// and is discarded from nancy2 onward
							dep.satisfied = true;
						}

						break;
					case DependencyType::kLogic:
						if (NancySceneState._flags.logicConditions[dep.label].flag == dep.condition) {
							// Wait for specified time before satisfying dependency condition
							Time elapsed = NancySceneState._timers.lastTotalTime - NancySceneState._flags.logicConditions[dep.label].timestamp;

							if (elapsed >= dep.timeData) {
								dep.satisfied = true;
							}
						}

						break;
					case DependencyType::kElapsedGameTime:
						if (NancySceneState._timers.lastTotalTime >= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kElapsedSceneTime:
						if (NancySceneState._timers.sceneTime >= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kElapsedPlayerTime:
						// TODO almost definitely wrong, as the original engine treats player time differently
						if (NancySceneState._timers.playerTime >= dep.timeData) {
							dep.satisfied = true;
						}

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
					case DependencyType::kElapsedPlayerDay:
						if (record->_days == -1) {
							record->_days = NancySceneState._timers.playerTime.getDays();
							dep.satisfied = true;
							break;
						}

						if (record->_days < NancySceneState._timers.playerTime.getDays()) {
							record->_days = NancySceneState._timers.playerTime.getDays();
							for (uint j = 0; j < record->_dependencies.size(); ++j) {
								if (record->_dependencies[j].type == DependencyType::kElapsedPlayerTime) {
									record->_dependencies[j].satisfied = false;
								}
							}
						}

						break;
					case DependencyType::kCursorType: {
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

						if (dep.condition == kCursInvNotHolding) {
							record->_itemRequired += kCursInvNotHoldingOffset;
						}

						dep.satisfied = true;
						break;
					}
					case DependencyType::kPlayerTOD:
						if (dep.label == NancySceneState.getPlayerTOD()) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kTimerLessThanDependencyTime:
						if (NancySceneState._timers.timerTime <= dep.timeData) {
							dep.satisfied = true;
						}

						break;
					case DependencyType::kTimerGreaterThanDependencyTime:
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
						warning("Unimplemented Dependency type %i", (int)dep.type);
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
