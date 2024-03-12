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
#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/action/actionmanager.h"
#include "engines/nancy/action/actionrecord.h"

#include "engines/nancy/action/secondarymovie.h"
#include "engines/nancy/action/soundrecords.h"

#include "engines/nancy/state/scene.h"
namespace Nancy {
namespace Action {

ActionManager::~ActionManager() {
	clearActionRecords();
}

void ActionManager::handleInput(NancyInput &input) {
	bool setHoverCursor = false;
	for (auto &rec : _records) {
		if (rec->_isActive && !rec->_isDone) {
			// First, loop through all records and handle special cases.
			// This needs to be a separate loop to handle Overlays as a special case
			// (see note in Overlay::handleInput())
			rec->handleInput(input);
		}
	}

	for (auto &rec : _records) {
		if (	rec->_isActive &&
				!rec->_isDone &&
				rec->_hasHotspot &&
				rec->_hotspot.isValidRect() && // Needed for nancy2 scene 1600
				NancySceneState.getViewport().convertViewportToScreen(rec->_hotspot).contains(input.mousePos)) {
			if (!setHoverCursor) {
				// Hotspots may overlap, but we want the hover cursor for the first one we encounter
				// This fixes the stairs in nancy3
				g_nancy->_cursor->setCursorType(rec->getHoverCursor());
				setHoverCursor = true;
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.input &= ~NancyInput::kLeftMouseButtonUp;

				rec->_cursorDependency = nullptr;
				processDependency(rec->_dependencies, *rec, false);

				if (!rec->_dependencies.satisfied) {
					if (rec->_cursorDependency != nullptr) {
						NancySceneState.playItemCantSound(
							rec->_cursorDependency->label,
							(g_nancy->getGameType() <= kGameTypeNancy2 && rec->_cursorDependency->condition == kCursInvNotHolding));
					} else {
						continue;
					}
				} else {
					rec->_state = ActionRecord::ExecutionState::kActionTrigger;

					input.eatMouseInput();

					if (rec->_cursorDependency) {
						int16 item = rec->_cursorDependency->label;

						// Re-add the object to the inventory unless it's marked as a one-time use
						if (item == NancySceneState.getHeldItem() && item != -1) {
							auto *inventoryData = GetEngineData(INV);
							assert(inventoryData);

							switch (inventoryData->itemDescriptions[item].keepItem) {
							case kInvItemKeepAlways :
								if (g_nancy->getGameType() >= kGameTypeNancy3) {
									// In nancy3 and up this means the object remains in hand, so do nothing
									// Older games had the kInvItemReturn behavior instead
									break;
								}

								// fall through
							case kInvItemReturn :
								NancySceneState.addItemToInventory(item);
								// fall through
							case kInvItemUseThenLose :
								NancySceneState.setHeldItem(-1);
								break;
							}
						}

						rec->_cursorDependency = nullptr;
					}

				}

				break;
			}
		}
	}
}

void ActionManager::addNewActionRecord(Common::SeekableReadStream &inputData) {
	ActionRecord *newRecord = createAndLoadNewRecord(inputData);
	if (!newRecord) {
		inputData.seek(0x30);
		byte ARType = inputData.readByte();

		warning("Action Record type %i is unimplemented or invalid!", ARType);
		return;
	}
	_records.push_back(newRecord);
}

ActionRecord *ActionManager::createAndLoadNewRecord(Common::SeekableReadStream &inputData) {
	inputData.seek(0);
	char descBuf[0x30];
	inputData.read(descBuf, 0x30);
	descBuf[0x2F] = '\0';
	byte ARType = inputData.readByte();
	byte execType = inputData.readByte();
	ActionRecord *newRecord = createActionRecord(ARType, &inputData);

	if (!newRecord) {
		newRecord = new Unimplemented();
	}

	newRecord->_description = descBuf;
	newRecord->_type = ARType;
	newRecord->_execType = (ActionRecord::ExecutionType)execType;

	newRecord->readData(inputData);

	// If the remaining data is less than the total data, there must be dependencies at the end of the chunk
	int64 dataRemaining = inputData.size() - inputData.pos();
	if (dataRemaining > 0 && newRecord->getRecordTypeName() != "Unimplemented") {
		// Each dependency is 12 (up to nancy2) or 16 (nancy3 and up) bytes long
		uint singleDepSize = g_nancy->getGameType() <= kGameTypeNancy2 ? 12 : 16;
		uint numDependencies = dataRemaining / singleDepSize;
		if (dataRemaining % singleDepSize) {
			warning("Action record type %u, %s has incorrect read size!\ndescription:\n%s",
				newRecord->_type,
				newRecord->getRecordTypeName().c_str(),
				newRecord->_description.c_str());

				delete newRecord;

				newRecord = new Unimplemented();
				newRecord->_description = descBuf;
				newRecord->_type = ARType;
				newRecord->_execType = (ActionRecord::ExecutionType)execType;
		}

		if (numDependencies == 0) {
			newRecord->_dependencies.satisfied = true;
		}

		Common::Stack<DependencyRecord *> depStack;
		depStack.push(&newRecord->_dependencies);

		// Initialize the dependencies data
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

	return newRecord;
}

void ActionManager::processActionRecords() {
	_activatedRecordsThisFrame.clear();

	for (auto record : _records) {
		if (record->_isDone) {
			continue;
		}

		// Process dependencies every call. We make sure to ignore cursor dependencies,
		// as they are only handled when calling from handleInput()
		processDependency(record->_dependencies, *record, record->canHaveHotspot());
		record->_isActive = record->_dependencies.satisfied;

		if (record->_isActive) {
			if(record->_state == ActionRecord::kBegin) {
				_activatedRecordsThisFrame.push_back(record);
			}

			record->execute();
			_recordsWereExecuted = true;
		}

		if (g_nancy->getGameType() >= kGameTypeNancy4 && NancySceneState._state == State::Scene::kLoad) {
			// changeScene() must have been called, abort any further processing.
			// Both old and new behavior is needed (nancy3 intro narration, nancy4 garden gate)
			return;
		}
	}

	synchronizeMovieWithSound();
	debugDrawHotspots();
}

void ActionManager::processDependency(DependencyRecord &dep, ActionRecord &record, bool doNotCheckCursor) {
	if (dep.children.size()) {
		// Recursively process child dependencies
		for (uint i = 0; i < dep.children.size(); ++i) {
			processDependency(dep.children[i], record, doNotCheckCursor);
		}

		// An orFlag marks that its corresponding dependency and the one after it
		// mutually satisfy each other; if one is satisfied, so is the other. The effect
		// can be chained indefinitely (for example, the chiming clock in nancy3)
		for (uint i = 0; i < dep.children.size(); ++i) {
			if (dep.children[i].orFlag) {
				// Found an orFlag, start going down the chain of dependencies with orFlags
				bool foundSatisfied = false;
				for (uint j = i; j < dep.children.size(); ++j) {
					if (dep.children[j].satisfied) {
						// A dependency has been satisfied
						foundSatisfied = true;
						break;
					}

					if (!dep.children[j].orFlag) {
						// orFlag chain ended, no satisfied deoendencies
						break;
					}
				}

				if (foundSatisfied) {
					for (; i < dep.children.size(); ++i) {
						dep.children[i].satisfied = true;
						if (!dep.children[i].orFlag) {
							// Last element of orFlag chain
							break;
						}
					}
				}
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
				} else {
					dep.satisfied = false;
				}
			} else {
				if (NancySceneState._flags.items[dep.label] == g_nancy->_true ||
					dep.label == NancySceneState._flags.heldItem) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
				}
			}

			break;
		case DependencyType::kEvent:
			if (NancySceneState.getEventFlag(dep.label, dep.condition)) {
				// nancy1 has code for some timer array that never gets used
				// and is discarded from nancy2 onward
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
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
					} else {
						dep.satisfied = false;
					}
				} else {
					dep.satisfied = false;
				}
			} else {
				dep.satisfied = NancySceneState.getLogicCondition(dep.label, dep.condition);
			}

