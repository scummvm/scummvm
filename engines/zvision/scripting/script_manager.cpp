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
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/file/save_manager.h"
#include "zvision/scripting/actions.h"
#include "zvision/scripting/menu.h"
#include "zvision/scripting/effects/timer_effect.h"

#include "common/algorithm.h"
#include "common/hashmap.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/config-manager.h"

namespace ZVision {

ScriptManager::ScriptManager(ZVision *engine)
	: _engine(engine),
	  _currentlyFocusedControl(0),
	  _activeControls(NULL) {
}

ScriptManager::~ScriptManager() {
	cleanScriptScope(universe);
	cleanScriptScope(world);
	cleanScriptScope(room);
	cleanScriptScope(nodeview);
	_controlEvents.clear();
}

void ScriptManager::initialize() {
	cleanScriptScope(universe);
	cleanScriptScope(world);
	cleanScriptScope(room);
	cleanScriptScope(nodeview);

	_currentLocation.node = 0;
	_currentLocation.world = 0;
	_currentLocation.room = 0;
	_currentLocation.view = 0;

	_changeLocationDelayCycles = 0;

	parseScrFile("universe.scr", universe);
	changeLocation('g', 'a', 'r', 'y', 0);

	_controlEvents.clear();
}

void ScriptManager::update(uint deltaTimeMillis) {
	if (_currentLocation != _nextLocation) {
		// The location is changing. The script that did that may have
		// triggered other scripts, so give them all one extra cycle to
		// run. This fixes some missing scoring in ZGI, and quite
		// possibly other minor glitches as well.
		//
		// Another idea would be to change if there are pending scripts
		// in the exec queues, but that could cause this to hang
		// indefinitely.
		if (_changeLocationDelayCycles-- <= 0) {
			ChangeLocationReal(false);
		}
	}

	updateNodes(deltaTimeMillis);
	if (!execScope(nodeview)) {
		return;
	}
	if (!execScope(room)) {
		return;
	}
	if (!execScope(world)) {
		return;
	}
	if (!execScope(universe)) {
		return;
	}
	updateControls(deltaTimeMillis);
}

bool ScriptManager::execScope(ScriptScope &scope) {
	// Swap queues
	PuzzleList *tmp = scope.execQueue;
	scope.execQueue = scope.scopeQueue;
	scope.scopeQueue = tmp;
	scope.scopeQueue->clear();

	for (PuzzleList::iterator PuzzleIter = scope.puzzles.begin(); PuzzleIter != scope.puzzles.end(); ++PuzzleIter) {
		(*PuzzleIter)->addedBySetState = false;
	}

	if (scope.procCount < 2 || getStateValue(StateKey_ExecScopeStyle)) {
		for (PuzzleList::iterator PuzzleIter = scope.puzzles.begin(); PuzzleIter != scope.puzzles.end(); ++PuzzleIter) {
			if (!checkPuzzleCriteria(*PuzzleIter, scope.procCount)) {
				return false;
			}
		}
	} else {
		for (PuzzleList::iterator PuzzleIter = scope.execQueue->begin(); PuzzleIter != scope.execQueue->end(); ++PuzzleIter) {
			if (!checkPuzzleCriteria(*PuzzleIter, scope.procCount)) {
				return false;
			}
		}
	}

	if (scope.procCount < 2) {
		scope.procCount++;
	}
	return true;
}

void ScriptManager::referenceTableAddPuzzle(uint32 key, PuzzleRef ref) {
	if (_referenceTable.contains(key)) {
		Common::Array<PuzzleRef> *arr = &_referenceTable[key];
		for (uint32 i = 0; i < arr->size(); i++) {
			if ((*arr)[i].puz == ref.puz) {
				return;
			}
		}
	}

	_referenceTable[key].push_back(ref);
}

void ScriptManager::addPuzzlesToReferenceTable(ScriptScope &scope) {
	// Iterate through each local Puzzle
	for (PuzzleList::iterator PuzzleIter = scope.puzzles.begin(); PuzzleIter != scope.puzzles.end(); ++PuzzleIter) {
		Puzzle *puzzlePtr = (*PuzzleIter);

		PuzzleRef ref;
		ref.scope = &scope;
		ref.puz = puzzlePtr;

		referenceTableAddPuzzle(puzzlePtr->key, ref);

		// Iterate through each CriteriaEntry and add a reference from the criteria key to the Puzzle
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = (*PuzzleIter)->criteriaList.begin(); criteriaIter != (*PuzzleIter)->criteriaList.end(); ++criteriaIter) {
			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = criteriaIter->begin(); entryIter != criteriaIter->end(); ++entryIter) {
				referenceTableAddPuzzle(entryIter->key, ref);
			}
		}
	}
}

