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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/hashmap.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "zvision/zvision.h"
#include "zvision/file/save_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/scripting/actions.h"
#include "zvision/scripting/menu.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/effects/timer_effect.h"

namespace ZVision {

ScriptManager::ScriptManager(ZVision *engine)
	: _engine(engine),
	  _currentlyFocusedControl(0),
	  _activeControls(NULL) {
}

ScriptManager::~ScriptManager() {
	cleanScriptScope(_universe);
	cleanScriptScope(_world);
	cleanScriptScope(_room);
	cleanScriptScope(_nodeview);
	_controlEvents.clear();
}

void ScriptManager::initialize(bool restarted) {
	if (restarted) {
		_globalState.clear();
		_globalStateFlags.clear();
	}
	cleanScriptScope(_universe);
	cleanScriptScope(_world);
	cleanScriptScope(_room);
	cleanScriptScope(_nodeview);
	_currentLocation.node = 0;
	_currentLocation.world = 0;
	_currentLocation.room = 0;
	_currentLocation.view = 0;
	if (restarted) {
		for (auto &fx : _activeSideFx)
			delete fx;
		_activeSideFx.clear();
		_referenceTable.clear();
		switch (_engine->getGameId()) {
		case GID_GRANDINQUISITOR:
			// Bypass logo video
			setStateValue(16966, 1);
			// Ensure post-logo screen redraw is not inhibited in CD version
			setStateValue(5813, 1);
			// Bypass additional logo videos in DVD version
			setStateValue(19810, 1);
			setStateValue(19848, 1);
			break;
		case GID_NEMESIS:
		default:
			break;
		}
	}
	parseScrFile("universe.scr", _universe);
	changeLocation('g', 'a', 'r', 'y', 0);

	_controlEvents.clear();
	if (restarted)
		_engine->loadSettings();
}

bool ScriptManager::changingLocation() const {
	return _currentLocation != _nextLocation;
}

void ScriptManager::process(uint deltaTimeMillis) {
	// If the location is changing, the script that did that may have
	// triggered other scripts, so we give them all a few extra cycles to
	// run. This fixes some missing scoring in ZGI, and quite
	// possibly other minor glitches as well.
	//
	// Another idea would be to change if there are pending scripts
	// in the exec queues, but that could cause this to hang
	// indefinitely.
	for (uint8 pass = 0; pass <= changingLocation() ? _changeLocationExtraCycles : 0; pass++) {
		updateNodes(pass == 0 ? deltaTimeMillis : 0);
		debugC(5, kDebugLoop, "Script nodes updated");
		if (!execScope(_nodeview))
			break;
		if (!execScope(_room))
			break;
		if (!execScope(_world))
			break;
		if (!execScope(_universe))
			break;
	}
	updateControls(deltaTimeMillis);
	if (changingLocation())
		ChangeLocationReal(false);
}

bool ScriptManager::execScope(ScriptScope &scope) {
	// Swap queues
	PuzzleList *tmp = scope.execQueue;
	scope.execQueue = scope.scopeQueue;
	scope.scopeQueue = tmp;
	scope.scopeQueue->clear();

	for (auto &puzzle : scope.puzzles)
		puzzle->addedBySetState = false;

	switch (getStateValue(StateKey_ExecScopeStyle)) {
	case 0:	// ZGI
		if (scope.procCount < 2)
			for (auto &puzzle : scope.puzzles) {
				if (!checkPuzzleCriteria(puzzle, scope.procCount))
					return false;
			}
		else
			for (auto &puzzle : (*scope.execQueue)) {
				if (!checkPuzzleCriteria(puzzle, scope.procCount))
					return false;
			}
		break;
	case 1:	// Nemesis
	default:
		for (auto &puzzle : scope.puzzles) {
			if (!checkPuzzleCriteria(puzzle, scope.procCount))
				return false;
		}
		break;
	}

	if (scope.procCount < 2)
		scope.procCount++;
	return true;
}

void ScriptManager::referenceTableAddPuzzle(uint32 key, PuzzleRef ref) {
	if (_referenceTable.contains(key)) {
		Common::Array<PuzzleRef> *arr = &_referenceTable[key];
		for (uint32 i = 0; i < arr->size(); i++) {
			if ((*arr)[i].puz == ref.puz)
				return;
		}
	}
	_referenceTable[key].push_back(ref);
}

void ScriptManager::addPuzzlesToReferenceTable(ScriptScope &scope) {
	// Iterate through each local Puzzle
	for (auto &puzzle : scope.puzzles) {
		Puzzle *puzzlePtr = puzzle;

		PuzzleRef ref;
		ref.scope = &scope;
		ref.puz = puzzlePtr;

		referenceTableAddPuzzle(puzzlePtr->key, ref);

		// Iterate through each CriteriaEntry and add a reference from the criteria key to the Puzzle
		for (auto &criteria : puzzle->criteriaList) {
			for (auto &entry : criteria)
				referenceTableAddPuzzle(entry.key, ref);
		}
	}
}

void ScriptManager::updateNodes(uint deltaTimeMillis) {
	// If process() returns true, it means the node can be deleted
	for (auto fx = _activeSideFx.begin(); fx != _activeSideFx.end();) {
		if ((*fx)->process(deltaTimeMillis)) {
			delete(*fx);
			// Remove the node
			fx = _activeSideFx.erase(fx);
		}
		else
			++fx;
	}
}

void ScriptManager::updateControls(uint deltaTimeMillis) {
	if (!_activeControls)
		return;

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

	for (auto &control : (*_activeControls)) {
		if (control->process(deltaTimeMillis))
			break;
	}
}

bool ScriptManager::checkPuzzleCriteria(Puzzle *puzzle, uint counter) {
	// Check if the puzzle is already finished
	// Also check that the puzzle isn't disabled
	if (getStateValue(puzzle->key) == 1 || (getStateFlag(puzzle->key) & Puzzle::DISABLED))
		return true;

	// Check each Criteria
	if (counter == 0 && (getStateFlag(puzzle->key) & Puzzle::DO_ME_NOW) == 0)
		return true;

	// WORKAROUNDS:
	switch (_engine->getGameId()) {
	case GID_NEMESIS:
		switch (puzzle->key) {
		case 16418:
			// WORKAROUND for script bug in Zork Nemesis, room mc30 (Monastery Entry)
			// Rumble sound effect should cease upon changing location to me10 (Hall of Masks),
			// but this puzzle erroneously restarted it immediately after.
			if(changingLocation())
				return true;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	bool criteriaMet = false;
	for (auto &criteria : puzzle->criteriaList) {
		criteriaMet = false;

		for (auto &entry : criteria) {
			// Get the value to compare against
			int argumentValue;
			if (entry.argumentIsAKey)
				argumentValue = getStateValue(entry.argument);
			else
				argumentValue = entry.argument;

			// Do the comparison
			switch (entry.criteriaOperator) {
			case Puzzle::EQUAL_TO:
				criteriaMet = getStateValue(entry.key) == argumentValue;
				break;
			case Puzzle::NOT_EQUAL_TO:
				criteriaMet = getStateValue(entry.key) != argumentValue;
				break;
			case Puzzle::GREATER_THAN:
				criteriaMet = getStateValue(entry.key) > argumentValue;
				break;
			case Puzzle::LESS_THAN:
				criteriaMet = getStateValue(entry.key) < argumentValue;
				break;
			default:
				break;
			}

			// If one check returns false, don't keep checking
			if (!criteriaMet)
				break;
		}

		// If any of the Criteria are *fully* met, then execute the results
		if (criteriaMet)
			break;
	}

	// criteriaList can be empty. Aka, the puzzle should be executed immediately
	if (puzzle->criteriaList.empty() || criteriaMet) {
		debugC(1, kDebugPuzzle, "Puzzle %u criteria passed. Executing its ResultActions", puzzle->key);

		// Set the puzzle as completed
		setStateValue(puzzle->key, 1);

		for (auto &result : puzzle->resultActions) {
			if (!result->execute())
				return false;
		}
	}

	return true;
}

void ScriptManager::cleanStateTable() {
	for (auto entry = _globalState.begin(); entry != _globalState.end(); ++entry) {
		// If the value is equal to zero, we can purge it since getStateValue()
		// will return zero if _globalState doesn't contain a key
		if (entry->_value == 0) {
			// Remove the node
			_globalState.erase(entry);
		}
	}
}

void ScriptManager::cleanScriptScope(ScriptScope &scope) {
	scope.privQueueOne.clear();
	scope.privQueueTwo.clear();
	scope.scopeQueue = &scope.privQueueOne;
	scope.execQueue = &scope.privQueueTwo;
	for (auto &puzzle : scope.puzzles)
		delete(puzzle);

	scope.puzzles.clear();

	for (auto &control : scope.controls)
		delete(control);

	scope.controls.clear();

	scope.procCount = 0;
}

int ScriptManager::getStateValue(uint32 key) {
	if (_globalState.contains(key))
		return _globalState[key];
	else
		return 0;
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
	if (value == 0)
		_globalState.erase(key);
	else
		_globalState[key] = value;

	queuePuzzles(key);
}

void ScriptManager::setStateValueSilent(uint32 key, int value) {
	if (value == 0)
		_globalState.erase(key);
	else
		_globalState[key] = value;
}

uint ScriptManager::getStateFlag(uint32 key) {
	if (_globalStateFlags.contains(key))
		return _globalStateFlags[key];
	else
		return 0;
}

void ScriptManager::setStateFlag(uint32 key, uint value) {
	queuePuzzles(key);

	_globalStateFlags[key] |= value;
}

void ScriptManager::setStateFlagSilent(uint32 key, uint value) {
	if (value == 0)
		_globalStateFlags.erase(key);
	else
		_globalStateFlags[key] = value;
}

void ScriptManager::unsetStateFlag(uint32 key, uint value) {
	queuePuzzles(key);

	if (_globalStateFlags.contains(key)) {
		_globalStateFlags[key] &= ~value;
		if (_globalStateFlags[key] == 0)
			_globalStateFlags.erase(key);
	}
}

Control *ScriptManager::getControl(uint32 key) {
	for (auto &control : (*_activeControls)) {
		if (control->getKey() == key)
			return control;
	}

	return nullptr;
}

void ScriptManager::focusControl(uint32 key) {
	if (!_activeControls)
		return;

	if (_currentlyFocusedControl == key)
		return;

	for (auto &control : (*_activeControls)) {
		uint32 controlKey = control->getKey();
		if (controlKey == key)
			control->focus();
		else if (controlKey == _currentlyFocusedControl)
			control->unfocus();
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
	for (auto &fx : _activeSideFx) {
		if (fx->getKey() == key)
			return fx;
	}
	return nullptr;
}

void ScriptManager::deleteSideFx(uint32 key) {
	for (auto fx = _activeSideFx.begin(); fx != _activeSideFx.end(); ++fx) {
		if ((*fx)->getKey() == key) {
			delete(*fx);
			_activeSideFx.erase(fx);
			break;
		}
	}
}

void ScriptManager::stopSideFx(uint32 key) {
	for (auto fx = _activeSideFx.begin(); fx != _activeSideFx.end(); ++fx) {
		if ((*fx)->getKey() == key) {
			bool ret = (*fx)->stop();
			if (ret) {
				delete(*fx);
				_activeSideFx.erase(fx);
			}
			break;
		}
	}
}

void ScriptManager::killSideFx(uint32 key) {
	for (auto fx = _activeSideFx.begin(); fx != _activeSideFx.end(); ++fx) {
		if ((*fx)->getKey() == key) {
			(*fx)->kill();
			delete(*fx);
			_activeSideFx.erase(fx);
			break;
		}
	}
}

void ScriptManager::killSideFxType(ScriptingEffect::ScriptingEffectType type) {
	for (auto fx = _activeSideFx.begin(); fx != _activeSideFx.end();) {
		if ((*fx)->getType() & type) {
			(*fx)->kill();
			delete(*fx);
			fx = _activeSideFx.erase(fx);
		} else {
			++fx;
		}
	}
}

void ScriptManager::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	debugC(1, kDebugMouse, "Mouse screen coordinates: %d, %d, background/script coordinates: %d, %d", screenSpacePos.x, screenSpacePos.y, backgroundImageSpacePos.x, backgroundImageSpacePos.y);
	if (!_activeControls)
		return;

	for (auto control = _activeControls->reverse_begin(); control != _activeControls->end(); control--) {
		if ((*control)->onMouseDown(screenSpacePos, backgroundImageSpacePos))
			return;
	}
}

void ScriptManager::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_activeControls)
		return;

	for (auto control = _activeControls->reverse_begin(); control != _activeControls->end(); control--) {
		if ((*control)->onMouseUp(screenSpacePos, backgroundImageSpacePos))
			return;
	}
}

bool ScriptManager::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_activeControls)
		return false;

	for (auto control = _activeControls->reverse_begin(); control != _activeControls->end(); control--) {
		if ((*control)->onMouseMove(screenSpacePos, backgroundImageSpacePos))
			return true;
	}
	return false;
}

