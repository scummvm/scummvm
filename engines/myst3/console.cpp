/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/myst3/console.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Console::Console(Myst3Engine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("infos",				WRAP_METHOD(Console, Cmd_Infos));
	DCmd_Register("lookAt",				WRAP_METHOD(Console, Cmd_LookAt));
	DCmd_Register("initScript",			WRAP_METHOD(Console, Cmd_InitScript));
	DCmd_Register("var",				WRAP_METHOD(Console, Cmd_Var));
	DCmd_Register("listNodes",			WRAP_METHOD(Console, Cmd_ListNodes));
	DCmd_Register("run",				WRAP_METHOD(Console, Cmd_Run));
}

Console::~Console() {
}

void Console::describeScript(const Common::Array<Opcode> &script) {
	Common::String d;

	for(uint j = 0; j < script.size(); j++) {
		const Opcode &opcode = script[j];

		d = Common::String::format("    op %s ( ",
				_vm->_scriptEngine->describeCommand(opcode.op).c_str());

		for(uint k = 0; k < opcode.args.size(); k++) {
			d += Common::String::format("%d ", opcode.args[k]);
		}

		d += ")\n";

		DebugPrintf("%s", d.c_str());
	}
}

Common::String Console::describeCondition(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int16 value = (unsignedCond >> 11) - 1;

	if (value < 0)
		value = 1;

	return Common::String::format("var[%d] %s %d (%s)",
			var, condition > 0 ? "==" : "!=", value,
			_vm->_vars->evaluate(condition) ? "true" : "false");
}

bool Console::Cmd_Infos(int argc, const char **argv) {

	uint16 nodeId = _vm->_currentNode;
	uint16 roomId = 0;

	if (argc >= 2) {
		nodeId = atoi(argv[1]);
	}

	if (argc >= 3) {
		roomId = atoi(argv[2]);
	}

	NodePtr nodeData = _vm->_db->getNodeData(nodeId, roomId);

	char roomName[8];
	_vm->_db->getRoomName(roomName, roomId);

	Common::Point lookAt = _vm->_scene->getMousePos();

	DebugPrintf("node: %s%d    ", roomName, nodeId);
	DebugPrintf("pitch: %d heading: %d",  lookAt.y, lookAt.x);

	for (uint i = 0; i < nodeData->hotspots.size(); i++) {
		DebugPrintf("\nhotspot %d > condition: %s\n",
				i, describeCondition(nodeData->hotspots[i].condition).c_str());

		for(uint j = 0; j < nodeData->hotspots[i].rects.size(); j++) {
			PolarRect &rect = nodeData->hotspots[i].rects[j];

			DebugPrintf("    rect > pitch: %d heading: %d height: %d width: %d\n",
					rect.centerPitch, rect.centerHeading, rect.width, rect.height);
		}

		describeScript(nodeData->hotspots[i].script);
	}

	for (uint i = 0; i < nodeData->scripts.size(); i++) {
		DebugPrintf("\nscript %d > condition: %s\n",
				i, describeCondition(nodeData->scripts[i].condition).c_str());

		describeScript(nodeData->scripts[i].script);
	}

	return true;
}

bool Console::Cmd_LookAt(int argc, const char **argv) {

	if (argc != 3) {
		DebugPrintf("Usage :\n");
		DebugPrintf("lookAt pitch heading\n");
		return true;
	}

	_vm->_scene->lookAt(atof(argv[1]), atof(argv[2]));

	return false;
}

bool Console::Cmd_InitScript(int argc, const char **argv) {
	describeScript(_vm->_db->getNodeInitScript());

	return true;
}


bool Console::Cmd_Var(int argc, const char **argv) {

	if (argc != 2 && argc != 3) {
		DebugPrintf("Usage :\n");
		DebugPrintf("var variable : Display var value\n");
		DebugPrintf("var variable value : Change var value\n");
		return true;
	}

	uint16 var = atoi(argv[1]);
	uint32 value = _vm->_vars->get(var);

	if (argc == 3) {
		value = atoi(argv[2]);
		_vm->_vars->set(var, value);
	}

	DebugPrintf("var[%d] : %d\n", var, value);
	return true;
}

bool Console::Cmd_ListNodes(int argc, const char **argv) {

	uint16 roomID = 0;

	if (argc == 2) {
		roomID = atoi(argv[1]);
	}

	DebugPrintf("Nodes:\n");

	Common::Array<uint16> list = _vm->_db->listRoomNodes(roomID);
	for (uint i = 0; i < list.size(); i++) {
		DebugPrintf("%d\n", list[i]);
	}

	return true;
}

bool Console::Cmd_Run(int argc, const char **argv) {
	uint16 nodeId = _vm->_currentNode;
	uint16 roomId = 0;

	if (argc >= 2) {
		nodeId = atoi(argv[1]);
	}

	if (argc >= 3) {
		roomId = atoi(argv[2]);
	}

	_vm->runScriptsFromNode(nodeId, roomId);

	return false;
}

} /* namespace Myst3 */