			break;
		case DependencyType::kElapsedGameTime:
			if (NancySceneState._timers.lastTotalTime >= dep.timeData) {
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
			}

			break;
		case DependencyType::kElapsedSceneTime:
			if (NancySceneState._timers.sceneTime >= dep.timeData) {
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
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
			// Note: nancy7 completely flipped the meaning of 1 and 2
			int count = NancySceneState._flags.sceneCounts.contains(dep.hours) ?
				NancySceneState._flags.sceneCounts[dep.hours] : 0;
			switch (dep.milliseconds) {
			case 1:
				if (	(dep.minutes < count && g_nancy->getGameType() <= kGameTypeNancy6) ||
						(dep.minutes > count && g_nancy->getGameType() >= kGameTypeNancy7)) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
				}

				break;
			case 2:
				if (	(dep.minutes > count && g_nancy->getGameType() <= kGameTypeNancy6) ||
						(dep.minutes < count && g_nancy->getGameType() >= kGameTypeNancy7)) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
				}

				break;
			case 3:
				if (dep.minutes == count) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
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
			} else {
				bool isSatisfied = false;
				int heldItem = NancySceneState.getHeldItem();
				if (heldItem == -1 && dep.label == kCursStandard) {
					isSatisfied = true;
				} else {
					if (g_nancy->getGameType() <= kGameTypeNancy2 && dep.condition == kCursInvNotHolding) {
						// Activate if _not_ holding the specified item. Dropped in nancy3
						if (heldItem != dep.label) {
							isSatisfied = true;
						}
					} else {
						// Activate if holding the specified item.
						if (heldItem == dep.label) {
							isSatisfied = true;
						}
					}
				}

				dep.satisfied = isSatisfied;

				if (isSatisfied) {
					// A satisfied dependency must be moved into the _cursorDependency slot, to make sure
					// the remove from/re-add to inventory logic works correctly
					record._cursorDependency = &dep;
				} else {
					if (record._cursorDependency == nullptr) {
						// However, if the current dependency was not satisfied, we only move it into
						// the _cursorDependency slot if nothing else was there before. This ensures
						// the "can't" sound played is the first dependency's
						record._cursorDependency = &dep;
					}
				}
			}