void ScriptManager::onKeyDown(Common::KeyState keyState) {
	if (!_activeControls)
		return;

	for (auto &control : (*_activeControls)) {
		if (control->onKeyDown(keyState))
			return;
	}
}

void ScriptManager::onKeyUp(Common::KeyState keyState) {
	if (!_activeControls)
		return;

	for (auto &control : (*_activeControls)) {
		if (control->onKeyUp(keyState))
			return;
	}
}

void ScriptManager::changeLocation(const Location &_newLocation) {
	changeLocation(_newLocation.world, _newLocation.room, _newLocation.node, _newLocation.view, _newLocation.offset);
}

void ScriptManager::changeLocation(char world, char room, char node, char view, uint32 offset) {
	debugC(1, kDebugScript, "\tPreparing to change location");
	_nextLocation.world = world;
	_nextLocation.room = room;
	_nextLocation.node = node;
	_nextLocation.view = view;
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
	debugC(1, kDebugScript, "\tChanging location to: World %c, Room %c, Node %c, View %c, Offset %u", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view, _nextLocation.offset);

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

	 _engine->setRenderDelay(2); // Necessary to ensure proper redraw in certain locations, in particular the infinite corridor in Zork Grand Inquisitor (room th20)

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
		if (isSaveScreen && !leavingMenu)
			_engine->getSaveManager()->prepareSaveBuffer();
	}
	else if (leavingMenu)
		_engine->getSaveManager()->flushSaveBuffer();

	setStateValue(StateKey_World, _nextLocation.world);
	setStateValue(StateKey_Room, _nextLocation.room);
	setStateValue(StateKey_Node, _nextLocation.node);
	setStateValue(StateKey_View, _nextLocation.view);
	setStateValue(StateKey_ViewPos, _nextLocation.offset);

	_referenceTable.clear();
	addPuzzlesToReferenceTable(_universe);

	_engine->getMenuManager()->setEnable(0xFFFF);

	TransitionLevel level = NONE;
	Common::Path filePath;
	if (_nextLocation.world != _currentLocation.world)
		level = WORLD;
	else if (_nextLocation.room != _currentLocation.room)
		level = ROOM;
	else if (_nextLocation.node != _currentLocation.node)
		level = NODE;
	else if (_nextLocation.view != _currentLocation.view)
		level = VIEW;

	switch (level) {
	case WORLD:
		cleanScriptScope(_world);
		filePath = Common::Path(Common::String::format("%c.scr", _nextLocation.world));
		parseScrFile(filePath, _world);
		// fall through
	case ROOM:
		cleanScriptScope(_room);
		filePath = Common::Path(Common::String::format("%c%c.scr", _nextLocation.world, _nextLocation.room));
		parseScrFile(filePath, _room);
		// fall through
	case NODE:
	case VIEW:
		cleanScriptScope(_nodeview);
		filePath = Common::Path(Common::String::format("%c%c%c%c.scr", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view));
		parseScrFile(filePath, _nodeview);
		addPuzzlesToReferenceTable(_world);
		addPuzzlesToReferenceTable(_room);
		addPuzzlesToReferenceTable(_nodeview);
		break;
	case NONE:
	default:
		break;
	}

	_activeControls = &_nodeview.controls;

	// Revert to the idle cursor
	_engine->getCursorManager()->changeCursor(CursorIndex_Idle);

	// Change the background position
	_engine->getRenderManager()->setBackgroundPosition(_nextLocation.offset);

	if (_currentLocation == "0000")
		level = WORLD;
	if (level != NONE)
		_currentLocation = _nextLocation;
	switch (level) {
	case WORLD:
		execScope(_world);
		// fall through
	case ROOM:
		execScope(_room);
		// fall through
	case NODE:
	case VIEW:
		execScope(_nodeview);
		break;
	case NONE:
	default:
		break;
	}

	_engine->getRenderManager()->checkBorders();

	_engine->onMouseMove();	// Trigger a pseudo mouse movement to change cursor if we enter the new location with it already over a hotspot

	debugC(1, kDebugScript, "\tLocation change complete");
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

	for (auto &fx : _activeSideFx)
		fx->serialize(stream);

	stream->writeUint32BE(MKTAG('F', 'L', 'A', 'G'));

	int32 slots = _engine->getGameId() == GID_NEMESIS ? 31000 : 21000;
	// Original games use key values up to 29500 and 19737, respectively
	// Values 30001~31000 and 20001~21000 are now set aside for auxiliary scripting to add extra directional audio effects.

	stream->writeUint32LE(slots * 2);

	for (int32 i = 0; i < slots; i++)
		stream->writeUint16LE(getStateFlag(i));

	stream->writeUint32BE(MKTAG('P', 'U', 'Z', 'Z'));
	stream->writeUint32LE(slots * 2);

	for (int32 i = 0; i < slots; i++)
		stream->writeSint16LE(getStateValue(i));
}

