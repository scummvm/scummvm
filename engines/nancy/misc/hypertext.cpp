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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "common/tokenizer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/misc/hypertext.h"

namespace Nancy {
namespace Misc {

struct ColorChange {
	uint numChars;
	byte colorID;
};

void HypertextParser::initSurfaces(uint width, uint height, const Graphics::PixelFormat &format, uint32 backgroundColor, uint32 highlightBackgroundColor) {
	_backgroundColor = backgroundColor;
	_highlightBackgroundColor = highlightBackgroundColor;
	_fullSurface.create(width, height, format);
	_fullSurface.clear(backgroundColor);
	_textHighlightSurface.create(width, height, format);
	_textHighlightSurface.clear(highlightBackgroundColor);
}

void HypertextParser::addTextLine(const Common::String &text) {
	_textLines.push_back(text);
	_needsTextRedraw = true;
}

void HypertextParser::drawAllText(const Common::Rect &textBounds, uint fontID, uint highlightFontID) {
	using namespace Common;

	// Used to get tab width
	const TBOX *tbox = (const TBOX *)g_nancy->getEngineData("TBOX");
	assert(tbox);

	_numDrawnLines = 0;

	for (uint lineID = 0; lineID < _textLines.size(); ++lineID) {
		Common::String currentLine;
		bool hasHotspot = false;
		Rect hotspot;
		Common::Queue<ColorChange> colorChanges;
		int curFontID = fontID;

		// Token braces plus invalid characters that are known to appear in strings
		Common::StringTokenizer tokenizer(_textLines[lineID], "<>\"");

		Common::String curToken;
		while(!tokenizer.empty()) {
			curToken = tokenizer.nextToken();

			if (curToken.size() <= 2) {
				switch (curToken.firstChar()) {
				case 'i' :
					// CC begin
					// fall through
				case 'o' :
					// CC end
					// fall through
				case 'e' :
					// Telephone end
					// Do nothing and just skip
					continue;
				case 'h' :
					// Hotspot
					if (hasHotspot) {
						// Replace duplicate hotspot token with a newline to copy the original behavior
						currentLine += '\n';
					}
					hasHotspot = true;
					continue;
				case 'n' :
					// Newline
					currentLine += '\n';
					continue;
				case 't' :
					// Tab
					for (uint i = 0; i < tbox->tabWidth; ++i) {
						currentLine += " ";
					}
					continue;
				case 'c' :
					// Color tokens
					// We keep the positions and colors of the color tokens in a queue
					if (curToken.size() != 2) {
						break;
					}
					
					colorChanges.push({currentLine.size(), (byte)(curToken[1] - 48)});
					continue;
				case 'f' :
					// Font token
					// This selects a specific font ID for the current line
					if (curToken.size() != 2) {
						break;
					}

					curFontID = (int)Common::String(curToken[1]).asUint64();

					continue;
				}
			}

			currentLine += curToken;
		}
		
		const Font *font = g_nancy->_graphicsManager->getFont(curFontID);
		const Font *highlightFont = g_nancy->_graphicsManager->getFont(highlightFontID);

		// Do word wrapping on the text, sans tokens
		Array<Common::String> wrappedLines;
		font->wordWrap(currentLine, textBounds.width(), wrappedLines, 0);

		// Setup most of the hotspot
		if (hasHotspot) {
			hotspot.left = textBounds.left;
			hotspot.top = textBounds.top + ((_numDrawnLines - 1) * font->getFontHeight()) - 1;
			hotspot.setHeight(wrappedLines.size() * font->getFontHeight());
			hotspot.setWidth(0);
		}

		// Go through the wrapped lines and draw them, making sure to
		// respect color tokens
		uint totalCharsDrawn = 0;
		byte colorID = 0;
		for (Common::String &line : wrappedLines) {
			uint horizontalOffset = 0;

			// Trim whitespaces at end of wrapped lines to make counting
			// of characters consistent. We do this manually since we _want_
			// some whitespaces at the beginning of a line (e.g. tabs)
			if (Common::isSpace(line.lastChar())) {
				line.deleteLastChar();
			}

			// Set the width of the hotspot
			if (hasHotspot) {
				hotspot.setWidth(MAX<int16>(hotspot.width(), font->getStringWidth(line)));
			}

			while (!line.empty()) {
				Common::String subLine;

				if (colorChanges.size()) {
					// Text contains color part

					if (totalCharsDrawn >= colorChanges.front().numChars) {
						// Token is at begginning of (what's left of) the current line
						colorID = colorChanges.pop().colorID;
					}

					if (totalCharsDrawn < colorChanges.front().numChars && colorChanges.front().numChars < (totalCharsDrawn + line.size())) {
						// There's a token inside the current line, so split off the part before it
						subLine = line.substr(0, colorChanges.front().numChars - totalCharsDrawn);
						line = line.substr(subLine.size());
					}
				}

				// Choose whether to draw the subLine, or the full line
				Common::String &stringToDraw = subLine.size() ? subLine : line;

				// Draw the normal text
				font->drawString(				&_fullSurface,
												stringToDraw,
												textBounds.left + horizontalOffset,
												textBounds.top + (_numDrawnLines - 1) * font->getFontHeight(),
												textBounds.width(),
												colorID);

				// Then, draw the highlight
				if (hasHotspot) {
					highlightFont->drawString(	&_textHighlightSurface,
												stringToDraw,
												textBounds.left + horizontalOffset,
												textBounds.top + (_numDrawnLines - 1) * highlightFont->getFontHeight(),
												textBounds.width(),
												colorID);
				}

				if (subLine.size()) {
					horizontalOffset += font->getStringWidth(subLine);
					totalCharsDrawn += subLine.size();
				} else {
					totalCharsDrawn += line.size();
					break;
				}
			}

			++totalCharsDrawn; // Account for newlines, which are removed from the string when doing word wrap
			++_numDrawnLines;

			// Record the height of the text currently drawn. Used for textbox scrolling
			_drawnTextHeight = (_numDrawnLines - 1) * font->getFontHeight();
		}

		// Add the hotspot to the list
		if (hasHotspot) {
			_hotspots.push_back(hotspot);
		}

		// Note: disabled since it was most likely a bug, and is behavior exclusive to the textbox
		/*
		// Simulate a bug in the original engine where player text longer than
		// a single line gets a double newline afterwards
		if (wrappedLines.size() > 1 && hasHotspot) {
			++_numLines;

			if (lineID == _textLines.size() - 1) {
				_lastResponseisMultiline = true;
			}
		}
		*/

		// Add a newline after every full piece of text
		++_numDrawnLines;
	}

	_needsTextRedraw = false;
}

void HypertextParser::clear() {
	if (_textLines.size()) {
		_fullSurface.clear(_backgroundColor);
		_textHighlightSurface.clear(_highlightBackgroundColor);
		_textLines.clear();
		_hotspots.clear();
		_numDrawnLines = 0;
		_drawnTextHeight = 0;
	}
}

} // End of namespace Misc
} // End of namespace Nancy
