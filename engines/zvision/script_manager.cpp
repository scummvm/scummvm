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

#include "zvision/script_manager.h"
#include "zvision/actions.h"
#include "zvision/action_node.h"
#include "zvision/utility.h"

namespace ZVision {

ScriptManager::ScriptManager(ZVision *engine) : _engine(engine) {}

// TODO: Actually do something in the initialize or remove it
void ScriptManager::initialize() {
	
}

void ScriptManager::createReferenceTable() {
	// Iterate through each Puzzle
	for (Common::List<Puzzle>::iterator activePuzzleIter = _activePuzzles.begin(); activePuzzleIter != _activePuzzles.end(); activePuzzleIter++) {
		Puzzle *puzzlePtr = &(*activePuzzleIter);

		// Iterate through each Criteria and add a reference from the criteria key to the Puzzle
		for (Common::List<Criteria>::iterator criteriaIter = activePuzzleIter->criteriaList.begin(); criteriaIter != (*activePuzzleIter).criteriaList.end(); criteriaIter++) {
			_referenceTable[criteriaIter->key].push_back(puzzlePtr);

			// If the argument is a key, add a reference to it as well
			if (criteriaIter->argument)
				_referenceTable[criteriaIter->argument].push_back(puzzlePtr);
		}
	}

	// Remove duplicate entries
	for (Common::HashMap<uint32, Common::Array<Puzzle *> >::iterator referenceTableIter; referenceTableIter != _referenceTable.end(); referenceTableIter++) {
		removeDuplicateEntries(&(referenceTableIter->_value));
	}
}

void ScriptManager::updateNodes(uint32 deltaTimeMillis) {
	// If process() returns true, it means the node can be deleted
	for (Common::List<ActionNode *>::iterator iter = _activeNodes.begin(); iter != _activeNodes.end();) {
		if ((*iter)->process(_engine, deltaTimeMillis)) {
			// Remove the node from _activeNodes, then delete it
			ActionNode *node = *iter;
			iter = _activeNodes.erase(iter);
			delete node;
		} else {
			iter++;
		}
	}
}

void ScriptManager::checkPuzzleCriteria() {
	while (!_puzzlesToCheck.empty()) {
		Puzzle *puzzle = _puzzlesToCheck.pop();

		// Check each Criteria
		bool criteriaMet = false;
		for (Common::List<Criteria>::iterator iter = puzzle->criteriaList.begin(); iter != puzzle->criteriaList.end(); iter++) {
			// Get the value to compare against
			byte argumentValue;
			if ((*iter).argument)
				argumentValue = getStateValue(iter->argument);
			else
				argumentValue = iter->argument;

			// Do the comparison
			switch ((*iter).criteriaOperator) {
			case EQUAL_TO:
				criteriaMet = getStateValue(iter->key) == argumentValue;
				break;
			case NOT_EQUAL_TO:
				criteriaMet = getStateValue(iter->key) != argumentValue;
				break;
			case GREATER_THAN:
				criteriaMet = getStateValue(iter->key) > argumentValue;
				break;
			case LESS_THAN:
				criteriaMet = getStateValue(iter->key) < argumentValue;
				break;
			}

			if (!criteriaMet) {
				break;
			}
		}

		// TODO: Add logic for the different Flags (aka, ONCE_PER_INST)
		// criteriaList can be empty. Aka, the puzzle should be executed immediately
		if (puzzle->criteriaList.empty() || criteriaMet) {
			for (Common::List<ResultAction *>::iterator resultIter = puzzle->resultActions.begin(); resultIter != puzzle->resultActions.end(); resultIter++) {
				(*resultIter)->execute(_engine);
			}
		}
	}
}

byte ScriptManager::getStateValue(uint32 key) {
	return _globalState[key];
}

// TODO: Add logic to check _referenceTable and add to _puzzlesToCheck if necessary
void ScriptManager::setStateValue(uint32 key, byte value) {
	_globalState[key] = value;
}

void ScriptManager::addToStateValue(uint32 key, byte valueToAdd) {
	_globalState[key] += valueToAdd;
}

void ScriptManager::addActionNode(ActionNode *node) {
	_activeNodes.push_back(node);
}

} // End of namespace ZVision
