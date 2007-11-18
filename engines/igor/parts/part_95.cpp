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
	const char *str;
};

static const SharewareScreenString STR_SHAREWARE[] = {
	// 950
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 228,  70, "This is SHAREWARE!" },
	{ 228,  81, "You can copy this version!" },
	{ 228,  96, "Pass it around, give it to your friends, family," },
	{ 228, 107, "colleagues and upload it to your favorite BBS." },
	{ 228, 122, "Let everyone enjoy IGOR!" },
	{ 228, 161, "To place an order: 1-800-OPTIK-99" },
	// 951
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 228,  42, "Shareware version" },
	{ 228,  85, "Order the full IGOR game for only $34.99 US." },
	{ 228,  96, "$5.00 for shipping and handling (US & CANADA)." },
	{ 228, 107, "Please add $3.00 for international shipping." },
	{ 228, 161, "To place an order: 1-800-OPTIK-99" },
	// 952
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 228,  42, "Shareware version" },
	{ 228,  70, "90 day limited warranty." },
	{ 228,  85, "Please allow 2-4 days for delivery (US only)." },
	{ 228,  96, "Elsewhere, up to a week or two..." },
	{ 228, 111, "Overnight/second day shipping available an" },
	{ 228, 122, "aditional change. Please call for exact pricing." },
	{ 228, 161, "To place an order: 1-800-OPTIK-99" },
	// 953
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 228,  42, "Shareware version" },
	{ 228,  70, "Three easy ways to order:" },
	{ 228,  85, "- Call 1-800-678-4599 (orders only) and use" },
	{ 228,  96, "Your Visa, Mastercard or Discover card." },
	{ 228, 110, "- Fax your order (please include credit card" },
	{ 228, 121, "information) to (412) 381-1031" },
	{ 228, 161, "To place an order: 1-800-OPTIK-99" },
	// 954
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 228,  42, "Shareware version" },
	{ 228,  74, "- Mail a check or money order to:" },
	{ 228,  85, "Optik Software Inc." },
	{ 228,  96, "1000 Napor Boulevard" },
	{ 228, 107, "Pittsburgh, PA. 15205" },
	{ 228, 118, "USA" },
	{ 228, 161, "To place an order: 1-800-OPTIK-99" },
	// 955
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 240,  42, "Shareware version" },
	{ 240,  79, "Optik Software Inc." },
	{ 240,  90, "Orders only: 1-800-OPTIK-99 (67845-99)" },
	{ 240, 101, "Fax: (412) 381-1031" },
	{ 240, 112, "E-mail: optiksoft\xFA""aol.com" },
	{ 240, 161, "To place an order: 1-800-OPTIK-99" },
	// 956
	{ 255,  30, "\"Igor. Objective Uikokahonia\"" },
	{ 228,  42, "Shareware version" },
	{ 228,  64, "A game by" },
	{ 228,  80, "PENDULO STUDIOS" },
	{ 228,  91, "P.O. Box 21091" },
	{ 228, 102, "28009 Madrid" },
	{ 228, 113, "Spain" },
	{ 228, 128, "E-mail: 100641.1737\xFA""compuserve.com" },
	{ 228, 161, "To place an order: 1-800-OPTIK-99" }
};

void IgorEngine::PART_95() {
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
	for (int i = startStr; i <= endStr; ++i) {
		const SharewareScreenString *s = &STR_SHAREWARE[i];
		drawString(_screenVGA, s->str, (320 - getStringWidth(s->str)) / 2, s->y, s->color, 0, 0);
	}
	fadeInPalette(768);
	for (int i = 0; !_inputVars[kInputEscape] && i < 3000; ++i) {
		waitForTimer();
	}
	_inputVars[kInputEscape] = 0;
	fadeOutPalette(768);
}

} // namespace Igor