void ScriptManager::updateNodes(uint deltaTimeMillis) {
	// If process() returns true, it means the node can be deleted
	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end();) {
		if ((*iter)->process(deltaTimeMillis)) {
			delete(*iter);
			// Remove the node
			iter = _activeSideFx.erase(iter);
		} else {
			++iter;
		}
	}
}

void ScriptManager::updateControls(uint deltaTimeMillis) {
	if (!_activeControls) {
		return;
	}

	// Process only one event
	if (!_controlEvents.empty()) {
		Common::Event _event = _controlEvents.front();
		Common::Point imageCoord;
		switch (_event.type) {
		case Common::EVENT_LBUTTONDOWN:
			imageCoord = _engine->getRenderManager()->screenSpaceToImageSpace(_event.mouse);
			onMouseDown(_event.mouse, imageCoord);
			break;
		case Common::EVENT_LBUTTONUP:
			imageCoord = _engine->getRenderManager()->screenSpaceToImageSpace(_event.mouse);
			onMouseUp(_event.mouse, imageCoord);
			break;
		case Common::EVENT_KEYDOWN:
			onKeyDown(_event.kbd);
			break;
		case Common::EVENT_KEYUP:
			onKeyUp(_event.kbd);
			break;
		default:
			break;
		}
		_controlEvents.pop_front();
	}

	for (ControlList::iterator iter = _activeControls->begin(); iter != _activeControls->end(); iter++) {
		if ((*iter)->process(deltaTimeMillis)) {
			break;
		}
	}
}

bool ScriptManager::checkPuzzleCriteria(Puzzle *puzzle, uint counter) {
	// Check if the puzzle is already finished
	// Also check that the puzzle isn't disabled
	if (getStateValue(puzzle->key) == 1 || (getStateFlag(puzzle->key) & Puzzle::DISABLED)) {
		return true;
	}

	// Check each Criteria
	if (counter == 0 && (getStateFlag(puzzle->key) & Puzzle::DO_ME_NOW) == 0) {
		return true;
	}

	bool criteriaMet = false;
	for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = puzzle->criteriaList.begin(); criteriaIter != puzzle->criteriaList.end(); ++criteriaIter) {
		criteriaMet = false;

		for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = criteriaIter->begin(); entryIter != criteriaIter->end(); ++entryIter) {
			// Get the value to compare against
			int argumentValue;
			if (entryIter->argumentIsAKey) {
				argumentValue = getStateValue(entryIter->argument);
			} else {
				argumentValue = entryIter->argument;
			}

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
			default:
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

		for (Common::List<ResultAction *>::iterator resultIter = puzzle->resultActions.begin(); resultIter != puzzle->resultActions.end(); ++resultIter) {
			if (!(*resultIter)->execute()) {
				return false;
			}
		}
	}

	return true;
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

void ScriptManager::cleanScriptScope(ScriptScope &scope) {
	scope.privQueueOne.clear();
	scope.privQueueTwo.clear();
	scope.scopeQueue = &scope.privQueueOne;
	scope.execQueue = &scope.privQueueTwo;
	for (PuzzleList::iterator iter = scope.puzzles.begin(); iter != scope.puzzles.end(); ++iter) {
		delete(*iter);
	}

	scope.puzzles.clear();

	for (ControlList::iterator iter = scope.controls.begin(); iter != scope.controls.end(); ++iter) {
		delete(*iter);
	}

	scope.controls.clear();

	scope.procCount = 0;
}

