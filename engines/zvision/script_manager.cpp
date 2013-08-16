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

#include "common/scummsys.h"

#include "common/algorithm.h"
#include "common/hashmap.h"
#include "common/debug.h"

#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/cursor_manager.h"
#include "zvision/actions.h"
#include "zvision/action_node.h"
#include "zvision/utility.h"

namespace ZVision {

ScriptManager::ScriptManager(ZVision *engine)
	: _engine(engine),
	  _changeLocation(false) {
}

void ScriptManager::initialize() {
	parseScrFile("universe.scr", true);
	changeLocation('g', 'a', 'r', 'y', 0);
}

void ScriptManager::update(uint deltaTimeMillis) {
	updateNodes(deltaTimeMillis);
	checkPuzzleCriteria();

	if (_changeLocation) {
		changeLocationIntern();
		_changeLocation = false;
	}
}

void ScriptManager::createReferenceTable() {
	// Iterate through each local Puzzle
	for (Common::List<Puzzle>::iterator activePuzzleIter = _activePuzzles.begin(); activePuzzleIter != _activePuzzles.end(); activePuzzleIter++) {
		Puzzle *puzzlePtr = &(*activePuzzleIter);

		// Iterate through each Criteria and add a reference from the criteria key to the Puzzle
		for (Common::List<Puzzle::Criteria>::iterator criteriaIter = activePuzzleIter->criteriaList.begin(); criteriaIter != (*activePuzzleIter).criteriaList.end(); criteriaIter++) {
			_referenceTable[criteriaIter->key].push_back(puzzlePtr);

			// If the argument is a key, add a reference to it as well
			if (criteriaIter->argumentIsAKey)
				_referenceTable[criteriaIter->argument].push_back(puzzlePtr);
		}
	}

	// Iterate through each global Puzzle
	for (Common::List<Puzzle>::iterator globalPuzzleIter = _globalPuzzles.begin(); globalPuzzleIter != _globalPuzzles.end(); globalPuzzleIter++) {
		Puzzle *puzzlePtr = &(*globalPuzzleIter);

		// Iterate through each Criteria and add a reference from the criteria key to the Puzzle
		for (Common::List<Puzzle::Criteria>::iterator criteriaIter = globalPuzzleIter->criteriaList.begin(); criteriaIter != (*globalPuzzleIter).criteriaList.end(); criteriaIter++) {
			_referenceTable[criteriaIter->key].push_back(puzzlePtr);

			// If the argument is a key, add a reference to it as well
			if (criteriaIter->argumentIsAKey)
				_referenceTable[criteriaIter->argument].push_back(puzzlePtr);
		}
	}

	// Remove duplicate entries
	for (Common::HashMap<uint32, Common::Array<Puzzle *> >::iterator referenceTableIter = _referenceTable.begin(); referenceTableIter != _referenceTable.end(); referenceTableIter++) {
		removeDuplicateEntries(referenceTableIter->_value);
	}
}

void ScriptManager::updateNodes(uint deltaTimeMillis) {
	// If process() returns true, it means the node can be deleted
	for (Common::List<Common::SharedPtr<ActionNode> >::iterator iter = _activeNodes.begin(); iter != _activeNodes.end();) {
		if ((*iter)->process(_engine, deltaTimeMillis)) {
			// Remove the node from _activeNodes, the SharedPtr destructor will delete the actual ActionNode
			iter = _activeNodes.erase(iter);
		} else {
			iter++;
		}
	}
}

void ScriptManager::checkPuzzleCriteria() {
	while (!_puzzlesToCheck.empty()) {
		Puzzle *puzzle = _puzzlesToCheck.pop();

		// Check if the puzzle is already finished
		// If it doesn't have the flag ONCE_PER_INST it can be done more than once
		// Also check that the puzzle isn't disabled
		if (getStateValue(puzzle->key) == 1 &&
		    (puzzle->flags & Puzzle::ONCE_PER_INST) == Puzzle::ONCE_PER_INST &&
		    (puzzle->flags & Puzzle::DISABLED) == 0) {
			continue;
		}

		// Check each Criteria
		bool criteriaMet = false;
		for (Common::List<Puzzle::Criteria>::iterator iter = puzzle->criteriaList.begin(); iter != puzzle->criteriaList.end(); iter++) {
			// Get the value to compare against
			uint argumentValue;
			if ((*iter).argumentIsAKey)
				argumentValue = getStateValue(iter->argument);
			else
				argumentValue = iter->argument;

			// Do the comparison
			switch ((*iter).criteriaOperator) {
			case Puzzle::EQUAL_TO:
				criteriaMet = getStateValue(iter->key) == argumentValue;
				break;
			case Puzzle::NOT_EQUAL_TO:
				criteriaMet = getStateValue(iter->key) != argumentValue;
				break;
			case Puzzle::GREATER_THAN:
				criteriaMet = getStateValue(iter->key) > argumentValue;
				break;
			case Puzzle::LESS_THAN:
				criteriaMet = getStateValue(iter->key) < argumentValue;
				break;
			}

			if (!criteriaMet) {
				break;
			}
		}

		// criteriaList can be empty. Aka, the puzzle should be executed immediately
		if (puzzle->criteriaList.empty() || criteriaMet) {
			debug("Puzzle %u criteria passed. Executing its ResultActions", puzzle->key);

			bool shouldContinue = true;
			for (Common::List<Common::SharedPtr<ResultAction> >::iterator resultIter = puzzle->resultActions.begin(); resultIter != puzzle->resultActions.end(); resultIter++) {
				shouldContinue = shouldContinue && (*resultIter)->execute(_engine);
			}

			// Set the puzzle as completed
			setStateValue(puzzle->key, 1);

			if (!shouldContinue) {
				break;
			}
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
		for (Common::Array<Puzzle *>::iterator iter = _referenceTable[key].begin(); iter != _referenceTable[key].end(); iter++) {
			_puzzlesToCheck.push((*iter));
		}
	}
}

void ScriptManager::addToStateValue(uint32 key, uint valueToAdd) {
	_globalState[key] += valueToAdd;
}

void ScriptManager::addActionNode(const Common::SharedPtr<ActionNode> &node) {
	_activeNodes.push_back(node);
}

void ScriptManager::changeLocation(char world, char room, char node, char view, uint32 offset) {
	_nextLocation.world = world;
	_nextLocation.room = room;
	_nextLocation.node = node;
	_nextLocation.view = view;
	_nextLocation.offset = offset;

	_changeLocation = true;
}

void ScriptManager::changeLocationIntern() {
	assert(_nextLocation.world != 0);
	debug("Changing location to: %c %c %c %c %u", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view, _nextLocation.offset);

	// Clear all the containers
	_referenceTable.clear();
	_puzzlesToCheck.clear();
	_activePuzzles.clear();
	// We can clear without deleting from the heap because we use SharedPtr
	_activeControls.clear();
	_engine->clearAllMouseEvents();

	// Revert to the idle cursor
	_engine->getCursorManager()->revertToIdle();

	// Change the background position
	_engine->getRenderManager()->setBackgroundPosition(_nextLocation.offset);

	// Reset the background velocity
	_engine->getRenderManager()->setBackgroundVelocity(0);

	// Parse into puzzles and controls
	Common::String fileName = Common::String::format("%c%c%c%c.scr", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view);
	parseScrFile(fileName);

	// Create the puzzle reference table
	createReferenceTable();

	// Add all the local puzzles to the stack to be checked
	for (Common::List<Puzzle>::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); iter++) {
		_puzzlesToCheck.push(&(*iter));
	}

	// Add all the global puzzles to the stack to be checked
	for (Common::List<Puzzle>::iterator iter = _globalPuzzles.begin(); iter != _globalPuzzles.end(); iter++) {
		_puzzlesToCheck.push(&(*iter));
	}
}

} // End of namespace ZVision
