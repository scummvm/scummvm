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

#include "igor/igor.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "graphics/paletteman.h"

namespace Igor {

void IgorEngine::setupDefaultPalette() {
	memset(_currentPalette, 0, 255 * 3);
	memset(&_currentPalette[255 * 3], 63, 3);
	updatePalette(768);
	// g_system->getPaletteManager()->setPalette(_currentPalette, 0, 256);
}

void IgorEngine::scrollPalette(int startColor, int endColor) {
	uint8 c[3];
	memcpy(c, &_currentPalette[startColor * 3], 3);
	memmove(&_currentPalette[startColor * 3], &_currentPalette[(startColor + 1) * 3], (endColor - startColor) * 3);
	memcpy(&_currentPalette[endColor * 3], c, 3);
}

void IgorEngine::setPaletteRange(int startColor, int endColor) {
	debugC(9, kDebugScreen, "setPaletteRange(%d, %d)", startColor, endColor);
	assert(endColor - startColor + 1 <= 256);
	for (int i = startColor; i <= endColor; ++i) {
		setPaletteColor(i, _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
	}
}

void IgorEngine::setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b) {
	uint8 pal[4];
	pal[0] = (r << 2) | (r >> 4);
	pal[1] = (g << 2) | (g >> 4);
	pal[2] = (b << 2) | (b >> 4);
	pal[3] = 0;
	g_system->getPaletteManager()->setPalette(pal, index, 1);
}

void IgorEngine::updatePalette(int count) {
	assert(count <= 768);
	uint8 pal[768];
	for (int j = 0, i = 0; i < count; ++i) {
		pal[j++] = (_currentPalette[i] << 2);// | (_currentPalette[i] >> 4);
		// if (((i + 1) % 3) == 0) {
		// 	pal[j++] = 0;
		// }
	}
	g_system->getPaletteManager()->setPalette(pal, 0, count / 3);
}

void IgorEngine::SET_PAL_240_48_1() {

	uint8 pal[48];
	for (int j = 0, i = 0; i < 48; ++i) {
		pal[j++] = (PAL_48_1[i] << 2);// | (_currentPalette[i] >> 4);
	// 	// if (((i + 1) % 3) == 0) {
	// 	// 	pal[j++] = 0;
	// 	// }
	}
	g_system->getPaletteManager()->setPalette(pal, 240, 16);
}

void IgorEngine::fadeInPalette(int count) {
	debugC(9, kDebugScreen, "fadeInPalette(%d)", count);
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	int m = 66;
	do {
		m -= 3;
		for (int i = 0; i < count; ++i) {
			if (_paletteBuffer[i] >= m) {
				uint8 color = _currentPalette[i] + 3;
				if (color > _paletteBuffer[i]) {
					color = _paletteBuffer[i];
				}
				_currentPalette[i] = color;
			}
		}
		updatePalette(count);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	} while (m > 0);
}

void IgorEngine::fadeOutPalette(int count) {
	debugC(9, kDebugScreen, "fadeOutPalette(%d)", count);
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	memcpy(_paletteBuffer, _currentPalette, 768);
	int m = 0;
	do {
		for (int i = 0; i < count; ++i) {
			if (_paletteBuffer[i] >= m) {
				uint8 color = _currentPalette[i];
				if (color >= 3) {
					color -= 3;
				} else {
					color = 0;
				}
				_currentPalette[i] = color;
			}
		}
		updatePalette(count);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
		m += 3;
	} while (m < 66);
}

} // End of namespace Igor
