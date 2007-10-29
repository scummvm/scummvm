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

void IgorEngine::PART_95() {
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	memset(_screenVGA, 0, 64000);
	switch (_currentPart) {
	case 950:
		loadData(PAL_Shareware1, _paletteBuffer);
		loadData(IMG_Shareware1, _screenVGA);
		break;
	case 951:
		loadData(PAL_Shareware2, _paletteBuffer);
		loadData(IMG_Shareware2, _screenVGA);
		break;
	case 952:
		loadData(PAL_Shareware3, _paletteBuffer);
		loadData(IMG_Shareware3, _screenVGA);
		break;
	case 953:
		loadData(PAL_Shareware4, _paletteBuffer);
		loadData(IMG_Shareware4, _screenVGA);
		break;
	case 954:
		loadData(PAL_Shareware5, _paletteBuffer);
		loadData(IMG_Shareware5, _screenVGA);
		break;
	case 955:
		loadData(PAL_Shareware6, _paletteBuffer);
		loadData(IMG_Shareware6, _screenVGA);
		break;
	case 956:
		loadData(PAL_Shareware7, _paletteBuffer);
		loadData(IMG_Shareware7, _screenVGA);
		break;
	}
	fadeInPalette(768);
	for (int i = 0; !_inputVars[kInputEscape] && i < 3000; ++i) {
		waitForTimer();
	}
	_inputVars[kInputEscape] = 0;
	fadeOutPalette(768);
}

} // namespace Igor
