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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "graphics/surface.h"
#include "nancy/console.h"
#include "nancy/nancy.h"
#include "nancy/resource.h"

namespace Nancy {

NancyConsole::NancyConsole(NancyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("res_hexdump", WRAP_METHOD(NancyConsole, Cmd_resHexDump));
	registerCmd("res_list", WRAP_METHOD(NancyConsole, Cmd_resList));
	registerCmd("res_info", WRAP_METHOD(NancyConsole, Cmd_resInfo));
	registerCmd("res_show_image", WRAP_METHOD(NancyConsole, Cmd_resShowImage));
}

NancyConsole::~NancyConsole() {
}

bool NancyConsole::Cmd_resHexDump(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Dumps the specified resource to standard output\n");
		debugPrintf("Usage: %s <resource name>\n", argv[0]);
		return true;
	}

	uint size;
	byte *buf = _vm->_res->loadResource(argv[1], size);
	if (!buf) {
		debugPrintf("Failed to load resource '%s'", argv[1]);
		return true;
	}

	Common::hexdump(buf, size);
	return true;
}

bool NancyConsole::Cmd_resList(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("List resources of a certain type\n");
		debugPrintf("Types - 0: all, 2: image, 3: script\n");
		debugPrintf("Usage: %s <resource type>\n", argv[0]);
		return true;
	}

	Common::Array<Common::String> list;
	_vm->_res->listResources(list, atoi(argv[1]));
	for (uint i = 0; i < list.size(); i++) {
		debugPrintf("%-10s", list[i].c_str());
		if ((i % 8) == 7 && i + 1 != list.size())
			debugPrintf("\n");
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_resInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Prints information about a resource\n");
		debugPrintf("Usage: %s <resource name>\n", argv[0]);
		return true;
	}

	debugPrintf("%s", _vm->_res->getResourceDesc(argv[1]).c_str());
	return true;
}

bool NancyConsole::Cmd_resShowImage(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Draws an image on the screen\n");
		debugPrintf("Usage: %s <resource name>\n", argv[0]);
		return true;
	}

	Graphics::Surface surf;
	if (_vm->_res->loadImage(argv[1], surf)) {
		_vm->_system->fillScreen(0);
		_vm->_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, surf.w > 640 ? 640 : surf.w, surf.h > 480 ? 480 : surf.h);
		surf.free();
	} else
		debugPrintf("Failed to load image\n");
	return true;
}

} // End of namespace Nancy
