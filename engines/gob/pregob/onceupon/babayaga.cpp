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

#include "common/textconsole.h"

#include "gob/gob.h"

#include "gob/pregob/onceupon/babayaga.h"

static const uint8 kCopyProtectionColors[7] = {
	14, 11, 13,  1,  7, 12,  2
};

static const uint8 kCopyProtectionShapes[7 * 20] = {
	0, 0, 1, 2, 1, 1, 2, 4, 3, 1, 4, 2, 4, 4, 2, 4, 1, 2, 3, 3,
	3, 1, 0, 3, 1, 3, 4, 2, 4, 4, 3, 2, 0, 2, 0, 1, 2, 0, 1, 4,
	1, 0, 2, 3, 4, 2, 3, 2, 2, 0, 0, 0, 4, 2, 3, 4, 4, 0, 4, 1,
	0, 2, 0, 4, 2, 4, 4, 2, 3, 0, 1, 1, 1, 1, 3, 0, 4, 2, 3, 4,
	3, 4, 3, 0, 1, 2, 0, 2, 2, 0, 2, 4, 0, 3, 4, 1, 1, 4, 1, 3,
	4, 2, 1, 1, 1, 1, 4, 3, 4, 2, 3, 0, 0, 3, 0, 2, 3, 0, 2, 4,
	4, 2, 4, 3, 0, 4, 0, 2, 3, 1, 4, 1, 3, 1, 0, 0, 2, 1, 3, 2
};

static const uint8 kCopyProtectionObfuscate[4] = {
	0, 1, 2, 3
};

namespace Gob {

namespace OnceUpon {

const OnceUpon::MenuButton BabaYaga::kAnimalsButtons = {
	true, 131, 127, 183, 164, 193, 0, 245, 37, 131, 127, 0
};

const OnceUpon::MenuButton BabaYaga::kAnimalButtons[] = {
	{false,  34,  84,  92, 127,  34,  84,  92, 127, 131, 25, 0},
	{false, 114,  65, 172, 111, 114,  65, 172, 111, 131, 25, 1},
	{false, 186,  72, 227,  96, 186,  72, 227,  96, 139, 25, 2},
	{false, 249,  87, 282, 112, 249,  87, 282, 112, 143, 25, 3},
	{false, 180,  97, 234, 138, 180,  97, 234, 138, 133, 25, 4},
	{false, 197, 145, 242, 173, 197, 145, 242, 173, 137, 25, 5},
	{false, 113, 156, 171, 176, 113, 156, 171, 176, 131, 25, 6},
	{false, 114, 127, 151, 150, 114, 127, 151, 150, 141, 25, 7},
	{false,  36, 136,  94, 176,  36, 136,  94, 176, 131, 25, 8},
	{false, 245, 123, 293, 155, 245, 123, 293, 155, 136, 25, 9}
};

const char *BabaYaga::kAnimalNames[] = {
	"vaut",
	"drag",
	"arai",
	"gren",
	"fauc",
	"abei",
	"serp",
	"tort",
	"sang",
	"rena"
};


BabaYaga::BabaYaga(GobEngine *vm) : OnceUpon(vm) {
	setAnimalsButton(&kAnimalsButtons);
}

BabaYaga::~BabaYaga() {
}

void BabaYaga::run() {
	init();

	// Copy protection
	bool correctCP = doCopyProtection(kCopyProtectionColors, kCopyProtectionShapes, kCopyProtectionObfuscate);
	if (_vm->shouldQuit() || !correctCP)
		return;

	// Show the intro
	showIntro();
	if (_vm->shouldQuit())
		return;

	mainLoop();

	if (!_vm->shouldQuit())
		warning("BabaYaga::run(): TODO: Show \"Bye Bye\"");
}

void BabaYaga::mainLoop() {
	clearScreen();

	MenuType mainMenu = kMenuTypeMainStart;

	while (!_vm->shouldQuit()) {
		MenuAction action = doMenu(mainMenu);
		if      (action == kMenuActionPlay)
			warning("BabaYaga::mainLoop(): TODO: Play");
		else if (action == kMenuActionRestart)
			warning("BabaYaga::mainLoop(): TODO: Restart");
		else if (action == kMenuActionAnimals)
			doAnimalNames(ARRAYSIZE(kAnimalButtons), kAnimalButtons, kAnimalNames);
		else if (action == kMenuActionQuit)
			break;
	}
}

} // End of namespace OnceUpon

} // End of namespace Gob
