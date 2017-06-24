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

#include "gui/debugger.h"

#include "supernova/supernova.h"
#include "supernova/console.h"

namespace Supernova {

Console::Console(SupernovaEngine *vm, GameManager *gm)
{
	registerCmd("render", WRAP_METHOD(Console, cmdRenderImage));
	registerCmd("play", WRAP_METHOD(Console, cmdPlaySound));
	registerCmd("list", WRAP_METHOD(Console, cmdList));
	registerCmd("inventory", WRAP_METHOD(Console, cmdInventory));

	_vm = vm;
	_gm = gm;
}

bool Console::cmdRenderImage(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: render [filenumber] [section]\n");
		return true;
	}

	int filenumber = atoi(argv[1]);
	int section = atoi(argv[2]);

	_vm->renderImage(atoi(argv[1]), atoi(argv[2]));

	return true;
}

bool Console::cmdPlaySound(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: play [0-%d]\n", kAudioNumSamples - 1);
		return true;
	}

	int sample = Common::String(argv[1]).asUint64();
	_vm->playSound(static_cast<AudioIndex>(sample));

	return true;
}

bool Console::cmdList(int argc, const char **argv) {
	// Objects in room and sections

	return true;
}

bool Console::cmdInventory(int argc, const char **argv) {
	if (argc != 2 || argc != 3) {
		debugPrintf("Usage: inventory [list][add/remove [object]]");
		return true;
	}

	// TODO

	return true;
}

}