int ScriptManager::getStateValue(uint32 key) {
	if (_globalState.contains(key)) {
		return _globalState[key];
	} else {
		return 0;
	}
}

void ScriptManager::queuePuzzles(uint32 key) {
	if (_referenceTable.contains(key)) {
		Common::Array<PuzzleRef> *arr = &_referenceTable[key];
		for (int32 i = arr->size() - 1; i >= 0; i--) {
			if (!(*arr)[i].puz->addedBySetState) {
				(*arr)[i].scope->scopeQueue->push_back((*arr)[i].puz);
				(*arr)[i].puz->addedBySetState = true;
			}
		}
	}
}

void ScriptManager::setStateValue(uint32 key, int value) {
	if (value == 0) {
		_globalState.erase(key);
	} else {
		_globalState[key] = value;
	}

	queuePuzzles(key);
}

void ScriptManager::setStateValueSilent(uint32 key, int value) {
	if (value == 0) {
		_globalState.erase(key);
	} else {
		_globalState[key] = value;
	}
}

uint ScriptManager::getStateFlag(uint32 key) {
	if (_globalStateFlags.contains(key)) {
		return _globalStateFlags[key];
	} else {
		return 0;
	}
}

void ScriptManager::setStateFlag(uint32 key, uint value) {
	queuePuzzles(key);

	_globalStateFlags[key] |= value;
}

void ScriptManager::setStateFlagSilent(uint32 key, uint value) {
	if (value == 0) {
		_globalStateFlags.erase(key);
	} else {
		_globalStateFlags[key] = value;
	}
}

void ScriptManager::unsetStateFlag(uint32 key, uint value) {
	queuePuzzles(key);

	if (_globalStateFlags.contains(key)) {
		_globalStateFlags[key] &= ~value;

		if (_globalStateFlags[key] == 0) {
			_globalStateFlags.erase(key);
		}
	}
}

Control *ScriptManager::getControl(uint32 key) {
	for (ControlList::iterator iter = _activeControls->begin(); iter != _activeControls->end(); ++iter) {
		if ((*iter)->getKey() == key) {
			return *iter;
		}
	}

	return nullptr;
}

void ScriptManager::focusControl(uint32 key) {
	if (!_activeControls) {
		return;
	}
	if (_currentlyFocusedControl == key) {
		return;
	}
	for (ControlList::iterator iter = _activeControls->begin(); iter != _activeControls->end(); ++iter) {
		uint32 controlKey = (*iter)->getKey();

		if (controlKey == key) {
			(*iter)->focus();
		} else if (controlKey == _currentlyFocusedControl) {
			(*iter)->unfocus();
		}
	}

	_currentlyFocusedControl = key;
}

void ScriptManager::setFocusControlKey(uint32 key) {
	_currentlyFocusedControl = key;
}

void ScriptManager::addSideFX(ScriptingEffect *fx) {
	_activeSideFx.push_back(fx);
}

ScriptingEffect *ScriptManager::getSideFX(uint32 key) {
	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end(); ++iter) {
		if ((*iter)->getKey() == key) {
			return (*iter);
		}
	}

	return nullptr;
}

void ScriptManager::deleteSideFx(uint32 key) {
	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end(); ++iter) {
		if ((*iter)->getKey() == key) {
			delete(*iter);
			_activeSideFx.erase(iter);
			break;
		}
	}
}

void ScriptManager::stopSideFx(uint32 key) {
	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end(); ++iter) {
		if ((*iter)->getKey() == key) {
			bool ret = (*iter)->stop();
			if (ret) {
				delete(*iter);
				_activeSideFx.erase(iter);
			}
			break;
		}
	}
}

