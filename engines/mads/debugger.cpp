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

#include "common/file.h"
#include "mads/compression.h"
#include "mads/mads.h"
#include "mads/debugger.h"
#include "mads/nebular/menu_nebular.h"

namespace MADS {

Debugger::Debugger(MADSEngine *vm) : GUI::Debugger(), _vm(vm) {
	_showMousePos = false;

	registerCmd("continue",		WRAP_METHOD(Debugger, cmdExit));
	registerCmd("mouse", WRAP_METHOD(Debugger, Cmd_Mouse));
	registerCmd("scene", WRAP_METHOD(Debugger, Cmd_LoadScene));
	registerCmd("show_hotspots", WRAP_METHOD(Debugger, Cmd_ShowHotSpots));
	registerCmd("list_hotspots", WRAP_METHOD(Debugger, Cmd_ListHotSpots));
	registerCmd("play_sound", WRAP_METHOD(Debugger, Cmd_PlaySound));
	registerCmd("play_audio", WRAP_METHOD(Debugger, Cmd_PlayAudio));
	registerCmd("show_codes", WRAP_METHOD(Debugger, Cmd_ShowCodes));
	registerCmd("dump_file", WRAP_METHOD(Debugger, Cmd_DumpFile));
	registerCmd("show_quote", WRAP_METHOD(Debugger, Cmd_ShowQuote));
	registerCmd("show_vocab", WRAP_METHOD(Debugger, Cmd_ShowVocab));
	registerCmd("dump_vocab", WRAP_METHOD(Debugger, Cmd_DumpVocab));
	registerCmd("show_message", WRAP_METHOD(Debugger, Cmd_ShowMessage));
	registerCmd("show_item", WRAP_METHOD(Debugger, Cmd_ShowItem));
	registerCmd("dump_items", WRAP_METHOD(Debugger, Cmd_DumpItems));
	registerCmd("item", WRAP_METHOD(Debugger, Cmd_Item));
	registerCmd("play_anim", WRAP_METHOD(Debugger, Cmd_PlayAnim));
	registerCmd("play_text", WRAP_METHOD(Debugger, Cmd_PlayText));
	registerCmd("set_camera", WRAP_METHOD(Debugger, Cmd_SetCamera));
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
		debugPrintf("Usage: %s [ on | off ]\n", argv[0]);
	} else {
		_showMousePos = strcmp(argv[1], "on") == 0;
	}

	return true;
}

bool Debugger::Cmd_LoadScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Current scene is: %d\n", _vm->_game->_scene._currentSceneId);
		debugPrintf("Usage: %s <scene number>\n", argv[0]);
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

	debugPrintf("%d hotspots present\n", hotspots.size());

	for (uint index = 0; index < hotspots.size(); ++index) {
		debugPrintf("(%d): %p x1 = %d; y1 = %d; x2 = %d; y2 = %d\n",
			index, (void *)&hotspots[index],
			hotspots[index]._bounds.left, hotspots[index]._bounds.top,
			hotspots[index]._bounds.right, hotspots[index]._bounds.bottom);
	}

	return true;
}

bool Debugger::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <sound file>\n", argv[0]);
	} else {
		int commandId = strToInt(argv[1]);
		int param = (argc >= 3) ? strToInt(argv[2]) : 0;

		_vm->_sound->command(commandId, param);
	}

	return false;
}

bool Debugger::Cmd_PlayAudio(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <sound index> <sound group>\n", argv[0]);
		debugPrintf("If the sound group isn't defined, the default one will be used\n");
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
	scene._depthSurface.blitFrom(scene._backgroundSurface);
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
	if (argc < 2) {
		debugPrintf("Usage: %s <resource> <unpack>\n", argv[0]);
		debugPrintf("  resource: the resource name\n");
		debugPrintf("  unpack: optional, when specified, the FAB/MADSPACK compressed resource is unpacked\n");
	} else {
		Common::DumpFile outFile;
		Common::File inFile;

		if (!inFile.open(argv[1])) {
			debugPrintf("Specified resource does not exist\n");
		} else {
			outFile.open(argv[1]);
			bool unpack = ((argc >= 3) && !scumm_stricmp(argv[2], "unpack"));

			byte *data;
			int totalSize = 0;

			if (!unpack) {
				totalSize = inFile.size();
				data = new byte[totalSize];
				inFile.read(data, totalSize);
			} else {
				MadsPack dataPack(&inFile);
				int count = dataPack.getCount();
				for (int i = 0; i < count; i++) {
					totalSize += dataPack.getItem(i)._size;
				}
				data = new byte[totalSize];
				byte *ptr = data;

				for (int i = 0; i < count; i++) {
					Common::SeekableReadStream *readStream = dataPack.getItemStream(i);
					readStream->read(ptr, readStream->size());
					ptr += readStream->size();
				}
			}

			outFile.write(data, totalSize);
			outFile.flush();

			delete[] data;
			inFile.close();
			outFile.close();

			debugPrintf("File written successfully.\n");
		}
	}

	return true;
}

bool Debugger::Cmd_ShowQuote(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <quote number>\n", argv[0]);
	else
		debugPrintf("%s", _vm->_game->getQuote(strToInt(argv[1])).c_str());

	return true;
}

