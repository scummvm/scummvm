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
 * $URL$
 * $Id$
 *
 */

#include "made/made.h"
#include "made/screen.h"
#include "made/screenfx.h"

namespace Made {

const byte ScreenEffects::vfxOffsTable[64] = {
	5, 2, 6, 1, 4, 7, 3, 0,
	7, 4, 0, 3, 6, 1, 5, 2,
	2, 5, 1, 6, 3, 0, 4, 7,
	0, 3, 7, 4, 1, 6, 2, 5,
	4, 0, 2, 5, 7, 3, 1, 6,
	1, 6, 4, 0, 2, 5, 7, 3,
	6, 1, 3, 7, 5, 2, 0, 4,
	3, 7, 5, 2, 0, 4, 6, 1
};

const byte ScreenEffects::vfxOffsIndexTable[8] = {
	6, 7, 2, 3, 4, 5, 0, 1
};


ScreenEffects::ScreenEffects(Screen *screen) : _screen(screen) {
	vfxOffsTablePtr = &vfxOffsTable[6 * 8];
	vfxX1 = 0;
	vfxY1 = 0;
	vfxWidth = 0;
	vfxHeight = 0;
}

void ScreenEffects::run(int16 effectNum, Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {

	// TODO: Put effect functions into an array
	// TODO: Implement more visual effects

	switch (effectNum) {

	case 0:		// No effect
		vfx00(surface, palette, newPalette, colorCount);
		break;

	case 1:
		vfx01(surface, palette, newPalette, colorCount);
		break;

	case 2:
		vfx02(surface, palette, newPalette, colorCount);
		break;

	case 3:
		vfx03(surface, palette, newPalette, colorCount);
		break;

	case 4:
		vfx04(surface, palette, newPalette, colorCount);
		break;

	case 5:
		vfx05(surface, palette, newPalette, colorCount);
		break;

	case 6:
		vfx06(surface, palette, newPalette, colorCount);
		break;

	case 7:
		vfx07(surface, palette, newPalette, colorCount);
		break;

	case 9:		// "Checkerboard" effect
		vfx09(surface, palette, newPalette, colorCount);
		break;

	case 10:	// "Screen wipe in", left to right
		vfx10(surface, palette, newPalette, colorCount);
		break;

	case 11:	// "Screen wipe in", right to left
		vfx11(surface, palette, newPalette, colorCount);
		break;

	case 12:	// "Screen wipe in", top to bottom
		vfx12(surface, palette, newPalette, colorCount);
		break;

	case 13:	// "Screen wipe in", bottom to top
		vfx13(surface, palette, newPalette, colorCount);
		break;

	case 14:	// "Screen open" effect
		vfx14(surface, palette, newPalette, colorCount);
		break;

	case 15:
		vfx15(surface, palette, newPalette, colorCount);
		break;

	case 16:
		vfx16(surface, palette, newPalette, colorCount);
		break;

	case 17:	// Palette fadeout/fadein
		vfx17(surface, palette, newPalette, colorCount);
		break;

	default:
		vfx00(surface, palette, newPalette, colorCount);
		warning("Unimplemented visual effect %d", effectNum);

	}

}

void ScreenEffects::flash(int flashCount, byte *palette, int colorCount) {
	int palSize = colorCount * 3;
	if (flashCount < 1)
		flashCount = 1;
	for (int i = 0; i < palSize; i++)
		_fxPalette[i] = CLIP<byte>(255 - palette[i], 0, 255);
	while (flashCount--) {
		_screen->setRGBPalette(_fxPalette, 0, colorCount);
		_screen->updateScreenAndWait(20);
		_screen->setRGBPalette(palette, 0, colorCount);
  		_screen->updateScreenAndWait(20);
	}
}

void ScreenEffects::setPalette(byte *palette) {
	if (!_screen->isPaletteLocked()) {
		_screen->setRGBPalette(palette, 0, 256);
	}
}

void ScreenEffects::setBlendedPalette(byte *palette, byte *newPalette, int colorCount, int16 value, int16 maxValue) {
	if (!_screen->isPaletteLocked()) {
		int32 mulValue = (value * 64) / maxValue;
		for (int i = 0; i < colorCount * 3; i++)
			_fxPalette[i] = CLIP<int32>(newPalette[i] - (newPalette[i] - palette[i]) * mulValue / 64, 0, 255);
		_screen->setRGBPalette(_fxPalette, 0, 256);
	}
}

void ScreenEffects::copyFxRect(Graphics::Surface *surface, int16 x1, int16 y1, int16 x2, int16 y2) {

	// TODO: Clean up

	byte *src, *dst;

	x1 = CLIP<int16>(x1, 0, 320);
	y1 = CLIP<int16>(y1, 0, 200);
	x2 = CLIP<int16>(x2, 0, 320);
	y2 = CLIP<int16>(y2, 0, 200);

	x2 -= x1;
	y2 -= y1;
	vfxX1 = x1 & 0x0E;
	x1 += 16;
	x1 = x1 & 0xFFF0;
	x2 += vfxX1;
	x2 -= 15;
	if (x2 < 0)
		x2 = 0;
	vfxWidth = x2 & 0x0E;
	x2 = x2 & 0xFFF0;

	vfxY1 = y1 & 7;

	byte *source = (byte*)surface->getBasePtr(x1, y1);

	Graphics::Surface *vgaScreen = _screen->lockScreen();
	byte *dest = (byte*)vgaScreen->getBasePtr(x1, y1);

	int16 addX = x2 / 16;

	while (y2-- > 0) {

		int16 addVal = vfxOffsTablePtr[vfxY1] * 2;
		int16 w = 0;
		vfxY1 = (vfxY1 + 1) & 7;

		src = source + addVal;
		dst = dest + addVal;

		if (addVal < vfxX1) {
			if (addVal < vfxWidth)
				w = 1;
			else
				w = 0;
		} else {
			src -= 16;
			dst -= 16;
			if (addVal < vfxWidth)
				w = 2;
			else
				w = 1;
		}

		w += addX;

		while (w-- > 0) {
			*dst++ = *src++;
			*dst++ = *src++;
			src += 14;
			dst += 14;
		}

		source += 320;
		dest += 320;

	}

	vfxHeight = (vfxHeight + 1) & 7;
	vfxOffsTablePtr = &vfxOffsTable[vfxOffsIndexTable[vfxHeight] * 8];

	_screen->unlockScreen();

}

void ScreenEffects::copyRect(Graphics::Surface *surface, int16 x1, int16 y1, int16 x2, int16 y2) {
	Graphics::Surface *vgaScreen = _screen->lockScreen();
	byte *source = (byte*)surface->getBasePtr(x1, y1);
	byte *dest = (byte*)vgaScreen->getBasePtr(x1, y1);
	for (int y = 0; y < y2 - y1; y++) {
		memcpy(dest, source, x2 - x1);
		dest += 320;
		source += 320;
	}
	_screen->unlockScreen();
}

// No effect
void ScreenEffects::vfx00(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	setPalette(palette);
	_screen->showWorkScreen();
	// Workaround for The Manhole, else animations will be shown too fast
	_screen->updateScreenAndWait(100);
}

void ScreenEffects::vfx01(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int x = 0; x < 320; x += 8) {
		copyRect(surface, x, 0, x + 8, 200);
		setBlendedPalette(palette, newPalette, colorCount, x, 312);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx02(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int x = 312; x >= 0; x -= 8) {
		copyRect(surface, x, 0, x + 8, 200);
		setBlendedPalette(palette, newPalette, colorCount, 312 - x, 312);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx03(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int y = 0; y < 200; y += 10) {
		copyRect(surface, 0, y, 320, y + 10);
		setBlendedPalette(palette, newPalette, colorCount, y, 190);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx04(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int y = 190; y >= 0; y -= 10) {
		copyRect(surface, 0, y, 320, y + 10);
		setBlendedPalette(palette, newPalette, colorCount, 190 - y, 190);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx05(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int y = 0; y < 100; y += 10) {
		copyRect(surface, 0, y + 100, 320, y + 110);
		copyRect(surface, 0, 90 - y, 320, 100 - y);
		setBlendedPalette(palette, newPalette, colorCount, y, 90);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx06(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int x = 0; x < 160; x += 8) {
		copyRect(surface, x + 160, 0, x + 168, 200);
		copyRect(surface, 152 - x, 0, 160 - x, 200);
		setBlendedPalette(palette, newPalette, colorCount, x, 152);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx07(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int x = 152; x >= 0; x -= 8) {
		copyRect(surface, x + 160, 0, x + 168, 200);
		copyRect(surface, 152 - x, 0, 160 - x, 200);
		setBlendedPalette(palette, newPalette, colorCount, 152 - x, 152);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

// "Checkerboard" effect
void ScreenEffects::vfx09(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int i = 0; i < 8; i++) {
		copyFxRect(surface, 0, 0, 320, 200);
		for (int j = 0; j < 4; j++) {
			setBlendedPalette(palette, newPalette, colorCount, i * 4 + j, 36/*FIX?*/);
			_screen->updateScreenAndWait(25);
		}
	}
	setPalette(palette);
}

// "Screen wipe in", left to right
void ScreenEffects::vfx10(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int x = -56; x < 312; x += 8) {
		copyFxRect(surface, x, 0, x + 64, 200);
		setBlendedPalette(palette, newPalette, colorCount, x + 56, 368);
		_screen->updateScreenAndWait(25);
	}
	setPalette(palette);
}

// "Screen wipe in", right to left
void ScreenEffects::vfx11(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int x = 312; x > -56; x -= 8) {
		copyFxRect(surface, x, 0, x + 64, 200);
		setBlendedPalette(palette, newPalette, colorCount, x + 56, 368);
		_screen->updateScreenAndWait(25);
	}
	setPalette(palette);
}

// "Screen wipe in", top to bottom
void ScreenEffects::vfx12(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int y = -70; y < 312; y += 10) {
		copyFxRect(surface, 0, y, 320, y + 80);
		setBlendedPalette(palette, newPalette, colorCount, y + 70, 260);
		_screen->updateScreenAndWait(25);
	}
	setPalette(palette);
}

// "Screen wipe in", bottom to top
void ScreenEffects::vfx13(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	for (int y = 312; y > -70; y -= 10) {
		copyFxRect(surface, 0, y, 320, y + 80);
		setBlendedPalette(palette, newPalette, colorCount, y + 70, 260);
		_screen->updateScreenAndWait(25);
	}
	setPalette(palette);
}

// "Screen open" effect
void ScreenEffects::vfx14(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	int16 x = 8, y = 5;
	for (int i = 0; i < 27; i++) {
		copyFxRect(surface, 160 - x, 100 - y, 160 + x, 100 + y);
		x += 8;
		y += 5;
		setBlendedPalette(palette, newPalette, colorCount, i, 27);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx15(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	int16 x = 8;
	for (int i = 0; i < 27; i++) {
		copyFxRect(surface, 160 - x, 0, 160 + x, 200);
		x += 8;
		setBlendedPalette(palette, newPalette, colorCount, i, 27);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

void ScreenEffects::vfx16(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {
	int16 y = 8;
	for (int i = 0; i < 27; i++) {
		copyFxRect(surface, 0, 100 - y, 320, 100 + y);
		y += 5;
		setBlendedPalette(palette, newPalette, colorCount, i, 27);
		_screen->updateScreenAndWait(25);
	}
 	setPalette(palette);
}

// Palette fadeout/fadein
void ScreenEffects::vfx17(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount) {

	byte tempPalette[768];

	bool savedPaletteLock = _screen->isPaletteLocked();
	_screen->setPaletteLock(false);

	memcpy(tempPalette, palette, 768);

	// Fade out to black
	memset(palette, 0, 768);
	for (int i = 0; i < 50; i++) {
		setBlendedPalette(palette, newPalette, colorCount, i, 50);
		_screen->updateScreenAndWait(25);
	}
	_screen->setRGBPalette(palette, 0, colorCount);

	memcpy(palette, tempPalette, 768);

	_screen->showWorkScreen();

	// Fade from black to palette
	memset(newPalette, 0, 768);
	for (int i = 0; i < 50; i++) {
		setBlendedPalette(palette, newPalette, colorCount, i, 50);
		_screen->updateScreenAndWait(25);
	}
	_screen->setRGBPalette(palette, 0, colorCount);

	_screen->setPaletteLock(savedPaletteLock);

}


} // End of namespace Made
