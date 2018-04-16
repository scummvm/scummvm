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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lilliput/lilliput.h"
#include "lilliput/sound.h"

#include "common/debug.h"

namespace Lilliput {

LilliputSound::LilliputSound(LilliputEngine *vm) : _vm(vm) {
}

LilliputSound::~LilliputSound() {
	free(_musicBuff);
}

void LilliputSound::loadMusic(Common::String filename) {
	debugC(1, kDebugSound, "loadMusic(%s)", filename.c_str());

	Common::File f;

	if (!f.open(filename))
		error("Missing music file %s", filename.c_str());

	byte *res = (byte *)malloc(sizeof(byte) * 50000);
	for (int i = 0; i < 50000; ++i)
		res[i] = f.readByte();

//	f.close();
	f.seek(0);
	int filenumb = f.readUint16LE();



	free(res);
}

// Used during initialization
void LilliputSound::init() {
	debugC(1, kDebugSound, "LilliputSound::init()");

	loadMusic("ROBIN.MUS");
}

void LilliputSound::refresh() {
	debugC(1, kDebugSound, "LilliputSound::refresh()");
}

void LilliputSound::play(int var1, Common::Point var2, Common::Point var3, Common::Point var4) {
	debugC(1, kDebugSound, "LilliputSound::play(%d, %d - %d, %d - %d, %d - %d)", var1, var2.x, var2.y, var3.x, var3.y, var4.x, var4.y);
}

void LilliputSound::stop(Common::Point pos) {
	debugC(1, kDebugSound, "LilliputSound::stop(%d - %d)", pos.x, pos.y);
}

void LilliputSound::toggleOnOff() {
	debugC(1, kDebugSound, "LilliputSound::toggleOnOff()");
}

void LilliputSound::update() {
	debugC(1, kDebugSound, "LilliputSound::update()");
}

void LilliputSound::remove() {
	debugC(1, kDebugSound, "Lilliput::remove()");
}

} // End of namespace