void ScriptManager::killSideFx(uint32 key) {
	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end(); ++iter) {
		if ((*iter)->getKey() == key) {
			(*iter)->kill();
			delete(*iter);
			_activeSideFx.erase(iter);
			break;
		}
	}
}

void ScriptManager::killSideFxType(ScriptingEffect::ScriptingEffectType type) {
	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end();) {
		if ((*iter)->getType() & type) {
			(*iter)->kill();
			delete(*iter);
			iter = _activeSideFx.erase(iter);
		} else {
			++iter;
		}
	}
}

void ScriptManager::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_activeControls) {
		return;
	}
	for (ControlList::iterator iter = _activeControls->reverse_begin(); iter != _activeControls->end(); iter--) {
		if ((*iter)->onMouseDown(screenSpacePos, backgroundImageSpacePos)) {
			return;
		}
	}
}

void ScriptManager::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_activeControls) {
		return;
	}
	for (ControlList::iterator iter = _activeControls->reverse_begin(); iter != _activeControls->end(); iter--) {
		if ((*iter)->onMouseUp(screenSpacePos, backgroundImageSpacePos)) {
			return;
		}
	}
}

bool ScriptManager::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_activeControls) {
		return false;
	}

	for (ControlList::iterator iter = _activeControls->reverse_begin(); iter != _activeControls->end(); iter--) {
		if ((*iter)->onMouseMove(screenSpacePos, backgroundImageSpacePos)) {
			return true;
		}
	}

	return false;
}

void ScriptManager::onKeyDown(Common::KeyState keyState) {
	if (!_activeControls) {
		return;
	}
	for (ControlList::iterator iter = _activeControls->begin(); iter != _activeControls->end(); ++iter) {
		if ((*iter)->onKeyDown(keyState)) {
			return;
		}
	}
}

void ScriptManager::onKeyUp(Common::KeyState keyState) {
	if (!_activeControls) {
		return;
	}
	for (ControlList::iterator iter = _activeControls->begin(); iter != _activeControls->end(); ++iter) {
		if ((*iter)->onKeyUp(keyState)) {
			return;
		}
	}
}

void ScriptManager::changeLocation(const Location &_newLocation) {
	changeLocation(_newLocation.world, _newLocation.room, _newLocation.node, _newLocation.view, _newLocation.offset);
}

void ScriptManager::changeLocation(char _world, char _room, char _node, char _view, uint32 offset) {
	_changeLocationDelayCycles = 1;

	_nextLocation.world = _world;
	_nextLocation.room = _room;
	_nextLocation.node = _node;
	_nextLocation.view = _view;
	_nextLocation.offset = offset;
	// If next location is 0000, return to the previous location.
	if (_nextLocation == "0000") {
		if (getStateValue(StateKey_World) != 'g' || getStateValue(StateKey_Room) != 'j') {
			_nextLocation.world = getStateValue(StateKey_LastWorld);
			_nextLocation.room = getStateValue(StateKey_LastRoom);
			_nextLocation.node = getStateValue(StateKey_LastNode);
			_nextLocation.view = getStateValue(StateKey_LastView);
			_nextLocation.offset = getStateValue(StateKey_LastViewPos);
		} else {
			_nextLocation.world = getStateValue(StateKey_Menu_LastWorld);
			_nextLocation.room = getStateValue(StateKey_Menu_LastRoom);
			_nextLocation.node = getStateValue(StateKey_Menu_LastNode);
			_nextLocation.view = getStateValue(StateKey_Menu_LastView);
			_nextLocation.offset = getStateValue(StateKey_Menu_LastViewPos);
		}
	}
}

