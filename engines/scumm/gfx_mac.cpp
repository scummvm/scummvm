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

#include "common/system.h"
#include "scumm/charset.h"
#include "scumm/usage_bits.h"

namespace Scumm {

void ScummEngine::mac_drawStripToScreen(VirtScreen *vs, int top, int x, int y, int width, int height) {
	const byte *pixels = vs->getPixels(x, top);
	const byte *ts = (byte *)_textSurface.getBasePtr(x * 2, y * 2);
	byte *mac = (byte *)_macScreen->getBasePtr(x * 2, y * 2);

	int pixelsPitch = vs->pitch;
	int tsPitch = _textSurface.pitch;
	int macPitch = _macScreen->pitch;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
				mac[2 * w] = pixels[w];
			if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
				mac[2 * w + 1] = pixels[w];
			if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
				mac[2 * w + macPitch] = pixels[w];
			if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
				mac[2 * w + macPitch + 1] = pixels[w];
		}

		pixels += pixelsPitch;
		ts += tsPitch * 2;
		mac += macPitch * 2;
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(x * 2, y * 2), _macScreen->pitch, x * 2, y * 2, width * 2, height * 2);
}

void ScummEngine::mac_restoreCharsetBg() {
	_nextLeft = _string[0].xpos;
	_nextTop = _string[0].ypos + _screenTop;

	if (_charset->_hasMask) {
		_charset->_hasMask = false;
		_charset->_str.left = -1;
		_charset->_left = -1;

		clearTextSurface();

		VirtScreen *vs = &_virtscr[_charset->_textScreenID];
		if (!vs->h)
			return;

		markRectAsDirty(vs->number, Common::Rect(vs->w, vs->h), USAGE_BIT_RESTORED);
	}
}

void ScummEngine::mac_drawLoomPracticeMode() {
	// In practice mode, the game shows the notes as they are being played.
	// In the DOS version, this is drawn by script 27 but the Mac version
	// just sets variables 50 and 54. I'm not sure what the difference
	// between the two is.

	int x = 216;
	int y = 377;
	int width = 62;
	int height = 22;
	int var = 50;

	byte *ptr = (byte *)_macScreen->getBasePtr(x,  y);
	int pitch = _macScreen->pitch;

	_macScreen->fillRect(Common::Rect(x, y, x + width, y + height), 0);

	if (VAR(var)) {
		for (int w = 1; w < width - 1; w++) {
			ptr[w] = 7;
			ptr[w + pitch * (height - 1)] = 7;
		}

		for (int h = 1; h < height - 1; h++) {
			ptr[h * pitch] = 7;
			ptr[h * pitch + width - 1] = 7;
		}

		// Draw the notes
		int colors[] = { 4, 12, 14, 10, 11, 3, 9, 15 };

		for (int i = 0; i < 4; i++) {
			int note = (VAR(var) >> (4 * i)) & 0x0F;

			if (note >= 2 && note <= 9) {
				_charset->setColor(colors[note - 2]);
				_charset->drawChar(14 + note, *_macScreen, i * 13 + x + 8, y + 4);
			}
		}
	}

	_system->copyRectToScreen(ptr, pitch, x, y, width, height);
}

} // End of namespace Scumm
