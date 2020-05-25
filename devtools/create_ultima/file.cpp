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

#include "file.h"

#define MD5_COMPUTE_SIZE 1024

uint32 File::computeMD5() {
	uint32 total = 0;
	seek(0);
	for (int idx = 0; idx < MD5_COMPUTE_SIZE; ++idx)
		total += readByte();

	seek(0);
	return total;
}


void Surface::setPaletteEntry(byte index, byte r, byte g, byte b) {
	byte *pal = _palette + index * 3;
	pal[0] = r;
	pal[1] = g;
	pal[2] = b;
}

void Surface::saveToFile(const char *filename) {
	WriteFile f(filename);
	f.writeByte('B');
	f.writeByte('M');
	f.writeLong(0x436 + _w * _h + 2);	// File size
	f.writeWord(0);			// Custom 1
	f.writeWord(0);			// Custom 2
	f.writeLong(0x436);		// Pixels offset

	int pitch = _w;
	if (pitch % 4)
		pitch += 4 - (pitch % 4);

	f.writeLong(40);		// Info size
	f.writeLong(_w);		// Width
	f.writeLong(_h);		// Height
	f.writeWord(1);			// # Planes
	f.writeWord(8);			// Bits per pixel
	f.writeLong(0);			// Compression
	f.writeLong(pitch * _h);	// Image size
	f.writeLong(3790);		// Pixels per meter X
	f.writeLong(3800);		// Pixels per meter Y
	f.writeLong(0);			// color count
	f.writeLong(0);			// important colors

	// Palette
	byte *pal = _palette;
	for (int idx = 0; idx < 256; ++idx, pal += 3) {
		f.write(pal, 3);
		f.writeByte(0);
	}

	// Write out each line from the bottom up
	for (int y = _h - 1; y >= 0; --y) {
		byte *lineP = getBasePtr(0, y);
		f.write(lineP, _w);

		if (_w % 4)
			f.writeRepeating(0, 4 - (_w % 4));
	}
}
