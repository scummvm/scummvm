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

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_90() {
	const char *str;

	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	switch (_currentPart) {
	case 900:
		loadData(PAL_OptikSoftware, _paletteBuffer);
		loadData(IMG_OptikSoftware, _screenVGA);
		break;
	case 901:
		loadData(PAL_Presents, _paletteBuffer);
		loadData(IMG_Presents, _screenVGA);
		break;
	case 902:
		loadData(PAL_GraphicAdventure, _paletteBuffer);
		loadData(IMG_GraphicAdventure, _screenVGA);
		break;
	case 903:
		loadData(PAL_PenduloStudios, _paletteBuffer);
		loadData(IMG_PenduloStudios, _screenVGA);
		break;
	case 904:
		loadData(PAL_TitleScreen, _paletteBuffer);
		loadData(IMG_TitleScreen, _screenVGA);
		str = (_game.version == kIdEngDemo110) ? getString(STR_Copyright1994) : getString(STR_Copyright1995);
		drawString(_screenVGA, str, 2, 187, 0xF5, 0, 0);
		break;
	}
	fadeInPalette(768);
	while (!_inputVars[kInputEscape]) {
		waitForTimer();
		if (_inputVars[kInputOptions]) {
			_inputVars[kInputOptions] = 0;
			handleOptionsMenu();
			break;
		}
	}
	_inputVars[kInputEscape] = 0;
	fadeOutPalette(768);
	if (_currentPart != kInvalidPart) {
		++_currentPart;
		if ((_game.version == kIdSpaCD && _currentPart == 904) || _currentPart == 905) {
			_currentPart = 850;
		}
	}
}

} // namespace Igor
