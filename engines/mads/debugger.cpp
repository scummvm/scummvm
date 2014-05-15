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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "mads/mads.h"
#include "mads/debugger.h"

namespace MADS {

Debugger::Debugger(MADSEngine *vm) : GUI::Debugger(), _vm(vm) {
	_showMousePos = false;

	DCmd_Register("continue",		WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("mouse", WRAP_METHOD(Debugger, Cmd_Mouse));
	DCmd_Register("scene", WRAP_METHOD(Debugger, Cmd_LoadScene));
	DCmd_Register("show_hotspots", WRAP_METHOD(Debugger, Cmd_ShowHotSpots));
	DCmd_Register("list_hotspots", WRAP_METHOD(Debugger, Cmd_ListHotSpots));
	DCmd_Register("play_sound", WRAP_METHOD(Debugger, Cmd_PlaySound));
	DCmd_Register("play_audio", WRAP_METHOD(Debugger, Cmd_PlayAudio));
	DCmd_Register("show_codes", WRAP_METHOD(Debugger, Cmd_ShowCodes));
	DCmd_Register("dump_file", WRAP_METHOD(Debugger, Cmd_DumpFile));
	DCmd_Register("show_quote", WRAP_METHOD(Debugger, Cmd_ShowQuote));
	DCmd_Register("item", WRAP_METHOD(Debugger, Cmd_Item));
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Debugger::Cmd_Mouse(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: %s [ on | off ]\n", argv[0]);
	} else {
		_showMousePos = strcmp(argv[1], "on") == 0;
	}

	return true;
}

bool Debugger::Cmd_LoadScene(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Current scene is: %d\n", _vm->_game->_scene._currentSceneId);
		DebugPrintf("Usage: %s <scene number>\n", argv[0]);
		return true;
	} else {
		_vm->_game->_scene._nextSceneId = strToInt(argv[1]);
		return false;
	}
}

bool Debugger::Cmd_ShowHotSpots(int argc, const char **argv) {
	Scene &scene = _vm->_game->_scene;

	// hotspots
	byte hotspotCol = _vm->getRandomNumber(255);
	for (uint i = 0; i < scene._hotspots.size(); i++) {
		scene._backgroundSurface.frameRect(scene._hotspots[i]._bounds, hotspotCol);
	}

	// Dynamic hotspots (red)
	hotspotCol = _vm->getRandomNumber(255);
	for (uint i = 0; i < scene._dynamicHotspots.size(); i++) {
		scene._backgroundSurface.frameRect(scene._dynamicHotspots[i]._bounds, hotspotCol);
	}

	scene._spriteSlots.fullRefresh();
	return false;
}

bool Debugger::Cmd_ListHotSpots(int argc, const char **argv) {
	Hotspots &hotspots = _vm->_game->_scene._hotspots;

	DebugPrintf("%d hotspots present\n", hotspots.size());

	for (uint index = 0; index < hotspots.size(); ++index) {
		DebugPrintf("(%d): %p x1 = %d; y1 = %d; x2 = %d; y2 = %d\n",
			index, (void *)&hotspots[index],
			hotspots[index]._bounds.left, hotspots[index]._bounds.top,
			hotspots[index]._bounds.right, hotspots[index]._bounds.bottom);
	}

	return true;
}

bool Debugger::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: %s <sound file>\n", argv[0]);
	} else {
		int commandId = strToInt(argv[1]);
		int param = (argc >= 3) ? strToInt(argv[2]) : 0;

		_vm->_sound->command(commandId, param);
	}

	return false;
}

bool Debugger::Cmd_PlayAudio(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: %s <sound index> <sound group>\n", argv[0]);
		DebugPrintf("If the sound group isn't defined, the default one will be used\n");
	} else {
		int index = strToInt(argv[1]);
		Common::String soundGroup = (argc >= 3) ? argv[2] : "";
		if (argc >= 3)
			_vm->_audio->setSoundGroup(argv[2]);
		else
			_vm->_audio->setDefaultSoundGroup();

		_vm->_audio->playSound(index);
	}

	return true;
}

bool Debugger::Cmd_ShowCodes(int argc, const char **argv) {
	Scene &scene = _vm->_game->_scene;

	// Copy the depth/walk surface to the background and flag for screen refresh
	scene._depthSurface.copyTo(&scene._backgroundSurface);
	scene._spriteSlots.fullRefresh();

	// Draw the locations of scene nodes onto the background
	int color = _vm->getRandomNumber(255);
	for (int i = 0; i < (int)scene._sceneInfo->_nodes.size(); ++i) {
		Common::Point &pt = scene._sceneInfo->_nodes[i]._walkPos;

		scene._backgroundSurface.hLine(pt.x - 2, pt.y, pt.x + 2, color);
		scene._backgroundSurface.vLine(pt.x, pt.y - 2, pt.y + 2, color);
	}

	return false;
}

bool Debugger::Cmd_DumpFile(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <resource>\n", argv[0]);
	} else {
		Common::DumpFile outFile;
		Common::File inFile;

		if (!inFile.open(argv[1])) {
			DebugPrintf("Specified resource does not exist\n");
		} else {
			outFile.open(argv[1]);
			byte *data = new byte[inFile.size()];

			inFile.read(data, inFile.size());
			outFile.write(data, inFile.size());
			outFile.flush();

			delete[] data;
			inFile.close();
			outFile.close();

			DebugPrintf("File written successfully.\n");
		}
	}

	return true;
}

bool Debugger::Cmd_ShowQuote(int argc, const char **argv) {
	if (argc != 2)
		DebugPrintf("Usage: %s <quote number>\n", argv[0]);
	else
		DebugPrintf("%s", _vm->_game->getQuote(strToInt(argv[1])).c_str());

	return true;
}

bool Debugger::Cmd_Item(int argc, const char **argv) {
	InventoryObjects &objects = _vm->_game->_objects;

	if (argc != 2) {
		DebugPrintf("Usage: %s <item number>\n", argv[0]);
		return true;
	} else {
		int objectId = strToInt(argv[1]);

		if (!objects.isInInventory(objectId))
			objects.addToInventory(objectId);

		DebugPrintf("Item added.\n");
		return false;
	}
}

} // End of namespace MADS