void ScriptManager::deserialize(Common::SeekableReadStream *stream) {
	// Clear out the current table values
	_globalState.clear();
	_globalStateFlags.clear();

	cleanScriptScope(_nodeview);
	cleanScriptScope(_room);
	cleanScriptScope(_world);

	_currentLocation.node = 0;
	_currentLocation.world = 0;
	_currentLocation.room = 0;
	_currentLocation.view = 0;

	for (auto &fx : _activeSideFx)
		delete fx;

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
			if (_engine->getGameId() == GID_GRANDINQUISITOR)
				time /= 100;
			else if (_engine->getGameId() == GID_NEMESIS)
				time /= 1000;
			addSideFX(new TimerNode(_engine, key, time));
		}
		break;
		case MKTAG('F', 'L', 'A', 'G'):
			for (uint32 i = 0; i < tagSize / 2; i++)
				setStateFlagSilent(i, stream->readUint16LE());
			break;
		case MKTAG('P', 'U', 'Z', 'Z'):
			for (uint32 i = 0; i < tagSize / 2; i++)
				setStateValueSilent(i, stream->readUint16LE());
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
	auto event = _controlEvents.begin();
	while (event != _controlEvents.end()) {
		if ((*event).type == type)
			event = _controlEvents.erase(event);
		else
			event++;
	}
}

void ScriptManager::trimCommentsAndWhiteSpace(Common::String *string) const {
	for (int i = string->size() - 1; i >= 0; i--) {
		if ((*string)[i] == '#')
			string->erase(i);
	}
	string->trim();
}

ValueSlot::ValueSlot(ScriptManager *scriptManager, const char *slotValue):
	_scriptManager(scriptManager) {
	_value = 0;
	_slot = false;
	const char *isSlot = strstr(slotValue, "[");
	if (isSlot) {
		_slot = true;
		_value = atoi(isSlot + 1);
	} else {
		_slot = false;
		_value = atoi(slotValue);
	}
}

int16 ValueSlot::getValue() {
	if (_slot) {
		if (_value >= 0)
			return _scriptManager->getStateValue(_value);
		else
			return 0;
	}
	else
		return _value;
}

} // End of namespace ZVision
