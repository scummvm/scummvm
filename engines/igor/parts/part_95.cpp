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

#include "common/system.h"

#include "igor/igor.h"

namespace Igor {

struct SharewareScreenString {
	uint8 color;
	int y;
	int strId;
};

static const SharewareScreenString STR_SHAREWARE[] = {
	// 950
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 228,  70, STR_SharewareText1 },
	{ 228,  81, STR_SharewareText2 },
	{ 228,  96, STR_SharewareText3 },
	{ 228, 107, STR_SharewareText4 },
	{ 228, 122, STR_SharewareText5 },
	{ 228, 161, STR_SharewarePlaceOrder },
	// 951
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 228,  42, STR_SharewareVersion },
	{ 228,  85, STR_SharewareOrder1 },
	{ 228,  96, STR_SharewareOrder2 },
	{ 228, 107, STR_SharewareOrder3 },
	{ 228, 161, STR_SharewareOrder4 },
	// 952
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 228,  42, STR_SharewareVersion },
	{ 228,  70, STR_SharewareShipping1 },
	{ 228,  85, STR_SharewareShipping2 },
	{ 228,  96, STR_SharewareShipping3 },
	{ 228, 111, STR_SharewareShipping4 },
	{ 228, 122, STR_SharewareShipping5 },
	{ 228, 161, STR_SharewarePlaceOrder },
	// 953
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 228,  42, STR_SharewareVersion },
	{ 228,  70, STR_SharewareShipping6 },
	{ 228,  85, STR_SharewareShipping7 },
	{ 228,  96, STR_SharewareShipping8 },
	{ 228, 110, STR_SharewareShipping9 },
	{ 228, 121, STR_SharewareShipping10 },
	{ 228, 161, STR_SharewarePlaceOrder },
	// 954
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 228,  42, STR_SharewareVersion },
	{ 228,  74, STR_SharewareOptikAddress1 },
	{ 228,  85, STR_SharewareOptikAddress2 },
	{ 228,  96, STR_SharewareOptikAddress3 },
	{ 228, 107, STR_SharewareOptikAddress4 },
	{ 228, 118, STR_SharewareOptikAddress5 },
	{ 228, 161, STR_SharewarePlaceOrder },
	// 955
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 240,  42, STR_SharewareVersion },
	{ 240,  79, STR_SharewareOptikAddress6 },
	{ 240,  90, STR_SharewareOptikAddress7 },
	{ 240, 101, STR_SharewareOptikAddress8 },
	{ 240, 112, STR_SharewareOptikAddress9 },
	{ 240, 161, STR_SharewarePlaceOrder },
	// 956
	{ 255,  30, STR_IgorObjectiveUikokahonia },
	{ 228,  42, STR_SharewareVersion },
	{ 228,  64, STR_SharewarePenduloAddress1 },
	{ 228,  80, STR_SharewarePenduloAddress2 },
	{ 228,  91, STR_SharewarePenduloAddress3 },
	{ 228, 102, STR_SharewarePenduloAddress4 },
	{ 228, 113, STR_SharewarePenduloAddress5 },
	{ 228, 128, STR_SharewarePenduloAddress6 },
	{ 228, 161, STR_SharewarePlaceOrder },
};

static const struct {
	int strId;
	int offset;
	int length;
	int extraSpacing;
} sharewareHighlights[] = {
	{ STR_SharewareOrder1, 34, 6, 0 },
	{ STR_SharewareOrder2, 0, 5, 0 },
	{ STR_SharewareOrder3, 11, 5, 0 },
	{ STR_SharewareShipping7, 7, 14, 0 },
	{ STR_SharewareShipping10, 16, 14, 5 },
	{ STR_SharewareOptikAddress7, 13, 25, 0 },
	{ STR_SharewareOptikAddress8, 5, 14, 0 },
	{ STR_SharewarePlaceOrder, 19, 14, 0 },
	{ STR_SharewareOrder4, 19, 14, 0 },
};

