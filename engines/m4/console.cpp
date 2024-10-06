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

#include "m4/console.h"
#include "m4/m4.h"
#include "m4/vars.h"
#include "m4/graphics/graphics.h"

namespace M4 {

Console::Console() : GUI::Debugger() {
	registerCmd("teleport",  WRAP_METHOD(Console, cmdTeleport));
	registerCmd("item",      WRAP_METHOD(Console, cmdItem));
	registerCmd("hyperwalk", WRAP_METHOD(Console, cmdHyperwalk));
	registerCmd("digi",      WRAP_METHOD(Console, cmdDigi));
	registerCmd("trigger",   WRAP_METHOD(Console, cmdTrigger));
	registerCmd("cels",      WRAP_METHOD(Console, cmdCels));
	registerCmd("cel",       WRAP_METHOD(Console, cmdCel));
	registerCmd("interface", WRAP_METHOD(Console, cmdInterface));
}

bool Console::cmdTeleport(int argc, const char **argv) {
	if (argc == 2) {
		_G(game).setRoom(atol(argv[1]));
		_G(kernel).teleported_in = true;
		return false;
	} else {
		debugPrintf("Currently in room %d\n", _G(game).room_id);
		return true;
	}
}

bool Console::cmdItem(int argc, const char **argv) {
	if (argc == 2) {
		inv_give_to_player(argv[1]);
		return false;
	} else {
		debugPrintf("item <item name>\n");
		return true;
	}
}

bool Console::cmdHyperwalk(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("hyperwalk [on | off]\n");
	} else {
		_G(hyperwalk) = !strcmp(argv[1], "on");
		debugPrintf("hyperwalk is now %s\n", _G(hyperwalk) ? "on" : "off");
	}

	return true;
}

bool Console::cmdDigi(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("digi <sound name>\n");
		return true;
	} else {
		digi_play(argv[1], 1);
		return false;
	}

	return true;
}

bool Console::cmdTrigger(int argc, const char **argv) {
	if (argc == 2) {
		kernel_trigger_dispatch_now(atol(argv[1]));
		return false;
	} else {
		debugPrintf("trigger <number>\n");
		return true;
	}
}

bool Console::cmdCels(int argc, const char **argv) {
	for (int i = 0; i < 256; ++i) {
		if (_GWS(globalCELSnames)[i]) {
			uint32 *celsPtr = (uint32 *)((intptr)*_GWS(globalCELSHandles)[i] +
				_GWS(globalCELSoffsets)[i]);
			debugPrintf("#%d - %s - count=%d, max w=%d, max h=%d\n",
				i, _GWS(globalCELSnames)[i], celsPtr[CELS_COUNT],
				celsPtr[CELS_SS_MAX_W], celsPtr[CELS_SS_MAX_H]);
		}
	}

	return true;
}

bool Console::cmdCel(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("cel <cel number>\n");
	} else {
		int num = atol(argv[1]);

		if (!_GWS(globalCELSHandles)[num]) {
			debugPrintf("cel index not in use\n");
		} else {
			uint32 *data = (uint32 *)((intptr)*_GWS(globalCELSHandles)[num] +
				_GWS(globalCELSoffsets)[num]);

			for (int i = 0; i < 15; i += 5) {
				Common::String line = Common::String::format(
					"%.8x %.8x %.8x %.8x %.8x",
					data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4]
				);
				debugPrintf("%s\n", line.c_str());
			}
		}
	}

	return true;
}

bool Console::cmdInterface(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("interface ['show', 'hide']\n");
		return true;
	} else {
		Common::String param(argv[1]);

		if (param == "hide" || param == "off" || param == "false")
			interface_hide();
		else
			interface_show();

		return false;
	}
}

} // End of namespace M4
