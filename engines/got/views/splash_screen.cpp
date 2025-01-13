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

#include "got/views/splash_screen.h"
#include "common/file.h"
#include "got/gfx/palette.h"
#include "got/vars.h"

namespace Got {
namespace Views {

#define SPLASH_FRAME_INTERVAL 2

void SplashScreen::draw() {
	if (_frameCtr == -1) {
		_frameCtr = 0;
		GfxSurface s = getSurface();

		// Display background. The rest of the logo animation will be
		// done in the frame routines called from tick
		s.clear();
		s.simpleBlitFrom(_G(gfx[92]), Common::Point(0, 24));
	}
}

bool SplashScreen::msgFocus(const FocusMessage &msg) {
	Gfx::GraphicChunk chunk = _G(gfx[93]);
	_frameCount = READ_LE_UINT16(chunk._data);
	_chunkSize = chunk._data + 2;
	_chunkPtr = chunk._data + 2 + _frameCount * 4;

	_frameCtr = -1;
	_delayCtr = 0;

	// This is the first screen shown, so start with black, and fade it in
	byte blackPal[PALETTE_SIZE];
	Common::fill(blackPal, blackPal + PALETTE_SIZE, 0);
	Gfx::xsetpal(blackPal);

	draw();
	Gfx::Palette63 pal = _G(gfx[91]);
	Gfx::fade_in(pal);

	return true;
}

bool SplashScreen::msgUnfocus(const UnfocusMessage &msg) {
	fadeOut();
	return true;
}

bool SplashScreen::tick() {
	if (++_delayCtr == SPLASH_FRAME_INTERVAL) {
		_delayCtr = 0;

		if (++_frameCtr < _frameCount) {
			GfxSurface s = getSurface();
			byte *dest = (byte *)s.getBasePtr(0, 24);
			executeFrame(_chunkPtr, dest);
			s.markAllDirty();

			_chunkPtr += READ_LE_UINT32(_chunkSize);
			_chunkSize += 4;
		} else if (_frameCtr == (_frameCount + 50)) {
			// Switch to the opening screen showing the game name
			replaceView("Opening", true);
		}
	}

	return true;
}
int frame = -1;

void SplashScreen::executeFrame(const byte *src, byte *dest) {
	const byte *codeP = src;
	int count = 0;
	int val = 0;

	while (*codeP != 0xcb) {
		if (codeP[0] == 0xc9) {
			// leave
			codeP++;
		} else if (codeP[0] == 0x33 && codeP[1] == 0xc9) {
			// xor cx, cx
			count = 0;
			codeP += 2;
		} else if (codeP[0] == 0x81 && codeP[1] == 0xc6) {
			// add si, 16-bit
			src += READ_LE_INT16(codeP + 2);
			codeP += 4;
		} else if (codeP[0] == 0x81 && codeP[1] == 0xee) {
			// sub si, 16-bit
			src -= READ_LE_INT16(codeP + 2);
			codeP += 4;
		} else if (codeP[0] == 0x8b && codeP[1] == 0xdf) {
			// mov bx, di
			codeP += 2;
		} else if (codeP[0] == 0x81 && codeP[1] == 0xc7) {
			// add di, 16-bit
			dest += READ_LE_INT16(codeP + 2);
			codeP += 4;
		} else if (codeP[0] == 0xb8) {
			// mov ax, 16-bit
			val = READ_LE_UINT16(codeP + 1);
			codeP += 3;
		} else if (codeP[0] == 0xb0) {
			// mov al, 8-bit
			val = codeP[1];
			codeP += 2;
		} else if (codeP[0] == 0xb1) {
			// mov cl, 8-bit
			count = codeP[1];
			codeP += 2;
		} else if (codeP[0] == 0xf3 && codeP[1] == 0xab) {
			// rep stosw
			while (count-- > 0) {
				WRITE_LE_UINT16(dest, val);
				dest += 2;
			}
			codeP += 2;
		} else if (codeP[0] == 0xab) {
			// stosw
			WRITE_LE_UINT16(dest, val);
			dest += 2;
			codeP++;
		} else if (codeP[0] == 0xaa) {
			// stosb
			*dest++ = (byte)val;
			++codeP;
		} else if (codeP[0] == 0xf3 && codeP[1] == 0xa5) {
			// rep movsw
			Common::copy(src, src + count * 2, dest);
			src += count * 2;
			dest += count * 2;
			codeP += 2;
		} else if (codeP[0] == 0xa4) {
			// movsb
			*dest++ = *src++;
			++codeP;
		} else {
			error("Unhandled opcode");
		}
	}
}

} // namespace Views
} // namespace Got
