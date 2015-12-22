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
#include "lab/processroom.h"
#include "lab/resource.h"

namespace Lab {

Console::Console(LabEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("scene",			WRAP_METHOD(Console, Cmd_Scene));
	registerCmd("scene_resources",  WRAP_METHOD(Console, Cmd_DumpSceneResources));
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
	RuleList *rules = roomData->_rules;
	const char *transitions[] = { "None", "Wipe", "ScrollWipe", "ScrollBlack", "ScrollBounce", "Transporter", "ReadFirstFrame", "ReadNextFrame" };
	const char *ruleTypes[] = { "None", "Action", "Operate", "Go forward", "Conditions", "Turn", "Go main view", "Turn from to" };

	debugPrintf("Room mesage: %s\n", roomData->_roomMsg.c_str());
	debugPrintf("Transition: %s (%d)\n", transitions[roomData->_transitionType], roomData->_transitionType);

	debugPrintf("Script:\n");
	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		debugPrintf("Rule type: %s\n", ruleTypes[rule->_ruleType]);

	}

	return true;
}

} // End of namespace Neverhood
