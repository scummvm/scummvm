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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/script_manager.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/cursors/cursor_manager.h"
#include "zvision/core/save_manager.h"
#include "zvision/scripting/actions.h"
#include "zvision/utility/utility.h"

#include "common/algorithm.h"
#include "common/hashmap.h"
#include "common/debug.h"
#include "common/stream.h"


namespace ZVision {

ScriptManager::ScriptManager(ZVision *engine)
	: _engine(engine),
	  _currentlyFocusedControl(0) {
}

ScriptManager::~ScriptManager() {
	for (PuzzleList::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); ++iter) {
		delete (*iter);
	}
	for (PuzzleList::iterator iter = _globalPuzzles.begin(); iter != _globalPuzzles.end(); ++iter) {
		delete (*iter);
	}
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		delete (*iter);
	}
}

void ScriptManager::initialize() {
	parseScrFile("universe.scr", true);
	changeLocation('g', 'a', 'r', 'y', 0);
}

void ScriptManager::update(uint deltaTimeMillis) {
	updateNodes(deltaTimeMillis);
	checkPuzzleCriteria();
}

void ScriptManager::createReferenceTable() {
	// Iterate through each local Puzzle
	for (PuzzleList::iterator activePuzzleIter = _activePuzzles.begin(); activePuzzleIter != _activePuzzles.end(); ++activePuzzleIter) {
		Puzzle *puzzlePtr = (*activePuzzleIter);

		// Iterate through each CriteriaEntry and add a reference from the criteria key to the Puzzle
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = (*activePuzzleIter)->criteriaList.begin(); criteriaIter != (*activePuzzleIter)->criteriaList.end(); ++criteriaIter) {
			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = criteriaIter->begin(); entryIter != criteriaIter->end(); ++entryIter) {
				_referenceTable[entryIter->key].push_back(puzzlePtr);

				// If the argument is a key, add a reference to it as well
				if (entryIter->argumentIsAKey) {
					_referenceTable[entryIter->argument].push_back(puzzlePtr);
				}
			}
		}
	}

	// Iterate through each global Puzzle
	for (PuzzleList::iterator globalPuzzleIter = _globalPuzzles.begin(); globalPuzzleIter != _globalPuzzles.end(); ++globalPuzzleIter) {
		Puzzle *puzzlePtr = (*globalPuzzleIter);

		// Iterate through each CriteriaEntry and add a reference from the criteria key to the Puzzle
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = (*globalPuzzleIter)->criteriaList.begin(); criteriaIter != (*globalPuzzleIter)->criteriaList.end(); ++criteriaIter) {
			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = criteriaIter->begin(); entryIter != criteriaIter->end(); ++entryIter) {
				_referenceTable[entryIter->key].push_back(puzzlePtr);

				// If the argument is a key, add a reference to it as well
				if (entryIter->argumentIsAKey) {
					_referenceTable[entryIter->argument].push_back(puzzlePtr);
				}
			}
		}
	}

	// Remove duplicate entries
	for (PuzzleMap::iterator referenceTableIter = _referenceTable.begin(); referenceTableIter != _referenceTable.end(); ++referenceTableIter) {
		removeDuplicateEntries(referenceTableIter->_value);
	}
}

void ScriptManager::updateNodes(uint deltaTimeMillis) {
	// If process() returns true, it means the node can be deleted
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end();) {
		if ((*iter)->process(deltaTimeMillis)) {
			delete (*iter);
			// Remove the node
			iter = _activeControls.erase(iter);
		} else {
			++iter;
		}
	}
}

