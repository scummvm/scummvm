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
#include "graphics/paletteman.h"
#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {

void BmpButton::loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled) {
	_base.loadBitmap(base);
	_selected.loadBitmap(selected);
	_focused.loadBitmap(focused);
	_disabled.loadBitmap(disabled);
}

void BmpButton::clear() {
	_base.clear();
	_selected.clear();
	_focused.clear();
	_disabled.clear();
}

void BmpButton::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_base);
}

/*------------------------------------------------------------------------*/

void ColorButton::draw() {
	GfxSurface s = getSurface();
	COLORREF myTextColor, oldTextColor,
		myPen, myInversePen, myBrush;
	Common::String text;
	int nUnderscore = -1;
	int i, x, y, dx, dy;
	Common::Point textInfo, letterInfo,
		underscoreInfo;
	Common::Rect focusRect;
	byte color;

	// Check for any ampersand character, which indicates underline
	// for the keyboard key associated with the button
	text = _text;
	for (i = 0; i < (int)text.size(); i++) {
		if (text[i] == '&') {
			nUnderscore = i;
			text.deleteChar(i);
		}
	}

	x = 0;
	y = 0;

	if ((_itemState & ODS_GRAYED) ||
		(_itemState & ODS_DISABLED)) {
		myPen = _cButtonHighlight;
		myInversePen = _cButtonShadow;
		myBrush = _cButtonFace;
		myTextColor = _cButtonTextDisabled;
	} else if (_itemState & ODS_SELECTED) {
		myPen = _cButtonShadow;
		myInversePen = _cButtonHighlight;
		myBrush = _cButtonFace;
		myTextColor = _cButtonText;
		x = SELECTED_TEXT_DX;
		y = SELECTED_TEXT_DY;
	} else {
		myPen = _cButtonHighlight;
		myInversePen = _cButtonShadow;
		myBrush = _cButtonFace;
		myTextColor = _cButtonText;
	}

	color = getPaletteIndex(myPen);
	Common::Rect area(0, 0, _bounds.width() - 1, _bounds.height() - 1);
	s.fillRect(area, color);

	// Draw the button edges
	color = getPaletteIndex(myInversePen);
	for (i = 1; i <= BUTTON_EDGE_WIDTH; i++) {              
		s.hLine(i, _bounds.height() - i, _bounds.width() - i, color);
		s.vLine(_bounds.width() - i, i - 1, _bounds.height() - i, color);
	}

#ifdef TODO
	color = getPaletteIndex(_cButtonOutline);

	dx = rcItem.right - rcItem.left;
	dy = rcItem.bottom - rcItem.top;
	x += (dx - textInfo.cx) >> 1;
	y += (dy - textInfo.cy) >> 1;

	(*pDC).SetBkMode(TRANSPARENT);        					// make the text overlay transparently
	oldTextColor = (*pDC).SetTextColor(myTextColor);         // set the color of the text

	(*pDC).TextOut(x, y, (LPCSTR)&chMyText, nMyTextLength);

	if (nUnderscore >= 0) {
		underscoreInfo = (*pDC).GetTextExtent(chMyText, nUnderscore);
		dx = x + underscoreInfo.cx;
		letterInfo = (*pDC).GetTextExtent(&chMyText[nUnderscore], 1);
		underscoreInfo = (*pDC).GetTextExtent((LPCSTR)"_", 1);
		dx += (letterInfo.cx - underscoreInfo.cx) >> 1;
		(*pDC).TextOut(dx, y, (LPCSTR)"_", 1);
	}

	(void)(*pDC).SetTextColor(oldTextColor);         		// set the color of the text

	if (_itemState & ODS_FOCUS) {
		focusRect.SetRect(x - FOCUS_RECT_DX,
			y - FOCUS_RECT_DY,
			x + textInfo.cx + FOCUS_RECT_DX,
			y + textInfo.cy + FOCUS_RECT_DY + 1);
		myQuill.CreateStockObject(DKGRAY_BRUSH);
		(*pDC).FrameRect(&focusRect, &myQuill);
	}
#endif
}

} // namespace HodjNPodj
} // namespace Bagel
