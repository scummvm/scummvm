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

#include "chewy/effect.h"
#include "chewy/globals.h"
#include "chewy/mcga_graphics.h"

namespace Chewy {

Effect::Effect() {
}
Effect::~Effect() {
}

void Effect::rnd_blende(byte *rnd_speicher, byte *sram_speicher, byte *screen, byte *palette, int16 col, int16 skip_line) {
	byte *sp = (byte *)MALLOC(8 * 8 + 4);
	int16 *rnd_zeiger = (int16 *)rnd_speicher;
	if (col < 256) {
		for (int16 i = 0; i < 1002; i++) {
			int16 x;
			int16 y = (rnd_zeiger[i] / 40) * 8;
			if (rnd_zeiger[i] > 39)
				x = (rnd_zeiger[i] - (40 * (y / 8))) * 8;
			else
				x = rnd_zeiger[i] * 8;
			_G(out)->boxFill(x, y, x + 8, y + 8, col);
		}
		_G(out)->setPalette(palette);
	}
	for (int16 i = 0; i < 1000; i++) {
		int16 x;
		int16 y = (rnd_zeiger[i] / 40) * 8;
		if (rnd_zeiger[i] > 39)
			x = (rnd_zeiger[i] - (40 * (y / 8))) * 8;
		else
			x = rnd_zeiger[i] * 8;
		_G(out)->setPointer(sram_speicher);
		_G(out)->spriteSave(sp, x, y, 8, 8);
		_G(out)->setPointer(screen);
		_G(out)->spriteSet(sp, x, y, 0);
	}
	free(sp);
}

void Effect::blende1(byte *memPtr, byte *screen, byte *palette, int16 frames, uint8 mode, int16 color) {
	byte *sp = (byte *)MALLOC(8 * 8 + 4);
	if (color < 256) {
		for (int16 i = 0; i < 13; i++) {
			int16 x, y, x1;
			for (x = i; x < 39 - i; x++)
				_G(out)->boxFill(x * 8, i * 8, x * 8 + 8, i * 8 + 8, color);
			for (y = i; y < 24 - i; y++)
				_G(out)->boxFill(x * 8, y * 8, x * 8 + 8, y * 8 + 8, color);
			for (x1 = 39 - i; x1 > i; x1--)
				_G(out)->boxFill(x1 * 8, y * 8, x1 * 8 + 8, y * 8 + 8, color);
			for (int16 y1 = 24 - i; y1 >= i; y1--)
				_G(out)->boxFill(x1 * 8, y1 * 8, x1 * 8 + 8, y1 * 8 + 8, color);
		}
		_G(out)->setPalette(palette);
	}
	switch (mode) {
	case 0:
		for (int16 i = 13; i >= 0; i--) {
			int16 x, y, x1;
			for (x = i; x < 39 - i; x++) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x * 8, i * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x * 8, i * 8, 0);
			}
			for (y = i; y < 24 - i; y++) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x * 8, y * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x * 8, y * 8, 0);
			}
			for (x1 = 39 - i; x1 > i; x1--) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x1 * 8, y * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x1 * 8, y * 8, 0);
			}
			for (int16 y1 = 24 - i; y1 > i; y1--) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x1 * 8, y1 * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x1 * 8, y1 * 8, 0);
			}
		}
		break;

	case 1:
		for (int16 i = 0; i < 13; i++) {
			int16 x, y, x1;
			for (x = i; x < 39 - i; x++) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x * 8, i * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x * 8, i * 8, 0);
			}
			for (y = i; y < 24 - i; y++) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x * 8, y * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x * 8, y * 8, 0);
			}
			for (x1 = 39 - i; x1 > i; x1--) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x1 * 8, y * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x1 * 8, y * 8, 0);
			}
			for (int16 y1 = 24 - i; y1 > i; y1--) {
				_G(out)->setPointer(memPtr);
				_G(out)->spriteSave(sp, x1 * 8, y1 * 8, 8, 8);
				_G(out)->setPointer(screen);
				_G(out)->spriteSet(sp, x1 * 8, y1 * 8, 0);
			}
		}
		break;

	default:
		break;
	}
	free(sp);
}

void Effect::border(byte *workpage_, int16 lines, uint8 mode, int16 color) {
	if (mode) {
		for (int i = 0, x = 0; i < 20; ++i, x += 8) {
			_G(out)->setPointer(workpage_ + 4);
			_G(out)->boxFill(152 - x, 0, 152 - x + 8, 200, color);
			_G(out)->boxFill(x + 160, 0, x + 168, 200, color);
			_G(out)->copyToScreen();
		}
	} else {
		for (int i = 0, x = 0; i < 20; ++i, x += 8) {
			_G(out)->setPointer(workpage_ + 4);
			_G(out)->boxFill(x, 0, x + 8, 200, color);
			_G(out)->boxFill(312 - x, 0, 31 - x + 8, 200, color);
			_G(out)->copyToScreen();
		}
	}
}

} // namespace Chewy