void ScriptManager::checkPuzzleCriteria() {
	while (!_puzzlesToCheck.empty()) {
		Puzzle *puzzle = _puzzlesToCheck.pop();

		// Check if the puzzle is already finished
		// Also check that the puzzle isn't disabled
		if (getStateValue(puzzle->key) == 1 && (getStateFlags(puzzle->key) & DISABLED) == 0) {
			continue;
		}

		// Check each Criteria

		bool criteriaMet = false;
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = puzzle->criteriaList.begin(); criteriaIter != puzzle->criteriaList.end(); ++criteriaIter) {
			criteriaMet = false;

			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = criteriaIter->begin(); entryIter != criteriaIter->end(); ++entryIter) {
				// Get the value to compare against
				uint argumentValue;
				if (entryIter->argumentIsAKey)
					argumentValue = getStateValue(entryIter->argument);
				else
					argumentValue = entryIter->argument;

				// Do the comparison
				switch (entryIter->criteriaOperator) {
				case Puzzle::EQUAL_TO:
					criteriaMet = getStateValue(entryIter->key) == argumentValue;
					break;
				case Puzzle::NOT_EQUAL_TO:
					criteriaMet = getStateValue(entryIter->key) != argumentValue;
					break;
				case Puzzle::GREATER_THAN:
					criteriaMet = getStateValue(entryIter->key) > argumentValue;
					break;
				case Puzzle::LESS_THAN:
					criteriaMet = getStateValue(entryIter->key) < argumentValue;
					break;
				}

				// If one check returns false, don't keep checking
				if (!criteriaMet) {
					break;
				}
			}

			// If any of the Criteria are *fully* met, then execute the results
			if (criteriaMet) {
				break;
			}
		}

		// criteriaList can be empty. Aka, the puzzle should be executed immediately
		if (puzzle->criteriaList.empty() || criteriaMet) {
			debug(1, "Puzzle %u criteria passed. Executing its ResultActions", puzzle->key);

			// Set the puzzle as completed
			setStateValue(puzzle->key, 1);

			bool shouldContinue = true;
			for (Common::List<ResultAction *>::iterator resultIter = puzzle->resultActions.begin(); resultIter != puzzle->resultActions.end(); ++resultIter) {
				shouldContinue = shouldContinue && (*resultIter)->execute(_engine);
				if (!shouldContinue) {
					break;
				}
			}

			if (!shouldContinue) {
				break;
			}
		}
	}
}

void ScriptManager::cleanStateTable() {
	for (StateMap::iterator iter = _globalState.begin(); iter != _globalState.end(); ++iter) {
		// If the value is equal to zero, we can purge it since getStateValue()
		// will return zero if _globalState doesn't contain a key
		if (iter->_value == 0) {
			// Remove the node
			_globalState.erase(iter);
		}
	}
}

uint ScriptManager::getStateValue(uint32 key) {
	if (_globalState.contains(key))
		return _globalState[key];
	else
		return 0;
}

void ScriptManager::setStateValue(uint32 key, uint value) {
	_globalState[key] = value;

	if (_referenceTable.contains(key)) {
		for (Common::Array<Puzzle *>::iterator iter = _referenceTable[key].begin(); iter != _referenceTable[key].end(); ++iter) {
			_puzzlesToCheck.push((*iter));
		}
	}
}

uint ScriptManager::getStateFlags(uint32 key) {
	if (_globalStateFlags.contains(key))
		return _globalStateFlags[key];
	else
		return 0;
}

void ScriptManager::setStateFlags(uint32 key, uint flags) {
	_globalStateFlags[key] = flags;

	if (_referenceTable.contains(key)) {
		for (Common::Array<Puzzle *>::iterator iter = _referenceTable[key].begin(); iter != _referenceTable[key].end(); ++iter) {
			_puzzlesToCheck.push((*iter));
		}
	}
}

void ScriptManager::addToStateValue(uint32 key, uint valueToAdd) {
	_globalState[key] += valueToAdd;
}

void ScriptManager::addControl(Control *control) {
	_activeControls.push_back(control);
}

Control *ScriptManager::getControl(uint32 key) {
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		if ((*iter)->getKey() == key) {
			return (*iter);
		}
	}

	return nullptr;
}

void ScriptManager::focusControl(uint32 key) {
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		uint32 controlKey = (*iter)->getKey();
		
		if (controlKey == key) {
			(*iter)->focus();
		} else if (controlKey == _currentlyFocusedControl) {
			(*iter)->unfocus();
		}
	}

	_currentlyFocusedControl = key;
}

void ScriptManager::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		(*iter)->onMouseDown(screenSpacePos, backgroundImageSpacePos);
	}
}

void ScriptManager::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		(*iter)->onMouseUp(screenSpacePos, backgroundImageSpacePos);
	}
}

bool ScriptManager::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	bool cursorWasChanged = false;
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		cursorWasChanged = cursorWasChanged || (*iter)->onMouseMove(screenSpacePos, backgroundImageSpacePos);
	}

	return cursorWasChanged;
}

void ScriptManager::onKeyDown(Common::KeyState keyState) {
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		(*iter)->onKeyDown(keyState);
	}
}

void ScriptManager::onKeyUp(Common::KeyState keyState) {
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		(*iter)->onKeyUp(keyState);
	}
}

