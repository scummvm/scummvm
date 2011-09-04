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

namespace Myst3 {

Console::Console(Myst3Engine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("infos",				WRAP_METHOD(Console, Cmd_Infos));
	DCmd_Register("lookAt",				WRAP_METHOD(Console, Cmd_LookAt));
	DCmd_Register("initScript",			WRAP_METHOD(Console, Cmd_InitScript));
}

Console::~Console() {
}

Common::String Console::describeScript(const Common::Array<Opcode> &script) {
	Common::String d;

	for(uint j = 0; j < script.size(); j++) {
		const Opcode &opcode = script[j];

		d += Common::String::format("    op %s ( ",
				_vm->_scriptEngine->describeCommand(opcode.op).c_str());

		for(uint k = 0; k < opcode.args.size(); k++) {
			d += Common::String::format("%d ", opcode.args[k]);
		}

		d += ")\n";
	}

	return d;
}

bool Console::Cmd_Infos(int argc, const char **argv) {

	uint16 nodeId = _vm->_node.getId();
	NodePtr nodeData = _vm->_db->getNodeData(nodeId);

	char roomName[8];
	_vm->_db->getRoomName(roomName);

	Common::Point lookAt = _vm->_scene.getMousePos();

	DebugPrintf("current node: %s%d    ", roomName, nodeId);
	DebugPrintf("pitch: %d heading: %d",  lookAt.y, lookAt.x);

	for (uint i = 0; i < nodeData->hotspots.size(); i++) {
		DebugPrintf("\nhotspot %d > condition: %d\n",
				i, nodeData->hotspots[i].condition);

		for(uint j = 0; j < nodeData->hotspots[i].rects.size(); j++) {
			PolarRect &rect = nodeData->hotspots[i].rects[j];

			DebugPrintf("    rect > pitch: %d heading: %d height: %d width: %d\n",
					rect.centerPitch, rect.centerHeading, rect.width, rect.height);
		}

		DebugPrintf("%s", describeScript(nodeData->hotspots[i].script).c_str());
	}

	for (uint i = 0; i < nodeData->scripts.size(); i++) {
		DebugPrintf("\nscript %d > condition: %d\n",
				i, nodeData->scripts[i].condition);

		DebugPrintf("%s", describeScript(nodeData->scripts[i].script).c_str());
	}

	return true;
}

bool Console::Cmd_LookAt(int argc, const char **argv) {

	if (argc != 3) {
		DebugPrintf("Usage :\n");
		DebugPrintf("lookAt pitch heading\n");
		return true;
	}

	_vm->_scene.lookAt(atof(argv[1]), atof(argv[2]));

	return false;
}

bool Console::Cmd_InitScript(int argc, const char **argv) {
	DebugPrintf("%s", describeScript(_vm->_db->getNodeInitScript()).c_str());

	return true;
}

} /* namespace Myst3 */
