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

#include "common/scummsys.h"
#include "glk/scott/disk_image.h"

namespace Glk {
namespace Scott {

ImageFile *diOpen(DiskImage *di, byte *rawname, byte type, const char *mode) {
	return nullptr;
}

int diRead(ImageFile *imgfile, byte *buffer, int len) {
	return 0;
}

int diRawnameFromName(byte *rawname, const char *name) {
	return 0;
}

DiskImage *diCreateFromData(uint8_t *data, int length) {
	return nullptr;
}

} // End of namespace Scott
} // End of namespace Glk
