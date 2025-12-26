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

#include "bagel/mfc/gfx/text_render.h"
#include "bagel/mfc/wingdi.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

CSize TextRender::renderText(const Common::String &str,
		Gfx::Surface *dest, Graphics::Font *font,
		uint textCol, LPCRECT lpRect, unsigned int nFormat,
		const Common::Array<int> &tabStops,
		int nTabOrigin, uint bkColor, int bkMode,
		uint textColor, uint textAlign) {
	const int maxWidth = lpRect->right - lpRect->left;
	Common::Rect textRect = *lpRect;
	Common::StringArray lines;
	CSize size;

	// FIXME: Currently doing a hack to replace
	// all tabs with spaces, rather than honoring
	// the tab stops array
	Common::String text = str;
	uint p;
	while ((p = text.findFirstOf('\t')) != Common::String::npos) {
		text.deleteChar(p);
		text.insertString("     ", p);
	}

	if (nFormat & DT_SINGLELINE) {
		lines.push_back(text);
	} else {
		// Hotkeys aren't supported on multi-line text
		assert((nFormat & DT_NOPREFIX) || !str.contains('&'));

		// Perform word wrapping of the text as necessary
		wordWrapText(font, text, tabStops, maxWidth, lines);
	}

	// Handle vertical alignment
	const int linesHeight = lines.size() * font->getFontHeight();

	if (nFormat & DT_BOTTOM) {
		textRect.moveTo(textRect.left,
			MAX<int16>(lpRect->top, textRect.bottom - linesHeight));
	}
	if (nFormat & DT_VCENTER) {
		textRect.moveTo(textRect.left, MAX<int16>(lpRect->top,
			lpRect->top + ((lpRect->bottom - lpRect->top) -
				linesHeight) / 2));
	}

	// Iterate through the lines
	for (const Common::String &line : lines) {
		// Constrain within passed rect
		if (textRect.top >= lpRect->bottom)
			break;

		const int lineWidth = getStringWidth(font, line);
		size.cx = MAX(size.cx, lineWidth);

		// Form sub-rect for the single line
		Common::Rect lineRect(textRect.left, textRect.top,
			textRect.right, textRect.top + font->getFontHeight());

		// Handle horizontal alignment
		if (nFormat & DT_RIGHT) {
			lineRect.moveTo(MAX<int16>(lineRect.left,
				lineRect.right - lineWidth),
				lineRect.top);
		}
		if (nFormat & DT_CENTER) {
			lineRect.moveTo(MAX<int16>(lineRect.left,
				lineRect.left + (lineRect.width() - lineWidth) / 2),
				lineRect.top);
		}

		lineRect.left = MAX<int16>(lineRect.left, 0);
		lineRect.right = MIN<int16>(lineRect.right, dest->w);

		// If the background is opaque, clear it
		if (bkMode == OPAQUE)
			dest->fillRect(lineRect, bkColor);

		// Write the actual text. This is slightly
		// complicated to detect '&' characters when
		// DT_NOPREFIX isn't set
		Common::String fragment;
		Common::String tempLine = line;
		while (!tempLine.empty()) {
			if (!(nFormat & DT_NOPREFIX) && tempLine.firstChar() == '&') {
				tempLine.deleteChar(0);

				// Draw an underline
				const int x1 = lineRect.left;
				const int x2 = x1 + font->getCharWidth(tempLine.firstChar()) - 1;
				const int y = lineRect.top + font->getFontAscent() + 1;

				dest->hLine(x1, y, x2, textCol);
			}

			uint idx = (nFormat & DT_NOPREFIX) ? Common::String::npos :
				tempLine.findFirstOf('&');
			if (idx == Common::String::npos) {
				fragment = tempLine;
				tempLine.clear();
			} else {
				fragment = Common::String(tempLine.c_str(), tempLine.c_str() + idx);
				tempLine = Common::String(tempLine.c_str() + fragment.size());
			}

			Graphics::ManagedSurface area = dest->getSubArea(lineRect);
			font->drawString(&area, fragment, 0, 0, area.w, textCol);
			lineRect.left += getStringWidth(font, fragment);
		}

		// Move to next line
		textRect.top += font->getFontHeight();
	}

	return size;
}

int TextRender::getStringWidth(Graphics::Font *font, const Common::String &str) {
	Common::String tempLine = str;
	uint idx;
	while ((idx = tempLine.findFirstOf('&')) != Common::String::npos)
		tempLine.deleteChar(idx);

	return font->getStringWidth(tempLine);
}

void TextRender::wordWrapText(Graphics::Font *font, const Common::String &str,
		const Common::Array<int> tabStops, int maxWidth,
		Common::StringArray &lines) {
	font->wordWrapText(str, maxWidth, lines);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