void ScriptManager::changeLocation(char world, char room, char node, char view, uint32 offset) {
	assert(world != 0);
	debug(1, "Changing location to: %c %c %c %c %u", world, room, node, view, offset);

	// Auto save
	_engine->getSaveManager()->autoSave();

	// Clear all the containers
	_referenceTable.clear();
	_puzzlesToCheck.clear();
	for (PuzzleList::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); ++iter) {
		delete (*iter);
	}
	_activePuzzles.clear();
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		delete (*iter);
	}
	_activeControls.clear();

	// Revert to the idle cursor
	_engine->getCursorManager()->revertToIdle();

	// Reset the background velocity
	_engine->getRenderManager()->setBackgroundVelocity(0);

	// Remove any alphaEntries
	_engine->getRenderManager()->clearAlphaEntries();

	// Clean the global state table
	cleanStateTable();

	// Parse into puzzles and controls
	Common::String fileName = Common::String::format("%c%c%c%c.scr", world, room, node, view);
	parseScrFile(fileName);

	// Change the background position
	_engine->getRenderManager()->setBackgroundPosition(offset);

	// Enable all the controls
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		(*iter)->enable();
	}

	// Add all the local puzzles to the queue to be checked
	for (PuzzleList::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); ++iter) {
		// Reset any Puzzles that have the flag ONCE_PER_INST
		if ((getStateFlags((*iter)->key) & ONCE_PER_INST) == ONCE_PER_INST) {
			setStateValue((*iter)->key, 0);
		}

		_puzzlesToCheck.push((*iter));
	}

	// Add all the global puzzles to the queue to be checked
	for (PuzzleList::iterator iter = _globalPuzzles.begin(); iter != _globalPuzzles.end(); ++iter) {
		// Reset any Puzzles that have the flag ONCE_PER_INST
		if ((getStateFlags((*iter)->key) & ONCE_PER_INST) == ONCE_PER_INST) {
			setStateValue((*iter)->key, 0);
		}

		_puzzlesToCheck.push((*iter));
	}

	// Create the puzzle reference table
	createReferenceTable();

	// Update _currentLocation
	_currentLocation.world = world;
	_currentLocation.room = room;
	_currentLocation.node = node;
	_currentLocation.view = view;
	_currentLocation.offset = offset;
}

void ScriptManager::serializeStateTable(Common::WriteStream *stream) {
	// Write the number of state value entries
	stream->writeUint32LE(_globalState.size());

	for (StateMap::iterator iter = _globalState.begin(); iter != _globalState.end(); ++iter) {
		// Write out the key/value pair
		stream->writeUint32LE(iter->_key);
		stream->writeUint32LE(iter->_value);
	}
}

void ScriptManager::deserializeStateTable(Common::SeekableReadStream *stream) {
	// Clear out the current table values
	_globalState.clear();

	// Read the number of key/value pairs
	uint32 numberOfPairs = stream->readUint32LE();

	for (uint32 i = 0; i < numberOfPairs; ++i) {
		uint32 key = stream->readUint32LE();
		uint32 value = stream->readUint32LE();
		// Directly access the state table so we don't trigger Puzzle checks
		_globalState[key] = value;
	}
}

void ScriptManager::serializeControls(Common::WriteStream *stream) {
	// Count how many controls need to save their data
	// Because WriteStream isn't seekable
	uint32 numberOfControlsNeedingSerialization = 0;
	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		if ((*iter)->needsSerialization()) {
			numberOfControlsNeedingSerialization++;
		}
	}
	stream->writeUint32LE(numberOfControlsNeedingSerialization);

	for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
		(*iter)->serialize(stream);
	}
}

void ScriptManager::deserializeControls(Common::SeekableReadStream *stream) {
	uint32 numberOfControls = stream->readUint32LE();

	for (uint32 i = 0; i < numberOfControls; ++i) {
		uint32 key = stream->readUint32LE();
		for (ControlList::iterator iter = _activeControls.begin(); iter != _activeControls.end(); ++iter) {
			if ((*iter)->getKey() == key) {
				(*iter)->deserialize(stream);
				break;
			}
		}
	}
}

Location ScriptManager::getCurrentLocation() const {
	Location location = _currentLocation;
	location.offset = _engine->getRenderManager()->getCurrentBackgroundOffset();

	return location;
}

} // End of namespace ZVision
