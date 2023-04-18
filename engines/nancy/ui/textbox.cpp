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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/textbox.h"
#include "engines/nancy/ui/scrollbar.h"

namespace Nancy {
namespace UI {

const char Textbox::_CCBeginToken[] = "<i>";
const char Textbox::_CCEndToken[] = "<o>";
const char Textbox::_colorBeginToken[] = "<c1>";
const char Textbox::_colorEndToken[] = "<c0>";
const char Textbox::_hotspotToken[] = "<h>";
const char Textbox::_newLineToken[] = "<n>";
const char Textbox::_tabToken[] = "<t>";
const char Textbox::_telephoneEndToken[] = "<e>";

Textbox::Textbox() :
		RenderObject(6),
		_needsTextRedraw(false),
		_scrollbar(nullptr),
		_scrollbarPos(0),
		_numLines(0),
		_lastResponseisMultiline(false),
		_highlightRObj(7),
		_fontIDOverride(-1) {}

Textbox::~Textbox() {
	delete _scrollbar;
}

void Textbox::init() {
	TBOX *tbox = g_nancy->_textboxData;
	assert(tbox);

	moveTo(g_nancy->_bootSummary->textboxScreenPosition);
	_highlightRObj.moveTo(g_nancy->_bootSummary->textboxScreenPosition);
	_fullSurface.create(tbox->innerBoundingBox.width(), tbox->innerBoundingBox.height(), g_nancy->_graphicsManager->getScreenPixelFormat());
	_textHighlightSurface.create(tbox->innerBoundingBox.width(), tbox->innerBoundingBox.height(), g_nancy->_graphicsManager->getScreenPixelFormat());
	_textHighlightSurface.setTransparentColor(g_nancy->_graphicsManager->getTransColor());

	Common::Rect outerBoundingBox = _screenPosition;
	outerBoundingBox.moveTo(0, 0);
	_drawSurface.create(_fullSurface, outerBoundingBox);

	RenderObject::init();

	// zOrder bumped by 2 to avoid overlap with the inventory box curtains in The Vampire Diaries
	_scrollbar = new Scrollbar(	11,
								tbox->scrollbarSrcBounds,
								tbox->scrollbarDefaultPos,
								tbox->scrollbarMaxScroll - tbox->scrollbarDefaultPos.y);
	_scrollbar->init();
}

void Textbox::registerGraphics() {
	RenderObject::registerGraphics();
	_scrollbar->registerGraphics();
	_highlightRObj.registerGraphics();
	_highlightRObj.setVisible(false);
}

void Textbox::updateGraphics() {
	if (_needsTextRedraw) {
		drawTextbox();
	}

	if (_scrollbarPos != _scrollbar->getPos()) {
		_scrollbarPos = _scrollbar->getPos();

		onScrollbarMove();
	}

	RenderObject::updateGraphics();
}

void Textbox::handleInput(NancyInput &input) {
	_scrollbar->handleInput(input);

	bool hasHighlight = false;
	for (uint i = 0; i < _hotspots.size(); ++i) {
		Common::Rect hotspot = _hotspots[i];
		hotspot.translate(0, -_drawSurface.getOffsetFromOwner().y);
		Common::Rect hotspotOnScreen = convertToScreen(hotspot).findIntersectingRect(_screenPosition);
		if (hotspotOnScreen.contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

			// Highlight the selected response
			if (g_nancy->getGameType() >= kGameTypeNancy2) {
				_highlightRObj.setVisible(true);
				Common::Rect hotspotInside = convertToLocal(hotspotOnScreen);
				hotspotInside.translate(0, _drawSurface.getOffsetFromOwner().y);
				_highlightRObj._drawSurface.create(_textHighlightSurface, hotspotInside);
				_highlightRObj.moveTo(hotspotOnScreen);
				hasHighlight = true;
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.input &= ~NancyInput::kLeftMouseButtonUp;
				NancySceneState.clearLogicConditions();
				NancySceneState.setLogicCondition(i);
			}

			break;
		}
	}

	if (!hasHighlight && _highlightRObj.isVisible()) {
		_highlightRObj.setVisible(false);
	}
}

void Textbox::drawTextbox() {
	using namespace Common;

	TBOX *tbox = g_nancy->_textboxData;
	assert(tbox);

	_numLines = 0;

	const Font *font = g_nancy->_graphicsManager->getFont(_fontIDOverride == -1 ? tbox->conversationFontID : _fontIDOverride);
	const Font *highlightFont = g_nancy->_graphicsManager->getFont(tbox->highlightConversationFontID);

	uint maxWidth = _fullSurface.w - tbox->maxWidthDifference - tbox->borderWidth - 2;
	uint lineDist = tbox->lineHeight + tbox->lineHeight / 4;

	for (uint lineID = 0; lineID < _textLines.size(); ++lineID) {
		Common::String currentLine = _textLines[lineID];

		uint horizontalOffset = 0;
		bool hasHotspot = false;
		Rect hotspot;

		// Erase the begin and end tokens from the line
		uint32 newLinePos;
		while (newLinePos = currentLine.find(_CCBeginToken), newLinePos != String::npos) {
			currentLine.erase(newLinePos, ARRAYSIZE(_CCBeginToken) - 1);
		}

		while (newLinePos = currentLine.find(_CCEndToken), newLinePos != String::npos) {
			currentLine.erase(newLinePos, ARRAYSIZE(_CCEndToken) - 1);
		}

		// Replace every newline token with \n
		while (newLinePos = currentLine.find(_newLineToken), newLinePos != String::npos) {
			currentLine.replace(newLinePos, ARRAYSIZE(_newLineToken) - 1, "\n");
		}

		// Simply remove telephone end token
		if (currentLine.hasSuffix(_telephoneEndToken)) {
			currentLine = currentLine.substr(0, currentLine.size() - ARRAYSIZE(_telephoneEndToken) + 1);
		}

		// Remove hotspot tokens and mark that we need to calculate the bounds
		// A single text line should only have one hotspot, but there's at least
		// one malformed line in TVD that breaks this
		uint32 hotspotPos, lastHotspotPos = 0;
		while (hotspotPos = currentLine.find(_hotspotToken), hotspotPos != String::npos) {
			currentLine.erase(hotspotPos, ARRAYSIZE(_hotspotToken) - 1);

			if (hasHotspot) {
				// Replace the second hotspot token with a newline to copy the original behavior
				// Maybe consider fixing the glitch instead of replicating it??
				currentLine.insertChar('\n', lastHotspotPos);
			}

			hasHotspot = true;
			lastHotspotPos = hotspotPos;
		}

		// Subdivide current line into sublines for proper handling of the tab and color tokens
		// Assumes the tab token is on a new line
		while (!currentLine.empty()) {
			if (currentLine.hasPrefix(_tabToken)) {
				horizontalOffset += font->getStringWidth("    "); // Replace tab with 4 spaces
				currentLine = currentLine.substr(ARRAYSIZE(_tabToken) - 1);
			}

			String currentSubstring;
			_lastResponseisMultiline = false;

			uint32 nextTabPos = currentLine.find(_tabToken);
			if (nextTabPos != String::npos) {
				currentSubstring = currentLine.substr(0, nextTabPos);
				currentLine = currentLine.substr(nextTabPos);
			} else {
				currentSubstring = currentLine;
				currentLine.clear();
			}
			
			Array<Common::String> wrappedLines;
			uint numColorLines = 0;
			uint colorLinesWidth = 0;

			// Color token denotes a highlighted section of the dialogue
			// The russian variant of nancy1 makes all player text colored, so we need to do font wrapping here
			if (currentSubstring.hasPrefix(_colorBeginToken)) {
				// Found color string, look for end token
				uint32 colorEndPos = currentSubstring.find(_colorEndToken);

				Common::String colorSubstring = currentSubstring.substr(ARRAYSIZE(_colorBeginToken) - 1, colorEndPos - ARRAYSIZE(_colorBeginToken) + 1);
				currentSubstring = currentSubstring.substr(ARRAYSIZE(_colorBeginToken) + ARRAYSIZE(_colorEndToken) + colorSubstring.size() - 2);

				font->wordWrap(colorSubstring, maxWidth, wrappedLines, 0);

				// Draw the color lines
				for (uint i = 0; i < wrappedLines.size(); ++i) {
					font->drawString(&_fullSurface, wrappedLines[i], tbox->borderWidth + horizontalOffset, tbox->firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 1);
					colorLinesWidth = MAX<int16>(colorLinesWidth, font->getStringWidth(wrappedLines[i]));
					if (i != wrappedLines.size() - 1) {
						++_numLines;
						++numColorLines;
					}
				}
				
				horizontalOffset += font->getStringWidth(wrappedLines.back());
				wrappedLines.clear();
			}

			// Do word wrapping on the rest of the text
			font->wordWrap(currentSubstring, maxWidth, wrappedLines, horizontalOffset);

			if (hasHotspot) {
				hotspot.left = tbox->borderWidth;
				hotspot.top = tbox->firstLineOffset - tbox->lineHeight + (_numLines - numColorLines) * lineDist - 1;
				hotspot.setHeight((wrappedLines.size() + MAX<int16>((numColorLines - 1), 0)) * lineDist - (lineDist - tbox->lineHeight));
				hotspot.setWidth(0);
			}

			// Draw the wrapped lines
			for (uint i = 0; i < wrappedLines.size(); ++i) {
				font->drawString(&_fullSurface, wrappedLines[i], tbox->borderWidth + (i == 0 ? horizontalOffset : 0), tbox->firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 0);
				if (hasHotspot) {
					highlightFont->drawString(&_textHighlightSurface, wrappedLines[i], tbox->borderWidth + (i == 0 ? horizontalOffset : 0), tbox->firstLineOffset - highlightFont->getFontHeight() + _numLines * lineDist, maxWidth, 0);
					hotspot.setWidth(MAX<int16>(hotspot.width(), font->getStringWidth(wrappedLines[i]) + (i == 0 ? horizontalOffset : 0)));
				}
				++_numLines;
			}

			// Make sure to adjust the hotspot width if we had a multi-line color string
			hotspot.setWidth(MAX<int16>(hotspot.width(), colorLinesWidth));

			// Simulate a bug in the original engine where player text longer than
			// a single line gets a double newline afterwards
			if ((wrappedLines.size() + numColorLines) > 1 && hasHotspot) {
				++_numLines;
				_lastResponseisMultiline = true;
			}

			horizontalOffset = 0;
		}

		// Add the hotspot to the list
		if (hasHotspot) {
			_hotspots.push_back(hotspot);
		}

		// Add a new line after every text line
		++_numLines;
	}

	setVisible(true);
	_needsTextRedraw = false;
}

void Textbox::clear() {
	_fullSurface.clear();
	_textHighlightSurface.clear(_textHighlightSurface.getTransparentColor());
	_textLines.clear();
	_hotspots.clear();
	_scrollbar->resetPosition();
	_numLines = 0;
	_fontIDOverride = -1;
	onScrollbarMove();
	_needsRedraw = true;
}

void Textbox::addTextLine(const Common::String &text) {
	// Scan for the hotspot token and assume the text is the main text if not found
	_textLines.push_back(text);

	_needsTextRedraw = true;
}

// A text line will often be broken up into chunks separated by nulls, use
// this function to put it back together as a Common::String
void Textbox::assembleTextLine(char *rawCaption, Common::String &output, uint size) {
	for (uint i = 0; i < size; ++i) {
		// A single line can be broken up into bits, look for them and
		// concatenate them when we're done
		if (rawCaption[i] != 0) {
			Common::String newBit(rawCaption + i);
			output += newBit;
			i += newBit.size();
		}
	}

	// Fix spaces at the end of the string in nancy1
	output.trim();

	// Scan the text line for doubly-closed tokens; happens in some strings in The Vampire Diaries
	uint pos = Common::String::npos;
	while (pos = output.find(">>"), pos != Common::String::npos) {
		output.replace(pos, 2, ">");
	}
}

void Textbox::onScrollbarMove() {
	_scrollbarPos = CLIP<float>(_scrollbarPos, 0, 1);

	uint16 inner = getInnerHeight();
	uint16 outer = _screenPosition.height();

	if (inner > outer) {
		Common::Rect bounds = getBounds();
		bounds.moveTo(0, (inner - outer) * _scrollbarPos);
		_drawSurface.create(_fullSurface, bounds);
		_highlightRObj._drawSurface.create(_textHighlightSurface, bounds);
	} else {
		_drawSurface.create(_fullSurface, getBounds());
		_highlightRObj._drawSurface.create(_textHighlightSurface, getBounds());
	}

	_needsRedraw = true;
}

uint16 Textbox::getInnerHeight() const {
	TBOX *tbox = g_nancy->_textboxData;
	assert(tbox);
	
	// These calculations are _almost_ correct, but off by a pixel sometimes
	uint lineDist = tbox->lineHeight + tbox->lineHeight / 4;
	if (g_nancy->getGameType() == kGameTypeVampire) {
		return _numLines * lineDist + tbox->firstLineOffset + (_lastResponseisMultiline ? - tbox->lineHeight / 2 : 1);
	} else {
		return _numLines * lineDist + tbox->firstLineOffset + lineDist / 2 - 1;
	}
}

} // End of namespace UI
} // End of namespace Nancy
