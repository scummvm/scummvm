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

#include "macs2/console.h"
#include "macs2/adlib.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/view1.h"

#include <cstdlib>

namespace Macs2 {

static int parseHexArg(const char *arg) {
	return strtol(arg, nullptr, 16);
}

Console::Console() : GUI::Debugger() {
	registerCmd("dumpblobs", WRAP_METHOD(Console, Cmd_dumpBlobs));
	registerCmd("autoclick", WRAP_METHOD(Console, Cmd_toggleAutoClick));
	registerCmd("marker", WRAP_METHOD(Console, Cmd_marker));
	registerCmd("addItem", WRAP_METHOD(Console, Cmd_addItem));
	registerCmd("removeItem", WRAP_METHOD(Console, Cmd_removeItem));
	registerCmd("giveAll", WRAP_METHOD(Console, Cmd_giveAll));
	registerCmd("listScenes", WRAP_METHOD(Console, Cmd_listScenes));
	registerCmd("changeScene", WRAP_METHOD(Console, Cmd_changeScene));
	registerCmd("scene", WRAP_METHOD(Console, Cmd_scene));
	registerCmd("setOrientation", WRAP_METHOD(Console, Cmd_setOrientation));
	registerCmd("set", WRAP_METHOD(Console, Cmd_set));
	registerCmd("record", WRAP_METHOD(Console, Cmd_inputRecord));
	registerCmd("playback", WRAP_METHOD(Console, Cmd_inputPlayback));
	registerCmd("stoprecord", WRAP_METHOD(Console, Cmd_inputStop));
}

Console::~Console() {
}

bool Console::Cmd_toggleAutoClick(int argc, const char **argv) {
	View1 *currentView = (View1 *)g_engine->findView("View1");
	currentView->_autoclickActive = !currentView->_autoclickActive;
	debugPrintf("Auto clicking set to %s.\n",
				currentView->_autoclickActive ? "on" : "off");
	return true;
}

bool Console::Cmd_dumpBlobs(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: dumpblobs <path>\n");
		return true;
	}
	Common::DumpFile df;
	Common::String path = argv[1];
	df.open(Common::Path(path));
	for (auto currentObject : GameObjects::instance()._objects) {
		if (currentObject == nullptr)
			continue;
		df.writeString(Common::String::format("Object %.2xh\n", currentObject->_index));
		for (uint i = 0; i < currentObject->_blobs.size(); i++) {
			auto currentBlob = currentObject->_blobs[i];
			df.writeString(Common::String::format("Blob %.2xh\n", i));
			for (const uint8 value : currentBlob) {
				df.writeString(Common::String::format("%.2x", value));
			}
			df.writeString("\n");
		}
	}
	df.close();
	debugPrintf("Dumping blobs to %s.\n", path.c_str());
	return true;
}

bool Console::Cmd_marker(int argc, const char **argv) {
	// TODO: I tried implementing a CLS command but it's non-trivial, so using this marker instead
	Common::String marker;
	for (int i = 0; i < argc; i++) {
		marker += argv[i];
		marker += " ";
	}
	debug("***** %s", marker.c_str());
	return true;
}

bool Console::Cmd_addItem(int argc, const char **argv) {
	// TODO: Just realizing this - can we have multiple of an item in the inventory?
	// TODO: Check args count
	int index = parseHexArg(argv[1]);
	for (GameObject *obj : GameObjects::instance()._objects) {
		if (obj == nullptr)
			continue;
		if (obj->_index == index) {
			obj->_sceneIndex = 0x1;
		}
	}
	return true;
}

bool Console::Cmd_removeItem(int argc, const char **argv) {
	int index = parseHexArg(argv[1]);
	for (GameObject *obj : GameObjects::instance()._objects) {
		if (obj == nullptr)
			continue;
		if (obj->_index == index) {
			obj->_sceneIndex = 0x0;
		}
	}
	return true;
}

bool Console::Cmd_giveAll(int argc, const char **argv) {
	int count = 0;
	for (GameObject *obj : GameObjects::instance()._objects) {
		if (obj == nullptr || obj->_index <= 1)
			continue;
		if (!obj->_blobs.empty() && obj->_blobs.size() > 0x13 && !obj->_blobs[0x13].empty()) {
			obj->_sceneIndex = 1;
			count++;
		}
	}
	View1 *currentView = (View1 *)g_engine->findView("View1");
	currentView->setInventorySource(GameObjects::instance().getProtagonistObject());
	debugPrintf("Added %d items with inventory icons to protagonist inventory.\n", count);
	return true;
}

bool Console::Cmd_setOrientation(int argc, const char **argv) {
	int orientation = parseHexArg(argv[1]);
	int index = 1;
	if (argc > 2) {
		index = parseHexArg(argv[2]);
	}
	GameObjects::instance().getObjectByIndex(index)->_orientation = orientation;

	return true;
}

bool Console::Cmd_set(int argc, const char **argv) {
	int index = parseHexArg(argv[1]);
	int v1 = parseHexArg(argv[2]);
	int v2 = parseHexArg(argv[3]);
	g_engine->_scriptExecutor->setVariableValue(index, v1, v2);
	return true;
}

bool Console::Cmd_inputRecord(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: record <filename>\n");
		return true;
	}
	g_engine->startInputRecording(Common::Path(argv[1]));
	debugPrintf("Recording input to %s\n", argv[1]);
	return true;
}

bool Console::Cmd_inputPlayback(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: playback <filename>\n");
		return true;
	}
	g_engine->startInputPlayback(Common::Path(argv[1]));
	debugPrintf("Playing back input from %s\n", argv[1]);
	return true;
}

bool Console::Cmd_inputStop(int argc, const char **argv) {
	g_engine->stopInputRecording();
	debugPrintf("Input recording/playback stopped\n");
	return true;
}

bool Console::Cmd_listScenes(int argc, const char **argv) {
	debugPrintf("Current scene: %d (0x%x)\n", Scenes::instance()._currentSceneIndex, Scenes::instance()._currentSceneIndex);
	debugPrintf("Scenes with data:\n");
	for (int i = 1; i <= 512; i++) {
		uint32 offset = 0xC + 0x4 + (i - 1) * 0xC;
		g_engine->_fileStream->seek(offset + 8, SEEK_SET);
		uint32 sceneDataOffset = g_engine->_fileStream->readUint32LE();
		if (sceneDataOffset != 0) {
			debugPrintf("  Scene %3d (0x%03x) - data at 0x%06x\n", i, i, sceneDataOffset);
		}
	}
	return true;
}

bool Console::Cmd_changeScene(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: changeScene <sceneIndex>\n");
		debugPrintf("Current scene: %d (0x%x)\n", Scenes::instance()._currentSceneIndex, Scenes::instance()._currentSceneIndex);
		return true;
	}
	int sceneIndex = parseHexArg(argv[1]);
	debugPrintf("Changing to scene %d (0x%x)\n", sceneIndex, sceneIndex);
	g_engine->changeScene(sceneIndex);
	return false; // close debugger to let scene load
}

bool Console::Cmd_scene(int argc, const char **argv) {
	debugPrintf("Current scene: %d (0x%x)\n", Scenes::instance()._currentSceneIndex, Scenes::instance()._currentSceneIndex);
	debugPrintf("Previous scene: %d (0x%x)\n", Scenes::instance()._lastSceneIndex, Scenes::instance()._lastSceneIndex);
	return true;
}

} // End of namespace Macs2
