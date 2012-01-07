/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/console.h"
#include "engines/myst3/database.h"
#include "engines/myst3/inventory.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Console::Console(Myst3Engine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("infos",				WRAP_METHOD(Console, Cmd_Infos));
	DCmd_Register("lookAt",				WRAP_METHOD(Console, Cmd_LookAt));
	DCmd_Register("initScript",			WRAP_METHOD(Console, Cmd_InitScript));
	DCmd_Register("var",				WRAP_METHOD(Console, Cmd_Var));
	DCmd_Register("listNodes",			WRAP_METHOD(Console, Cmd_ListNodes));
	DCmd_Register("run",				WRAP_METHOD(Console, Cmd_Run));
	DCmd_Register("go",					WRAP_METHOD(Console, Cmd_Go));
	DCmd_Register("extract",			WRAP_METHOD(Console, Cmd_Extract));
	DCmd_Register("fillInventory",		WRAP_METHOD(Console, Cmd_FillInventory));
}

Console::~Console() {
}

void Console::describeScript(const Common::Array<Opcode> &script) {
	for(uint j = 0; j < script.size(); j++) {
		DebugPrintf("%s", _vm->_scriptEngine->describeOpcode(script[j]).c_str());
	}
}

bool Console::Cmd_Infos(int argc, const char **argv) {

	uint16 nodeId = _vm->_vars->getLocationNode();
	uint32 roomId = _vm->_vars->getLocationRoom();

	if (argc >= 2) {
		nodeId = atoi(argv[1]);
	}

	if (argc >= 3) {
		roomId = _vm->_db->getRoomId(argv[2]);

		if (roomId == 0) {
			DebugPrintf("Unknown room name %s\n", argv[2]);
			return true;
		}
	}

	NodePtr nodeData = _vm->_db->getNodeData(nodeId, roomId);

	if (!nodeData) {
		DebugPrintf("No node with id %d\n", nodeId);
		return true;
	}

	char roomName[8];
	_vm->_db->getRoomName(roomName, roomId);

	DebugPrintf("node: %s %d    ", roomName, nodeId);

	for (uint i = 0; i < nodeData->hotspots.size(); i++) {
		DebugPrintf("\nhotspot %d > %s (%s)\n", i,
				_vm->_vars->describeCondition(nodeData->hotspots[i].condition).c_str(),
				_vm->_vars->evaluate(nodeData->hotspots[i].condition) ? "true" : "false");

		for(uint j = 0; j < nodeData->hotspots[i].rects.size(); j++) {
			PolarRect &rect = nodeData->hotspots[i].rects[j];

			DebugPrintf("    rect > pitch: %d heading: %d height: %d width: %d\n",
					rect.centerPitch, rect.centerHeading, rect.width, rect.height);
		}

		describeScript(nodeData->hotspots[i].script);
	}

	for (uint i = 0; i < nodeData->scripts.size(); i++) {
		DebugPrintf("\nscript %d > %s (%s)\n", i,
				_vm->_vars->describeCondition(nodeData->scripts[i].condition).c_str(),
				_vm->_vars->evaluate(nodeData->scripts[i].condition) ? "true" : "false");

		describeScript(nodeData->scripts[i].script);
	}

	return true;
}

bool Console::Cmd_LookAt(int argc, const char **argv) {

	if (argc != 1 && argc != 3) {
		DebugPrintf("Usage :\n");
		DebugPrintf("lookAt pitch heading\n");
		return true;
	}

	Common::Point lookAt = _vm->_scene->getMousePos();
	DebugPrintf("pitch: %d heading: %d\n",  lookAt.y, lookAt.x);

	if (argc >= 3){
		_vm->_scene->lookAt(atof(argv[1]), atof(argv[2]));
		return false;
	}

	return true;
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

	DebugPrintf("%s: %d\n", _vm->_vars->describeVar(var).c_str(), value);

	return true;
}

bool Console::Cmd_ListNodes(int argc, const char **argv) {

	uint32 roomID = 0;

	if (argc == 2) {
		roomID = _vm->_db->getRoomId(argv[1]);

		if (roomID == 0) {
			DebugPrintf("Unknown room name %s\n", argv[1]);
			return true;
		}
	}

	DebugPrintf("Nodes:\n");

	Common::Array<uint16> list = _vm->_db->listRoomNodes(roomID);
	for (uint i = 0; i < list.size(); i++) {
		DebugPrintf("%d\n", list[i]);
	}

	return true;
}

bool Console::Cmd_Run(int argc, const char **argv) {
	uint16 nodeId = _vm->_vars->getLocationNode();
	uint32 roomId = 0;

	if (argc >= 2) {
		nodeId = atoi(argv[1]);
	}

	if (argc >= 3) {
		roomId = _vm->_db->getRoomId(argv[2]);

		if (roomId == 0) {
			DebugPrintf("Unknown room name %s\n", argv[2]);
			return true;
		}
	}

	_vm->runScriptsFromNode(nodeId, roomId);

	return false;
}


bool Console::Cmd_Go(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage :\n");
		DebugPrintf("go [room name] [node id] : Go to node\n");
		return true;
	}

	uint32 roomID = _vm->_db->getRoomId(argv[1]);
	uint16 nodeId = atoi(argv[2]);

	if (roomID == 0) {
		DebugPrintf("Unknown room name %s\n", argv[1]);
		return true;
	}

	_vm->goToNode(nodeId, roomID);

	return false;
}

bool Console::Cmd_Extract(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Extract a file from the game's archives\n");
		DebugPrintf("Usage :\n");
		DebugPrintf("extract [node id] [face number] [object type]\n");
		return true;
	}

	uint16 id = atoi(argv[1]);
	uint16 face = atoi(argv[2]);
	DirectorySubEntry::ResourceType type = (DirectorySubEntry::ResourceType) atoi(argv[3]);

	const DirectorySubEntry *desc = _vm->getFileDescription(id, face, type);

	if (!desc) {
		DebugPrintf("File with %d, face %d and type %d does not exist\n", id, face, type);
		return true;
	}

	Common::MemoryReadStream *s = desc->getData();
	Common::String filename = Common::String::format("node%d_face%d.%d", id, face, type);
	Common::DumpFile f;
	f.open(filename);

	uint8 *buf = new uint8[s->size()];

	s->read(buf, s->size());
	f.write(buf, s->size());

	delete[] buf;

	f.close();

	delete s;

	DebugPrintf("File '%s' successfully written\n", filename.c_str());

	return true;
}

bool Console::Cmd_FillInventory(int argc, const char **argv) {
	_vm->_inventory->addAll();
	return false;
}

} /* namespace Myst3 */
