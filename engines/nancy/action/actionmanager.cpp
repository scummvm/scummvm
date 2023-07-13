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
#include "common/stack.h"
#include "common/config-manager.h"

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

				processDependency(rec->_dependencies, *rec, false);

				if (!rec->_dependencies.satisfied) {
					if (g_nancy->getGameType() >= kGameTypeNancy2 && rec->_cursorDependency != nullptr) {
						SoundDescription &sound = g_nancy->_inventoryData->itemDescriptions[rec->_cursorDependency->label].specificCantSound;
						g_nancy->_sound->loadSound(sound);
						g_nancy->_sound->playSound(sound);
					} else {
						g_nancy->_sound->playSound("CANT");
					}
				} else {
					rec->_state = ActionRecord::ExecutionState::kActionTrigger;

					if (rec->_cursorDependency) {
						int16 item = rec->_cursorDependency->label;
						if (item > 100 && item <= (100 + g_nancy->getStaticData().numItems)) {
							item -= 100;
						}

						// Re-add the object to the inventory unless it's marked as a one-time use
						if (item == NancySceneState.getHeldItem() && item != -1) {
							if (g_nancy->_inventoryData->itemDescriptions[item].keepItem == kInvItemKeepAlways) {
								NancySceneState.addItemToInventory(item);
							}

							NancySceneState.setHeldItem(-1);
						}

						rec->_cursorDependency = nullptr;
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
		// Each dependency is 12 (up to nancy2) or 16 (nancy3 and up) bytes long 
		uint singleDepSize = g_nancy->getGameType() <= kGameTypeNancy2 ? 12 : 16;
		uint numDependencies = depsDataSize / singleDepSize;
		if (depsDataSize % singleDepSize) {
			error("Action record type %s has incorrect read size!\nScene S%u, AR %u, description:\n%s",
				newRecord->getRecordTypeName().c_str(),
				NancySceneState.getSceneInfo().sceneID,
				_records.size(),
				newRecord->_description.c_str());
		}
		
		if (numDependencies == 0) {
			newRecord->_dependencies.satisfied = true;
		}

		Common::Stack<DependencyRecord *> depStack;
		depStack.push(&newRecord->_dependencies);

		// Initialize the dependencies data
		inputData.seek(localChunkSize);
		for (uint16 i = 0; i < numDependencies; ++i) {
			depStack.top()->children.push_back(DependencyRecord());
			DependencyRecord &dep = depStack.top()->children.back();

			if (singleDepSize == 12) {
				dep.type = (DependencyType)inputData.readByte();
				dep.label = inputData.readByte();
				dep.condition = inputData.readByte();
				dep.orFlag = inputData.readByte();
			} else if (singleDepSize == 16) {
				dep.type = (DependencyType)inputData.readUint16LE();
				dep.label = inputData.readUint16LE();
				dep.condition = inputData.readUint16LE();
				dep.orFlag = inputData.readUint16LE();
			}

			dep.hours = inputData.readSint16LE();
			dep.minutes = inputData.readSint16LE();
			dep.seconds = inputData.readSint16LE();
			dep.milliseconds = inputData.readSint16LE();

			switch (dep.type) {
			case DependencyType::kElapsedPlayerTime:
				dep.timeData = dep.hours * 3600000 + dep.minutes * 60000;

				if (g_nancy->getGameType() < kGameTypeNancy3) {
					// Older titles only checked if the time is less than the one in the dependency
					dep.condition = 0;
				}

				break;
			case DependencyType::kSceneCount:
				break;
			case DependencyType::kOpenParenthesis:
				depStack.push(&dep);
				break;
			case DependencyType::kCloseParenthesis:
				depStack.top()->children.pop_back();
				depStack.pop();
				break;			
			default:
				if (dep.hours != -1 || dep.minutes != -1 || dep.seconds != -1) {
					dep.timeData = ((dep.hours * 60 + dep.minutes) * 60 + dep.seconds) * 1000 + dep.milliseconds;
				}
				
				break;
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
			processDependency(record->_dependencies, *record, record->canHaveHotspot());
			if (record->_dependencies.satisfied) {
				record->_isActive = true;
			}
		}

		if (record->_isActive) {
			record->execute();
		}
	}
}

void ActionManager::processDependency(DependencyRecord &dep, ActionRecord &record, bool doNotCheckCursor) {
	if (dep.children.size()) {
		// Recursively process child dependencies
		for (uint i = 0; i < dep.children.size(); ++i) {
			processDependency(dep.children[i], record, doNotCheckCursor);
		}

		// An orFlag marks that its corresponding dependency and the one after it
		// mutually satisfy each other; if one is satisfied, so is the other
		for (uint i = 1; i < dep.children.size(); ++i) {
			if (dep.children[i - 1].orFlag) {
				if (dep.children[i - 1].satisfied)
					dep.children[i].satisfied = true;
				if (dep.children[i].satisfied)
					dep.children[i - 1].satisfied = true;
			}
		}

		// If all children are satisfied, so is the parent
		dep.satisfied = true;
		for (uint i = 0; i < dep.children.size(); ++i) {
			if (!dep.children[i].satisfied) {
				dep.satisfied = false;
				break;
			}
		}
	} else {
		switch (dep.type) {
		case DependencyType::kNone:
			dep.satisfied = true;
			break;
		case DependencyType::kInventory:
			if (dep.condition == g_nancy->_false) {
				// Item not in possession or held
				if (NancySceneState._flags.items[dep.label] == g_nancy->_false &&
					dep.label != NancySceneState._flags.heldItem) {
					dep.satisfied = true;
				}
			} else {
				if (NancySceneState._flags.items[dep.label] == g_nancy->_true ||
					dep.label == NancySceneState._flags.heldItem) {
					dep.satisfied = true;
				}
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
			if (g_nancy->getGameType() <= kGameTypeNancy2) {
				// First few games used 2 for false and 1 for true, but we store them the
				// other way around here. So, we need to check for inequality
				if (!NancySceneState.getLogicCondition(dep.label, dep.condition)) {
					// Wait for specified time before satisfying dependency condition
					Time elapsed = NancySceneState._timers.lastTotalTime - NancySceneState._flags.logicConditions[dep.label].timestamp;

					if (elapsed >= dep.timeData) {
						dep.satisfied = true;
					}
				}
			} else {
				dep.satisfied = NancySceneState.getLogicCondition(dep.label, dep.condition);
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
		case DependencyType::kElapsedPlayerTime: {
			// We're only interested in the hours and minutes
			Time playerTime = NancySceneState._timers.playerTime.getHours() * 3600000 +
								NancySceneState._timers.playerTime.getMinutes() * 60000;
			switch (dep.condition) {
			case 0:
				dep.satisfied = dep.timeData < playerTime;
				break;
			case 1:
				dep.satisfied = dep.timeData > playerTime;
				break;
			case 2:
				dep.satisfied = dep.timeData == playerTime;
			}

			break;
		}
		case DependencyType::kSceneCount: {
			// Check how many times a scene has been visited.
			// This dependency type keeps its data in the time variables
			int count = NancySceneState._flags.sceneCounts.contains(dep.hours) ?
				NancySceneState._flags.sceneCounts[dep.hours] : 0;
			switch (dep.milliseconds) {
			case 1:
				if (dep.seconds < count) {
					dep.satisfied = true;
				}

				break;
			case 2:
				if (dep.seconds > count) {
					dep.satisfied = true;
				}

				break;
			case 3:
				if (dep.seconds == count) {
					dep.satisfied = true;
				}

				break;
			}

			break;
		}
		case DependencyType::kElapsedPlayerDay:
			if (record._days == -1) {
				record._days = NancySceneState._timers.playerTime.getDays();
				dep.satisfied = true;
				break;
			}

			if (record._days < NancySceneState._timers.playerTime.getDays()) {
				record._days = NancySceneState._timers.playerTime.getDays();

				// This is not used in nancy3 and up, so it's a safe assumption that we
				// do not need to check types recursively
				for (uint j = 0; j < record._dependencies.children.size(); ++j) {
					if (record._dependencies.children[j].type == DependencyType::kElapsedPlayerTime) {
						record._dependencies.children[j].satisfied = false;
					}
				}
			}

			break;
		case DependencyType::kCursorType: {
			if (doNotCheckCursor) {
				dep.satisfied = true;
				record._cursorDependency = &dep;
			} else {
				bool isSatisfied = false;
				int heldItem = NancySceneState.getHeldItem();
				if (heldItem == -1 && dep.label == -2) {
					isSatisfied = true;
				} else {
					if (dep.label <= 100) {
						if (heldItem == dep.label) {
							isSatisfied = true;
						}
					} else if (dep.label - 100 != heldItem) {
						// IDs above 100 mean the record will activate when the object is _not_ the specified one
						isSatisfied = true;
					}
				}

				dep.satisfied = isSatisfied;
				record._cursorDependency = &dep;
			}
			
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
		case DependencyType::kClosedCaptioning:
			if (ConfMan.getBool("subtitles")) {
				if (dep.condition == 2) {
					dep.satisfied = true;
				}
			} else {
				if (dep.condition == 1) {
					dep.satisfied = true;
				}
			}

			break;
		case DependencyType::kSound:
			if (g_nancy->_sound->isSoundPlaying(dep.label)) {
				dep.satisfied = dep.condition == 1;
			} else {
				dep.satisfied = dep.condition == 0;
			}

			break;
		default:
			warning("Unimplemented Dependency type %i", (int)dep.type);
			break;
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
