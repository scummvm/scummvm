/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
#include "engines/myst3/script.h"
#include "engines/myst3/state.h"

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
	DCmd_Register("dumpArchive",		WRAP_METHOD(Console, Cmd_DumpArchive));
}

Console::~Console() {
}

void Console::describeScript(const Common::Array<Opcode> &script) {
	for(uint j = 0; j < script.size(); j++) {
		DebugPrintf("%s", _vm->_scriptEngine->describeOpcode(script[j]).c_str());
	}
}

bool Console::Cmd_Infos(int argc, const char **argv) {

	uint16 nodeId = _vm->_state->getLocationNode();
	uint32 roomId = _vm->_state->getLocationRoom();

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

	for (uint i = 0; i < nodeData->scripts.size(); i++) {
		DebugPrintf("\ninit %d > %s (%s)\n", i,
				_vm->_state->describeCondition(nodeData->scripts[i].condition).c_str(),
				_vm->_state->evaluate(nodeData->scripts[i].condition) ? "true" : "false");

		describeScript(nodeData->scripts[i].script);
	}

	for (uint i = 0; i < nodeData->hotspots.size(); i++) {
		DebugPrintf("\nhotspot %d > %s (%s)\n", i,
				_vm->_state->describeCondition(nodeData->hotspots[i].condition).c_str(),
				_vm->_state->evaluate(nodeData->hotspots[i].condition) ? "true" : "false");

		for(uint j = 0; j < nodeData->hotspots[i].rects.size(); j++) {
			PolarRect &rect = nodeData->hotspots[i].rects[j];

			DebugPrintf("    rect > pitch: %d heading: %d width: %d height: %d\n",
					rect.centerPitch, rect.centerHeading, rect.width, rect.height);
		}

		describeScript(nodeData->hotspots[i].script);
	}

	for (uint i = 0; i < nodeData->soundScripts.size(); i++) {
		DebugPrintf("\nsound %d > %s (%s)\n", i,
				_vm->_state->describeCondition(nodeData->soundScripts[i].condition).c_str(),
				_vm->_state->evaluate(nodeData->soundScripts[i].condition) ? "true" : "false");

		describeScript(nodeData->soundScripts[i].script);
	}

	for (uint i = 0; i < nodeData->backgroundSoundScripts.size(); i++) {
		DebugPrintf("\nbackground sound %d > %s (%s)\n", i,
				_vm->_state->describeCondition(nodeData->backgroundSoundScripts[i].condition).c_str(),
				_vm->_state->evaluate(nodeData->backgroundSoundScripts[i].condition) ? "true" : "false");

		describeScript(nodeData->backgroundSoundScripts[i].script);
	}

	return true;
}

bool Console::Cmd_LookAt(int argc, const char **argv) {

	if (argc != 1 && argc != 3) {
		DebugPrintf("Usage :\n");
		DebugPrintf("lookAt pitch heading\n");
		return true;
	}

	float pitch = _vm->_state->getLookAtPitch();
	float heading = _vm->_state->getLookAtHeading();

	DebugPrintf("pitch: %d heading: %d\n", (int)pitch, (int)heading);

	if (argc >= 3){
		_vm->_state->lookAt(atof(argv[1]), atof(argv[2]));
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
	uint32 value = _vm->_state->getVar(var);

	if (argc == 3) {
		value = atoi(argv[2]);
		_vm->_state->setVar(var, value);
	}

	DebugPrintf("%s: %d\n", _vm->_state->describeVar(var).c_str(), value);

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
	uint16 nodeId = _vm->_state->getLocationNode();
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

	_vm->_state->setLocationNextRoom(roomID);
	_vm->_state->setLocationNextNode(nodeId);

	_vm->goToNode(0, 1);

	return false;
}

bool Console::Cmd_Extract(int argc, const char **argv) {
	if (argc != 5) {
		DebugPrintf("Extract a file from the game's archives\n");
		DebugPrintf("Usage :\n");
		DebugPrintf("extract [room] [node id] [face number] [object type]\n");
		return true;
	}

	// Room names are uppercase
	Common::String room = Common::String(argv[1]);
	room.toUppercase();

	uint16 id = atoi(argv[2]);
	uint16 face = atoi(argv[3]);
	DirectorySubEntry::ResourceType type = (DirectorySubEntry::ResourceType) atoi(argv[4]);

	const DirectorySubEntry *desc = _vm->getFileDescription(room.c_str(), id, face, type);

	if (!desc) {
		DebugPrintf("File with room %s, id %d, face %d and type %d does not exist\n", room.c_str(), id, face, type);
		return true;
	}

	Common::MemoryReadStream *s = desc->getData();
	Common::String filename = Common::String::format("node%s_%d_face%d.%d", room.c_str(), id, face, type);
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

bool Console::Cmd_DumpArchive(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Extract all the files from a game archive.\n");
		DebugPrintf("The destination folder, named 'dump', must exist.\n");
		DebugPrintf("Usage :\n");
		DebugPrintf("dumpArchive [file name]\n");
		return true;
	}

	// Is the archive multi-room
	Common::String temp = Common::String(argv[1]);
	temp.toUppercase();

	bool multiRoom = !temp.hasSuffix(".M3A");
	if (!multiRoom) {
		temp = Common::String(argv[1], 4);
		temp.toUppercase();
	}

	Archive archive;
	if (!archive.open(argv[1], multiRoom ? 0 : temp.c_str())) {
		DebugPrintf("Can't open archive with name '%s'\n", argv[1]);
		return true;
	}

	archive.dumpToFiles();
	archive.close();

	return true;
}

} /* namespace Myst3 */