			break;
		}
		case DependencyType::kPlayerTOD:
			if (dep.label == NancySceneState.getPlayerTOD()) {
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
			}

			break;
		case DependencyType::kTimerLessThanDependencyTime:
			if (NancySceneState._timers.timerTime <= dep.timeData) {
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
			}

			break;
		case DependencyType::kTimerGreaterThanDependencyTime:
			if (NancySceneState._timers.timerTime > dep.timeData) {
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
			}

			break;
		case DependencyType::kDifficultyLevel:
			if (dep.condition == NancySceneState._difficulty) {
				dep.satisfied = true;
			} else {
				dep.satisfied = false;
			}

			break;
		case DependencyType::kClosedCaptioning:
			if (ConfMan.getBool("subtitles")) {
				if (dep.condition == 2) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
				}
			} else {
				if (dep.condition == 1) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
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
		case DependencyType::kRandom:
			// Pick a random number and compare it with the value in condition
			// This is only executed once
			if (!dep.stopEvaluating) {
				if ((int)g_nancy->_randomSource->getRandomNumber(99) < dep.condition) {
					dep.satisfied = true;
				} else {
					dep.satisfied = false;
				}

				dep.stopEvaluating = true;
			}

			break;
		case DependencyType::kDefaultAR:
			// Only execute if no other AR has executed yet
			if (_recordsWereExecuted) {
				dep.satisfied = false;
			} else {
				dep.satisfied = true;
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
	_recordsWereExecuted = false;
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

		// Forcefully re-activate Autotext records, since we need to regenerate the surface
		if (ser.isLoading() && g_nancy->getGameType() >= kGameTypeNancy6 && rec->_type == 61) {
			rec->_isDone = false;
		}
	}
}

