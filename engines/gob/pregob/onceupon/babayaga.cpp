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

BabaYaga::BabaYaga(GobEngine *vm) : OnceUpon(vm) {
}

BabaYaga::~BabaYaga() {
}

void BabaYaga::run() {
	init();

	bool correctCP = doCopyProtection(kCopyProtectionColors, kCopyProtectionShapes, kCopyProtectionObfuscate);
	if (_vm->shouldQuit() || !correctCP)
		return;

	showWait();
	if (_vm->shouldQuit())
		return;

	showQuote();
	if (_vm->shouldQuit())
		return;

	showTitle();
}

} // End of namespace OnceUpon

} // End of namespace Gob
