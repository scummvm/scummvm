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
#include "nancy/video.h"

namespace Nancy {

NancyConsole::NancyConsole(NancyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("res_load_cal", WRAP_METHOD(NancyConsole, Cmd_resLoadCal));
	registerCmd("res_hexdump", WRAP_METHOD(NancyConsole, Cmd_resHexDump));
	registerCmd("res_diskdump", WRAP_METHOD(NancyConsole, Cmd_resDiskDump));
	registerCmd("res_list", WRAP_METHOD(NancyConsole, Cmd_resList));
	registerCmd("res_info", WRAP_METHOD(NancyConsole, Cmd_resInfo));
	registerCmd("res_show_image", WRAP_METHOD(NancyConsole, Cmd_resShowImage));
	registerCmd("play_video", WRAP_METHOD(NancyConsole, Cmd_playVideo));
}

NancyConsole::~NancyConsole() {
}

void NancyConsole::postEnter() {
	if (!_videoFile.empty()) {
		Video::VideoDecoder *dec = new AVFDecoder;
	
		if (dec->loadFile(_videoFile)) {
			dec->start();
			while (!dec->endOfVideo()) {
				const Graphics::Surface *frame = dec->decodeNextFrame();
				_vm->_system->fillScreen(0);
				_vm->_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
				_vm->_system->updateScreen();
				_vm->_system->delayMillis(60);
			}
		} else {
			debugPrintf("Failed to load '%s'\n", _videoFile.c_str());
		}
		_videoFile.clear();
	}
}

bool NancyConsole::Cmd_resHexDump(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Dumps the specified resource to standard output\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	uint size;
	byte *buf = _vm->_res->loadCif((argc == 2 ? "ciftree" : argv[2]), argv[1], size);
	if (!buf) {
		debugPrintf("Failed to load resource '%s'\n", argv[1]);
		return true;
	}

	Common::hexdump(buf, size);
	delete[] buf;
	return true;
}

bool NancyConsole::Cmd_resDiskDump(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Dumps the specified resource to disk\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	uint size;
	byte *buf = _vm->_res->loadCif((argc == 2 ? "ciftree" : argv[2]), argv[1], size);
	if (!buf) {
		debugPrintf("Failed to load resource '%s'\n", argv[1]);
		return true;
	}

	Common::String filename = argv[1];
	filename += ".raw";
	Common::DumpFile dump;
	if (!dump.open(filename)) {
		debugPrintf("Failed to open dump file '%s'\n", filename.c_str());
		delete[] buf;
		return true;
	}

	if (dump.write(buf, size) < size)
		debugPrintf("Failed to write dump file '%s'\n", filename.c_str());

	dump.close();
	delete[] buf;
	return true;
}

bool NancyConsole::Cmd_resList(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("List resources of a certain type\n");
		debugPrintf("Types - 0: all, 2: image, 3: script\n");
		debugPrintf("Usage: %s type [cal]\n", argv[0]);
		return true;
	}

	Common::Array<Common::String> list;
	_vm->_res->list((argc == 2 ? "ciftree" : argv[2]), list, atoi(argv[1]));
	for (uint i = 0; i < list.size(); i++) {
		debugPrintf("%-38s", list[i].c_str());
		if ((i % 2) == 1 && i + 1 != list.size())
			debugPrintf("\n");
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_resInfo(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Prints information about a resource\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	debugPrintf("%s", _vm->_res->getCifDescription((argc == 2 ? "ciftree" : argv[2]), argv[1]).c_str());
	return true;
}

bool NancyConsole::Cmd_resShowImage(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Draws an image on the screen\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	Graphics::Surface surf;
	if (_vm->_res->loadImage((argc == 2 ? "ciftree" : argv[2]), argv[1], surf)) {
		_vm->_system->fillScreen(0);
		_vm->_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, surf.w > 640 ? 640 : surf.w, surf.h > 480 ? 480 : surf.h);
		surf.free();
		return cmdExit(0, 0);
	} else {
		debugPrintf("Failed to load image\n");
		return true;
	}
}

bool NancyConsole::Cmd_resLoadCal(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Loads a .cal file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	if (!_vm->_res->loadCifTree(argv[1], "cal"))
		debugPrintf("Failed to load '%s.cal'\n", argv[1]);
	return true;
}

bool NancyConsole::Cmd_playVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Plays a video\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	_videoFile = argv[1];
	_videoFile += ".avf";
	return cmdExit(0, 0);
}

} // End of namespace Nancy