void ScriptManager::ChangeLocationReal(bool isLoading) {
	assert(_nextLocation.world != 0);
	debug(1, "Changing location to: %c %c %c %c %u", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view, _nextLocation.offset);

	const bool enteringMenu = (_nextLocation.world == 'g' && _nextLocation.room == 'j');
	const bool leavingMenu = (_currentLocation.world == 'g' && _currentLocation.room == 'j');
	const bool isSaveScreen = (enteringMenu && _nextLocation.node == 's' && _nextLocation.view == 'e');
	const bool isRestoreScreen = (enteringMenu && _nextLocation.node == 'r' && _nextLocation.view == 'e');

	if (enteringMenu && !ConfMan.getBool("originalsaveload")) {
		if (isSaveScreen || isRestoreScreen) {
			// Hook up the ScummVM save/restore dialog
			bool gameSavedOrLoaded = _engine->getSaveManager()->scummVMSaveLoadDialog(isSaveScreen);
			if (!gameSavedOrLoaded || isSaveScreen) {
				// Reload the current room
				_nextLocation.world = _currentLocation.world;
				_nextLocation.room = _currentLocation.room;
				_nextLocation.node = _currentLocation.node;
				_nextLocation.view = _currentLocation.view;
				_nextLocation.offset = _currentLocation.offset;

				return;
			} else {
				_currentLocation.world = 'g';
				_currentLocation.room = '0';
				_currentLocation.node = '0';
				_currentLocation.view = '0';
				_currentLocation.offset = 0;
			}
		}
	}

	_engine->setRenderDelay(2);

	if (!leavingMenu) {
		if (!isLoading && !enteringMenu) {
			setStateValue(StateKey_LastWorld, getStateValue(StateKey_World));
			setStateValue(StateKey_LastRoom, getStateValue(StateKey_Room));
			setStateValue(StateKey_LastNode, getStateValue(StateKey_Node));
			setStateValue(StateKey_LastView, getStateValue(StateKey_View));
			setStateValue(StateKey_LastViewPos, getStateValue(StateKey_ViewPos));
		} else {
			setStateValue(StateKey_Menu_LastWorld, getStateValue(StateKey_World));
			setStateValue(StateKey_Menu_LastRoom, getStateValue(StateKey_Room));
			setStateValue(StateKey_Menu_LastNode, getStateValue(StateKey_Node));
			setStateValue(StateKey_Menu_LastView, getStateValue(StateKey_View));
			setStateValue(StateKey_Menu_LastViewPos, getStateValue(StateKey_ViewPos));
		}
	}

	if (enteringMenu) {
		if (isSaveScreen && !leavingMenu) {
			_engine->getSaveManager()->prepareSaveBuffer();
		}
	} else {
		if (leavingMenu) {
			_engine->getSaveManager()->flushSaveBuffer();
		}
	}

	setStateValue(StateKey_World, _nextLocation.world);
	setStateValue(StateKey_Room, _nextLocation.room);
	setStateValue(StateKey_Node, _nextLocation.node);
	setStateValue(StateKey_View, _nextLocation.view);
	setStateValue(StateKey_ViewPos, _nextLocation.offset);

	_referenceTable.clear();
	addPuzzlesToReferenceTable(universe);

	_engine->getMenuHandler()->setEnable(0xFFFF);

	if (_nextLocation.world != _currentLocation.world) {
		cleanScriptScope(nodeview);
		cleanScriptScope(room);
		cleanScriptScope(world);

		Common::String fileName = Common::String::format("%c%c%c%c.scr", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view);
		parseScrFile(fileName, nodeview);
		addPuzzlesToReferenceTable(nodeview);

		fileName = Common::String::format("%c%c.scr", _nextLocation.world, _nextLocation.room);
		parseScrFile(fileName, room);
		addPuzzlesToReferenceTable(room);

		fileName = Common::String::format("%c.scr", _nextLocation.world);
		parseScrFile(fileName, world);
		addPuzzlesToReferenceTable(world);
	} else if (_nextLocation.room != _currentLocation.room) {
		cleanScriptScope(nodeview);
		cleanScriptScope(room);

		addPuzzlesToReferenceTable(world);

		Common::String fileName = Common::String::format("%c%c%c%c.scr", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view);
		parseScrFile(fileName, nodeview);
		addPuzzlesToReferenceTable(nodeview);

		fileName = Common::String::format("%c%c.scr", _nextLocation.world, _nextLocation.room);
		parseScrFile(fileName, room);
		addPuzzlesToReferenceTable(room);

	} else if (_nextLocation.node != _currentLocation.node || _nextLocation.view != _currentLocation.view) {
		cleanScriptScope(nodeview);

		addPuzzlesToReferenceTable(room);
		addPuzzlesToReferenceTable(world);

		Common::String fileName = Common::String::format("%c%c%c%c.scr", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view);
		parseScrFile(fileName, nodeview);
		addPuzzlesToReferenceTable(nodeview);
	}

	_activeControls = &nodeview.controls;

	// Revert to the idle cursor
	_engine->getCursorManager()->changeCursor(CursorIndex_Idle);

	// Change the background position
	_engine->getRenderManager()->setBackgroundPosition(_nextLocation.offset);

	if (_currentLocation == "0000") {
		_currentLocation = _nextLocation;
		execScope(world);
		execScope(room);
		execScope(nodeview);
	} else if (_nextLocation.world != _currentLocation.world) {
		_currentLocation = _nextLocation;
		execScope(room);
		execScope(nodeview);
	} else if (_nextLocation.room != _currentLocation.room) {
		_currentLocation = _nextLocation;
		execScope(room);
		execScope(nodeview);
	} else if (_nextLocation.node != _currentLocation.node || _nextLocation.view != _currentLocation.view) {
		_currentLocation = _nextLocation;
		execScope(nodeview);
	}

	_engine->getRenderManager()->checkBorders();
}