void IgorEngine::part95() {
	hideCursor();
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	memset(_screenVGA, 0, 64000);
	int startStr = -1, endStr = -1;
	switch (_currentPart) {
	case 950:
		loadData(PAL_Shareware1, _paletteBuffer);
		loadData(IMG_Shareware1, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 0;
			endStr = 6;
		}
		break;
	case 951:
		loadData(PAL_Shareware2, _paletteBuffer);
		loadData(IMG_Shareware2, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 7;
			endStr = 12;
		}
		break;
	case 952:
		loadData(PAL_Shareware3, _paletteBuffer);
		loadData(IMG_Shareware3, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 13;
			endStr = 20;
		}
		break;
	case 953:
		loadData(PAL_Shareware4, _paletteBuffer);
		loadData(IMG_Shareware4, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 21;
			endStr = 28;
		}
		break;
	case 954:
		loadData(PAL_Shareware5, _paletteBuffer);
		loadData(IMG_Shareware5, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 29;
			endStr = 36;
		}
		break;
	case 955:
		loadData(PAL_Shareware6, _paletteBuffer);
		loadData(IMG_Shareware6, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 37;
			endStr = 43;
		}
		break;
	case 956:
		loadData(PAL_Shareware7, _paletteBuffer);
		loadData(IMG_Shareware7, _screenVGA + ((_game.version == kIdEngDemo110) ? 29 : 28) * 320);
		if (_game.version == kIdEngDemo110) {
			startStr = 44;
			endStr = 52;
		}
		break;
	}
	if (startStr != -1) {
		for (int i = startStr; i <= endStr; ++i) {
			const SharewareScreenString *s = &STR_SHAREWARE[i];
			const char *str = getString(s->strId);
			const int x = (320 - _font.getStringWidth(str)) / 2;
			bool highlighted = false;
			for (uint j = 0; j < ARRAYSIZE(sharewareHighlights); ++j) {
				if (sharewareHighlights[j].strId == s->strId) {
					const Common::String prefix(str, sharewareHighlights[j].offset);
					const Common::String text(str + sharewareHighlights[j].offset, sharewareHighlights[j].length);
					const int highlightX = x + _font.getStringWidth(prefix.c_str()) + sharewareHighlights[j].extraSpacing;
					_font.drawString(_screenVGA, prefix.c_str(), x, s->y, s->color, 0, 0);
					_font.drawString(_screenVGA, text.c_str(), highlightX, s->y, s->color == 240 ? 239 : 223, 0, 0);
					_font.drawString(_screenVGA, str + sharewareHighlights[j].offset + sharewareHighlights[j].length,
						highlightX + _font.getStringWidth(text.c_str()), s->y, s->color, 0, 0);
					highlighted = true;
					break;
				}
			}
			if (!highlighted)
				_font.drawString(_screenVGA, str, x, s->y, s->color, 0, 0);
		}
	}
	part90FadePalette(true);
	_inputVars[kInputEscape] = 0;
	_inputVars[kInputConfirm] = 0;
	_inputVars[kInputClick] = 0;
	_inputVars[kInputRightClick] = 0;
	const uint32 timerStart = _system->getMillis();
	for (int i = 0; !_eventQuitGame && i < 3000; ++i) {
		const uint32 deadline = timerStart + (uint64)(i + 1) * 1000 / kTickDelay / _fastMode;
		waitForTimer(0, true, deadline);
		_gameTicks = (_gameTicks + 1) & 63;
		if (_inputVars[kInputEscape] || _inputVars[kInputConfirm] || _inputVars[kInputClick] || _inputVars[kInputRightClick])
			break;
		_inputVars[kInputEscape] = 0;
		_inputVars[kInputConfirm] = 0;
		_inputVars[kInputClick] = 0;
		_inputVars[kInputRightClick] = 0;
	}
	_inputVars[kInputEscape] = 0;
	_inputVars[kInputConfirm] = 0;
	_inputVars[kInputClick] = 0;
	_inputVars[kInputRightClick] = 0;
	part90FadePalette(false);
}

} // namespace Igor
