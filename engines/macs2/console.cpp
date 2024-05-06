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
#include "macs2/view1.h"
#include "macs2/macs2.h"
#include "macs2/gameobjects.h"

namespace Macs2 {

Console::Console() : GUI::Debugger() {
	registerCmd("dumpblobs", WRAP_METHOD(Console, Cmd_dumpBlobs));
	registerCmd("autoclick", WRAP_METHOD(Console, Cmd_toggleAutoClick));
}

Console::~Console() {
}

bool Console::Cmd_toggleAutoClick(int argc, const char **argv) {
	View1 *currentView = (View1 *)g_engine->findView("View1");
	currentView->autoclickActive = !currentView->autoclickActive;
	debugPrintf("Auto clicking set to %s.\n",
				currentView->autoclickActive ? "on" : "off"
		);
	return true;
}

bool Console::Cmd_dumpBlobs(int argc, const char **argv) {
	Common::DumpFile df;
	// TODO: Read from args
	Common::String path = "C:\\Users\\Flori\\Downloads\\test.dmp";
	df.open(path);
	for (auto currentObject : GameObjects::instance().Objects) {
		df.writeString(Common::String::format("Object %.2xh\n", currentObject->Index));
		for (int i = 0; i < currentObject->Blobs.size(); i++) {
			auto currentBlob = currentObject->Blobs[i];
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

} // End of namespace Macs2