void ScriptManager::serialize(Common::WriteStream *stream) {
	stream->writeUint32BE(MKTAG('Z', 'N', 'S', 'G'));
	stream->writeUint32LE(4);
	stream->writeUint32LE(0);
	stream->writeUint32BE(MKTAG('L', 'O', 'C', ' '));
	stream->writeUint32LE(8);
	stream->writeByte(getStateValue(StateKey_World));
	stream->writeByte(getStateValue(StateKey_Room));
	stream->writeByte(getStateValue(StateKey_Node));
	stream->writeByte(getStateValue(StateKey_View));
	stream->writeUint32LE(getStateValue(StateKey_ViewPos));

	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end(); ++iter) {
		(*iter)->serialize(stream);
	}

	stream->writeUint32BE(MKTAG('F', 'L', 'A', 'G'));

	int32 slots = 20000;
	if (_engine->getGameId() == GID_NEMESIS) {
		slots = 30000;
	}

	stream->writeUint32LE(slots * 2);

	for (int32 i = 0; i < slots; i++) {
		stream->writeUint16LE(getStateFlag(i));
	}

	stream->writeUint32BE(MKTAG('P', 'U', 'Z', 'Z'));

	stream->writeUint32LE(slots * 2);

	for (int32 i = 0; i < slots; i++) {
		stream->writeSint16LE(getStateValue(i));
	}
}