bool Debugger::Cmd_ShowVocab(int argc, const char **argv) {
	if (argc != 2) {
		for (uint32 i = 0; i < _vm->_game->_scene.getVocabStringsCount(); i++) {
			debugPrintf("%03d: '%s'\n", i, _vm->_game->_scene.getVocab(i + 1).c_str());
		}
	} else {
		int vocabId = strToInt(argv[1]);
		debugPrintf("%03d: '%s'\n", vocabId, _vm->_game->_scene.getVocab(vocabId + 1).c_str());
	}

	return true;
}

bool Debugger::Cmd_DumpVocab(int argc, const char **argv) {
	Common::DumpFile outFile;
	outFile.open("vocab.txt");

	for (uint32 i = 0; i < _vm->_game->_scene.getVocabStringsCount(); i++) {
		Common::String curId = Common::String::format("%x", i + 1);
		Common::String curVocab = _vm->_game->_scene.getVocab(i + 1);
		curVocab.toUppercase();

		for (uint j = 0; j < curVocab.size(); j++) {
			if (curVocab[j] == ' ' || curVocab[j] == '-')
				curVocab.setChar('_', j);
		}

		Common::String cur = "\tNOUN_" + curVocab + " = 0x" + curId + ",\n";

		outFile.writeString(cur.c_str());
	}

	outFile.flush();
	outFile.close();

	debugPrintf("Game vocab dumped\n");

	return true;
}

bool Debugger::Cmd_ShowMessage(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <message number>\n", argv[0]);
	} else {
		int messageId = strToInt(argv[1]);
		Common::StringArray msg = _vm->_game->getMessage(messageId);
		for (uint idx = 0; idx < msg.size(); ++idx) {
			Common::String srcLine = msg[idx];
			debugPrintf("%s\n", srcLine.c_str());
		}
	}

	return true;
}

bool Debugger::Cmd_ShowItem(int argc, const char **argv) {
	InventoryObjects &objects = _vm->_game->_objects;

	if (argc != 2) {
		for (uint32 i = 0; i < objects.size(); i++) {
			Common::String desc = _vm->_game->_scene.getVocab(objects[i]._descId);
			debugPrintf("%03d: '%s'\n", i, desc.c_str());
		}
	} else {
		int vocabId = strToInt(argv[1]);
		Common::String desc = _vm->_game->_scene.getVocab(objects[vocabId]._descId);
		debugPrintf("%03d: '%s'\n", vocabId, desc.c_str());
	}

	return true;
}

bool Debugger::Cmd_DumpItems(int argc, const char **argv) {
	InventoryObjects &objects = _vm->_game->_objects;

	Common::DumpFile outFile;
	outFile.open("items.txt");

	for (uint32 i = 0; i < objects.size(); i++) {
		Common::String curId = Common::String::format("%d", i);
		Common::String desc = _vm->_game->_scene.getVocab(objects[i]._descId);
		desc.toUppercase();

		for (uint j = 0; j < desc.size(); j++) {
			if (desc[j] == ' ' || desc[j] == '-')
				desc.setChar('_', j);
		}

		Common::String cur = "\tOBJ_" + desc + " = " + curId + ",\n";

		outFile.writeString(cur.c_str());
	}

	outFile.flush();
	outFile.close();

	debugPrintf("Game items dumped\n");

	return true;
}

bool Debugger::Cmd_Item(int argc, const char **argv) {
	InventoryObjects &objects = _vm->_game->_objects;

	if (argc != 2) {
		debugPrintf("Usage: %s <item number>\n", argv[0]);
		return true;
	} else {
		int objectId = strToInt(argv[1]);

		if (!objects.isInInventory(objectId))
			objects.addToInventory(objectId);

		debugPrintf("Item added.\n");
		return false;
	}
}

bool Debugger::Cmd_PlayAnim(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <anim name>\n", argv[0]);
		return true;
	} else {
		Common::String resName = argv[1];
		if (resName.hasPrefix("@"))
			resName.deleteChar(0);

		Common::File f;
		if (f.exists(resName) || f.exists(resName + ".res")) {
			AnimationView::execute(_vm, resName);
			return false;
		} else {
			debugPrintf("Could not find resource file\n");
			return true;
		}
	}
}

bool Debugger::Cmd_PlayText(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <text name>\n", argv[0]);
		return true;
	} else {
		Common::String resName = argv[1];
		if (resName.hasPrefix("@"))
			resName.deleteChar(0);

		Common::File f;
		if (f.exists(resName) || f.exists(resName + ".txr")) {
			TextView::execute(_vm, resName);
			return false;
		} else {
			debugPrintf("Could not find resource file\n");
			return true;
		}
	}
}

bool Debugger::Cmd_SetCamera(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s <x> <y>\n", argv[0]);
		return true;
	} else {
		int x = strToInt(argv[1]);
		int y = strToInt(argv[2]);
		_vm->_game->_scene.setCamera(Common::Point(x, y));
		_vm->_game->_scene.resetScene();
		_vm->_game->_scene.drawElements(kTransitionNone, false);
		return false;
	}
}
} // End of namespace MADS
