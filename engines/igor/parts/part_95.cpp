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

struct SharewareScreenString {
	uint8 color;
	int y;
	int strId;
};

static const SharewareScreenString STR_SHAREWARE[] = {
	// 950
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 228,  70, STR_SHAREWARE_TEXT1 },
	{ 228,  81, STR_SHAREWARE_TEXT2 },
	{ 228,  96, STR_SHAREWARE_TEXT3 },
	{ 228, 107, STR_SHAREWARE_TEXT4 },
	{ 228, 122, STR_SHAREWARE_TEXT5 },
	{ 228, 161, STR_SHAREWARE_PLACE_ORDER },
	// 951
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 228,  42, STR_SHAREWARE_VERSION },
	{ 228,  85, STR_SHAREWARE_ORDER_1 },
	{ 228,  96, STR_SHAREWARE_ORDER_2 },
	{ 228, 107, STR_SHAREWARE_ORDER_3 },
	{ 228, 161, STR_SHAREWARE_ORDER_4 },
	// 952
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 228,  42, STR_SHAREWARE_VERSION },
	{ 228,  70, STR_SHAREWARE_SHIPPING_1 },
	{ 228,  85, STR_SHAREWARE_SHIPPING_2 },
	{ 228,  96, STR_SHAREWARE_SHIPPING_3 },
	{ 228, 111, STR_SHAREWARE_SHIPPING_4 },
	{ 228, 122, STR_SHAREWARE_SHIPPING_5 },
	{ 228, 161, STR_SHAREWARE_PLACE_ORDER },
	// 953
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 228,  42, STR_SHAREWARE_VERSION },
	{ 228,  70, STR_SHAREWARE_SHIPPING_6 },
	{ 228,  85, STR_SHAREWARE_SHIPPING_7 },
	{ 228,  96, STR_SHAREWARE_SHIPPING_8 },
	{ 228, 110, STR_SHAREWARE_SHIPPING_9 },
	{ 228, 121, STR_SHAREWARE_SHIPPING_10 },
	{ 228, 161, STR_SHAREWARE_PLACE_ORDER },
	// 954
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 228,  42, STR_SHAREWARE_VERSION },
	{ 228,  74, STR_SHAREWARE_OPTIK_ADDRESS_1 },
	{ 228,  85, STR_SHAREWARE_OPTIK_ADDRESS_2 },
	{ 228,  96, STR_SHAREWARE_OPTIK_ADDRESS_3 },
	{ 228, 107, STR_SHAREWARE_OPTIK_ADDRESS_4 },
	{ 228, 118, STR_SHAREWARE_OPTIK_ADDRESS_5 },
	{ 228, 161, STR_SHAREWARE_PLACE_ORDER },
	// 955
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 240,  42, STR_SHAREWARE_VERSION },
	{ 240,  79, STR_SHAREWARE_OPTIK_ADDRESS_6 },
	{ 240,  90, STR_SHAREWARE_OPTIK_ADDRESS_7 },
	{ 240, 101, STR_SHAREWARE_OPTIK_ADDRESS_8 },
	{ 240, 112, STR_SHAREWARE_OPTIK_ADDRESS_9 },
	{ 240, 161, STR_SHAREWARE_PLACE_ORDER },
	// 956
	{ 255,  30, STR_IGOR_OBJECTIVE_UIKOKAHONIA },
	{ 228,  42, STR_SHAREWARE_VERSION },
	{ 228,  64, STR_SHAREWARE_PENDULO_ADDRESS_1 },
	{ 228,  80, STR_SHAREWARE_PENDULO_ADDRESS_2 },
	{ 228,  91, STR_SHAREWARE_PENDULO_ADDRESS_3 },
	{ 228, 102, STR_SHAREWARE_PENDULO_ADDRESS_4 },
	{ 228, 113, STR_SHAREWARE_PENDULO_ADDRESS_5 },
	{ 228, 128, STR_SHAREWARE_PENDULO_ADDRESS_6 },
	{ 228, 161, STR_SHAREWARE_PLACE_ORDER }
};

void IgorEngine::PART_95() {
	hideCursor();
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	memset(_screenVGA, 0, 64000);
	int startStr = -1, endStr = -1;
	switch (_currentPart) {
	case 950:
		loadData(PAL_Shareware1, _paletteBuffer);
		loadData(IMG_Shareware1, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 0;
			endStr = 6;
		}
		break;
	case 951:
		loadData(PAL_Shareware2, _paletteBuffer);
		loadData(IMG_Shareware2, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 7;
			endStr = 12;
		}
		break;
	case 952:
		loadData(PAL_Shareware3, _paletteBuffer);
		loadData(IMG_Shareware3, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 13;
			endStr = 19;
		}
		break;
	case 953:
		loadData(PAL_Shareware4, _paletteBuffer);
		loadData(IMG_Shareware4, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 20;
			endStr = 27;
		}
		break;
	case 954:
		loadData(PAL_Shareware5, _paletteBuffer);
		loadData(IMG_Shareware5, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 28;
			endStr = 35;
		}
		break;
	case 955:
		loadData(PAL_Shareware6, _paletteBuffer);
		loadData(IMG_Shareware6, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 36;
			endStr = 42;
		}
		break;
	case 956:
		loadData(PAL_Shareware7, _paletteBuffer);
		loadData(IMG_Shareware7, _screenVGA);
		if (_gameVersion == kIdEngDemo110) {
			startStr = 43;
			endStr = 51;
		}
		break;
	}
	if (startStr != -1) {
		for (int i = startStr; i <= endStr; ++i) {
			const SharewareScreenString *s = &STR_SHAREWARE[i];
			const char *str = getString(s->strId);
			drawString(_screenVGA, str, (320 - getStringWidth(str)) / 2, s->y, s->color, 0, 0);
		}
	}
	fadeInPalette(768);
	for (int i = 0; !_inputVars[kInputEscape] && i < 3000; ++i) {
		waitForTimer();
	}
	_inputVars[kInputEscape] = 0;
	fadeOutPalette(768);
}

} // namespace Igor