void ScriptManager::deserialize(Common::SeekableReadStream *stream) {
	// Clear out the current table values
	_globalState.clear();
	_globalStateFlags.clear();

	cleanScriptScope(nodeview);
	cleanScriptScope(room);
	cleanScriptScope(world);

	_currentLocation.node = 0;
	_currentLocation.world = 0;
	_currentLocation.room = 0;
	_currentLocation.view = 0;

	for (SideFXList::iterator iter = _activeSideFx.begin(); iter != _activeSideFx.end(); iter++) {
		delete(*iter);
	}

	_activeSideFx.clear();

	_referenceTable.clear();

	if (stream->readUint32BE() != MKTAG('Z', 'N', 'S', 'G') || stream->readUint32LE() != 4) {
		changeLocation('g', 'a', 'r', 'y', 0);
		return;
	}

	stream->seek(4, SEEK_CUR);

	if (stream->readUint32BE() != MKTAG('L', 'O', 'C', ' ') || stream->readUint32LE() != 8) {
		changeLocation('g', 'a', 'r', 'y', 0);
		return;
	}

	Location nextLocation;

	nextLocation.world = stream->readByte();
	nextLocation.room = stream->readByte();
	nextLocation.node = stream->readByte();
	nextLocation.view = stream->readByte();
	nextLocation.offset = stream->readUint32LE() & 0x0000FFFF;

	while (stream->pos() < stream->size()) {
		uint32 tag = stream->readUint32BE();
		uint32 tagSize = stream->readUint32LE();
		switch (tag) {
		case MKTAG('T', 'I', 'M', 'R'): {
			uint32 key = stream->readUint32LE();
			uint32 time = stream->readUint32LE();
			if (_engine->getGameId() == GID_GRANDINQUISITOR) {
				time /= 100;
			} else if (_engine->getGameId() == GID_NEMESIS) {
				time /= 1000;
			}
			addSideFX(new TimerNode(_engine, key, time));
		}
		break;
		case MKTAG('F', 'L', 'A', 'G'):
			for (uint32 i = 0; i < tagSize / 2; i++) {
				setStateFlagSilent(i, stream->readUint16LE());
			}
			break;
		case MKTAG('P', 'U', 'Z', 'Z'):
			for (uint32 i = 0; i < tagSize / 2; i++) {
				setStateValueSilent(i, stream->readUint16LE());
			}
			break;
		default:
			stream->seek(tagSize, SEEK_CUR);
		}
	}

	_nextLocation = nextLocation;

	ChangeLocationReal(true);

	_engine->setRenderDelay(10);
	setStateValue(StateKey_RestoreFlag, 1);

	_engine->loadSettings();
}

Location ScriptManager::getCurrentLocation() const {
	Location location = _currentLocation;
	location.offset = _engine->getRenderManager()->getCurrentBackgroundOffset();

	return location;
}

Location ScriptManager::getLastLocation() {
	Location location;
	location.world = getStateValue(StateKey_LastWorld);
	location.room = getStateValue(StateKey_LastRoom);
	location.node = getStateValue(StateKey_LastNode);
	location.view = getStateValue(StateKey_LastView);
	location.offset = getStateValue(StateKey_LastViewPos);

	return location;
}

Location ScriptManager::getLastMenuLocation() {
	Location location;
	location.world = getStateValue(StateKey_Menu_LastWorld);
	location.room = getStateValue(StateKey_Menu_LastRoom);
	location.node = getStateValue(StateKey_Menu_LastNode);
	location.view = getStateValue(StateKey_Menu_LastView);
	location.offset = getStateValue(StateKey_Menu_LastViewPos);

	return location;
}

void ScriptManager::addEvent(Common::Event event) {
	_controlEvents.push_back(event);
}

void ScriptManager::flushEvent(Common::EventType type) {
	EventList::iterator it = _controlEvents.begin();
	while (it != _controlEvents.end()) {

		if ((*it).type == type) {
			it = _controlEvents.erase(it);
		} else {
			it++;
		}
	}
}

void ScriptManager::trimCommentsAndWhiteSpace(Common::String *string) const {
	for (int i = string->size() - 1; i >= 0; i--) {
		if ((*string)[i] == '#') {
			string->erase(i);
		}
	}

	string->trim();
}

ValueSlot::ValueSlot(ScriptManager *scriptManager, const char *slotValue):
	_scriptManager(scriptManager) {
	value = 0;
	slot = false;
	const char *isSlot = strstr(slotValue, "[");
	if (isSlot) {
		slot = true;
		value = atoi(isSlot + 1);
	} else {
		slot = false;
		value = atoi(slotValue);
	}
}
int16 ValueSlot::getValue() {
	if (slot) {
		if (value >= 0) {
			return _scriptManager->getStateValue(value);
		}
		else {
			return 0;
		}
	} else {
		return value;
	}
}

} // End of namespace ZVision
