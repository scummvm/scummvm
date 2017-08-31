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
#include "supernova/state.h"
#include "supernova/console.h"

namespace Supernova {

Console::Console(SupernovaEngine *vm, GameManager *gm)
{
	registerCmd("render", WRAP_METHOD(Console, cmdRenderImage));
	registerCmd("play", WRAP_METHOD(Console, cmdPlaySound));
	registerCmd("music", WRAP_METHOD(Console, cmdMusic));
	registerCmd("list", WRAP_METHOD(Console, cmdList));
	registerCmd("inventory", WRAP_METHOD(Console, cmdInventory));
	registerCmd("giveall", WRAP_METHOD(Console, cmdGiveAll));

	_vm = vm;
	_gm = gm;
}

bool Console::cmdRenderImage(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: render [filenumber] [section]\n");
		return true;
	}

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

bool Console::cmdMusic(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: music [49/52]\n");
		return true;
	}

	_vm->playSoundMod(atoi(argv[1]));

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

bool Console::cmdGiveAll(int argc, const char **argv) {
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(0));
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(1));
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(2));
	_gm->takeObject(*_gm->_rooms[GENERATOR]->getObject(2)); // Commander Keycard
	_gm->takeObject(*_gm->_rooms[GENERATOR]->getObject(0)); // Power Cord with Plug
	_gm->takeObject(*_gm->_rooms[CABIN_L1]->getObject(5));  // Pen
	_gm->takeObject(*_gm->_rooms[CABIN_R3]->getObject(0));  // Chess Board
	_gm->takeObject(*_gm->_rooms[CABIN_R3]->getObject(9));  // Rope
	_gm->takeObject(*_gm->_rooms[AIRLOCK]->getObject(4));   // Helmet
	_gm->takeObject(*_gm->_rooms[AIRLOCK]->getObject(5));   // Space Suit
	_gm->takeObject(*_gm->_rooms[AIRLOCK]->getObject(6));   // Supply
	return true;
}

}