void ActionManager::synchronizeMovieWithSound() {
	// Improvement:

	// The original engine had really bad timing issues with AVF videos,
	// as it set the next frame time by adding the frame length to the current evaluation
	// time, instead of to the time the previous frame was drawn. As a result, all
	// movie (and SecondaryVideos) frames play about 12 ms slower than they should.
	// This results in some unfortunate issues in nancy4: if we do as the original
	// engine did and just make frames 12 ms slower, some dialogue scenes (like scene 1400)
	// are very visibly not in sync; also, the entire videocam sequence suffers from
	// visible stitches where the scene changes not at the time it was intended to.
	// On the other hand, if instead we don't add those 12ms, that same videocam
	// sequence has a really nasty sound cutoff in the middle of a character speaking.

	// This function intends to fix this issue by subtly manipulating the playback rate
	// of the movie so its length ends up matching that of the sound; if the sound rate was
	// changed instead, we would get slightly off-pitch dialogue, which would be undesirable.

	// The heuristic for catching these cases relies on the scene having a movie and a sound
	// record start at the same frame, and have a (valid) scene change to the same scene.
	PlaySecondaryMovie *movie = nullptr;
	PlaySound *sound = nullptr;

	for (uint i = 0; i < _activatedRecordsThisFrame.size(); ++i) {
		byte type = _activatedRecordsThisFrame[i]->_type;
		// Rely on _type for cheaper type check
		if (type == 53) {
			movie = dynamic_cast<PlaySecondaryMovie *>(_activatedRecordsThisFrame[i]);
		} else if (type == 150 || type == 151 || type == 157) {
			sound = dynamic_cast<PlaySound *>(_activatedRecordsThisFrame[i]);
		}

		if (movie && sound) {
			break;
		}
	}

	if (movie && sound && movie->_sound.name != "NO SOUND") {
		// A movie and a sound both got activated this frame, check if their scene changes match
		if (	movie->_videoSceneChange == PlaySecondaryMovie::kMovieSceneChange &&
				movie->_sceneChange.sceneID == sound->_sceneChange.sceneID &&
				movie->_sceneChange.sceneID != kNoScene) {
			// They match, check how long the sound is...
			Audio::Timestamp length = g_nancy->_sound->getLength(sound->_sound);

			if (length.msecs() != 0) {
				// ..and set the movie's playback speed to match
				movie->_decoder->setRate(Common::Rational(movie->_decoder->getDuration().msecs(), length.msecs()));
			}
		}
	}
}

void ActionManager::debugDrawHotspots() {
	// Draws a rectangle around (non-puzzle) hotspots as well as the id
	// and type of the owning ActionRecord. Hardcoded to font 0 since that's
	// the smallest one available in the engine.
	RenderObject &obj = NancySceneState._hotspotDebug;
	if (ConfMan.getBool("debug_hotspots", Common::ConfigManager::kTransientDomain)) {
		const Font *font = g_nancy->_graphics->getFont(0);
		assert(font);
		uint16 yOffset = NancySceneState.getViewport().getCurVerticalScroll();
		obj.setVisible(true);
		obj._drawSurface.clear(obj._drawSurface.getTransparentColor());

		for (uint i = 0; i < _records.size(); ++i) {
			ActionRecord *rec = _records[i];
			if (rec->_isActive && !rec->_isDone && rec->_hasHotspot) {
				Common::Rect hotspot = rec->_hotspot;
				hotspot.translate(0, -yOffset);
				hotspot.clip(obj._drawSurface.getBounds());

				if (!hotspot.isEmpty()) {
					font->drawString(&obj._drawSurface, Common::String::format("%u, %s", i, rec->getRecordTypeName().c_str()),
					hotspot.left, hotspot.bottom - font->getFontHeight() - 2, hotspot.width(), 0,
					Graphics::kTextAlignCenter, 0, true);
					obj._drawSurface.frameRect(hotspot, 0xFFFFFF);
				}
			}
		}
	} else {
		if (obj.isVisible()) {
			obj.setVisible(false);
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
