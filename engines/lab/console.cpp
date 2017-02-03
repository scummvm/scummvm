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

#include "gui/debugger.h"

#include "lab/lab.h"
#include "lab/console.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"

namespace Lab {

Console::Console(LabEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("scene",			WRAP_METHOD(Console, Cmd_Scene));
	registerCmd("scene_resources",  WRAP_METHOD(Console, Cmd_DumpSceneResources));
	registerCmd("find_action",      WRAP_METHOD(Console, Cmd_FindAction));
}

Console::~Console() {
}

bool Console::Cmd_Scene(int argc, const char **argv) {
	if (argc != 2) {
		const char *directions[] = { "North", "South", "East", "West" };
		debugPrintf("Current scene is %d, direction: %s\n", _vm->_roomNum, directions[_vm->getDirection()]);
		debugPrintf("Use %s <scene number> to change the current scene\n", argv[0]);
		return true;
	}

	_vm->_roomNum = atoi(argv[1]);
	_vm->_music->checkRoomMusic(1, _vm->_roomNum);
	_vm->_curFileName = " ";
	_vm->_closeDataPtr = nullptr;
	_vm->_mainDisplay = true;
	_vm->_followingCrumbs = false;
	_vm->_event->simulateEvent();
	_vm->_graphics->_longWinInFront = false;

	return false;
}

bool Console::Cmd_DumpSceneResources(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <scene number> to dump the resources for a scene\n", argv[0]);
		return true;
	}

	int scene = atoi(argv[1]);
	_vm->_resource->readViews(scene);
	RoomData *roomData = &_vm->_rooms[scene];
	RuleList &rules = roomData->_rules;
	const char *transitions[] = { "None", "Wipe", "ScrollWipe", "ScrollBlack", "ScrollBounce", "Transporter", "ReadFirstFrame", "ReadNextFrame" };
	const char *ruleTypes[] = { "None", "Action", "Operate", "Go forward", "Conditions", "Turn", "Go main view", "Turn from to" };
	const char *directions[] = { "", "North", "South", "East", "West" };
	const char *actionTypes[] = {
		"", "PlaySound", "PlaySoundLooping", "ShowDiff", "ShowDiffLooping", "LoadDiff", "LoadBitmap", "ShowBitmap", "Transition", "NoUpdate", "ForceUpdate",
		"ShowCurPict", "SetElement", "UnsetElement", "ShowMessage", "ShowMessages", "ChangeRoom", "SetCloseup", "MainView", "SubInv", "AddInv", "ShowDir",
		"WaitSecs", "StopMusic", "StartMusic", "ChangeMusic", "ResetMusic", "FillMusic", "WaitSound", "ClearSound", "WinMusic", "WinGame", "LostGame",
		"ResetBuffer", "SpecialCmd", "CShowMessage", "PlaySoundNoWait"
	};

	debugPrintf("Room message: %s\n", roomData->_roomMsg.c_str());
	debugPrintf("Transition: %s (%d)\n", transitions[roomData->_transitionType], roomData->_transitionType);

	debugPrintf("Script:\n");

	for (RuleList::iterator rule = rules.begin(); rule != rules.end(); ++rule) {
		debugPrintf("Rule type: %s", ruleTypes[rule->_ruleType]);
		if (rule->_ruleType == kRuleTypeAction || rule->_ruleType == kRuleTypeOperate)
			debugPrintf(" (item %d, closeup %d)", rule->_param1, rule->_param2);
		else if (rule->_ruleType == kRuleTypeGoForward)
			debugPrintf(" (%s)", directions[rule->_param1]);
		else if (rule->_ruleType == kRuleTypeTurnFromTo)
			debugPrintf(" (from %s to %s)", directions[rule->_param1], directions[rule->_param2]);
		debugPrintf("\n");

		ActionList::iterator action;
		for (action = rule->_actionList.begin(); action != rule->_actionList.end(); ++action) {
			debugPrintf("  - %s ('%s', %d, %d, %d)\n", actionTypes[action->_actionType], action->_messages[0].c_str(), action->_param1, action->_param2, action->_param3);
		}
	}

	return true;
}

bool Console::Cmd_FindAction(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <action id> [param 1] [param 2] [param 3]\n", argv[0]);
		return true;
	}

	int actionId = atoi(argv[1]);
	int param1 = (argc > 2) ? atoi(argv[2]) : -1;
	int param2 = (argc > 3) ? atoi(argv[3]) : -1;
	int param3 = (argc > 4) ? atoi(argv[4]) : -1;

	for (int i = 1; i <= _vm->_manyRooms; i++) {
		_vm->_resource->readViews(i);

		for (RuleList::iterator rule = _vm->_rooms[i]._rules.begin(); rule != _vm->_rooms[i]._rules.end(); ++rule) {
			ActionList::iterator action;
			for (action = rule->_actionList.begin(); action != rule->_actionList.end(); ++action) {
				if (action->_actionType == actionId &&
					(action->_param1 == param1 || param1 == -1) &&
					(action->_param2 == param2 || param2 == -1) &&
					(action->_param3 == param3 || param3 == -1)) {
						debugPrintf("Found at script %d\n", i);
				}
			}
		}
	}

	return true;
}

} // End of namespace Neverhood
