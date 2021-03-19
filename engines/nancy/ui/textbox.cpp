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
 */

#include "engines/nancy/ui/textbox.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"

#include "common/error.h"
#include "common/util.h"
#include "common/events.h"
#include "common/util.h"

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

void Textbox::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("TBOX");
	chunk->seek(0);
	readRect(*chunk, _scrollbarSourceBounds);

	chunk->seek(0x20);
	Common::Rect innerBoundingBox;
	readRect(*chunk, innerBoundingBox);
	_fullSurface.create(innerBoundingBox.width(), innerBoundingBox.height(), GraphicsManager::_screenPixelFormat);

	_scrollbarDefaultDest.x = chunk->readUint16LE();
	_scrollbarDefaultDest.y = chunk->readUint16LE();

	chunk->seek(0x36);
	_firstLineOffset = chunk->readUint16LE();
	_lineHeight = chunk->readUint16LE();
	// Not sure why but to get exact results we subtract 1
	_borderWidth = chunk->readUint16LE() - 1;

	chunk->seek(0x1FE, SEEK_SET);
	_fontID = chunk->readUint16LE();

	chunk = g_nancy->getBootChunkStream("BSUM");
	chunk->seek(0x164);
	readRect(*chunk, _screenPosition);

	Common::Rect outerBoundingBox = _screenPosition;
	outerBoundingBox.moveTo(0, 0);
	_drawSurface.create(_fullSurface, outerBoundingBox);

	RenderObject::init();

	_scrollbar.init();
}

void Textbox::registerGraphics() {
	RenderObject::registerGraphics();
	_scrollbar.registerGraphics();
}

void Textbox::updateGraphics() {
	if (_needsTextRedraw) {
		drawTextbox();
	}

	if (_scrollbarPos != _scrollbar.getPos()) {
		_scrollbarPos = _scrollbar.getPos();

		onScrollbarMove();
	}

	RenderObject::updateGraphics();
}

void Textbox::handleInput(NancyInput &input) {
	_scrollbar.handleInput(input);

	for (uint i = 0; i < _hotspots.size(); ++i) {
		Common::Rect hotspot = _hotspots[i];
		hotspot.translate(0, -_drawSurface.getOffsetFromOwner().y);
		if (convertToScreen(hotspot).findIntersectingRect(_screenPosition).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.input &= ~NancyInput::kLeftMouseButtonUp;
				NancySceneState.clearLogicConditions();
				NancySceneState.setLogicCondition(i);
			}

			break;
		}
	}
}

void Textbox::drawTextbox() {
	using namespace Common;

	_numLines = 0;

	Font *font = g_nancy->_graphicsManager->getFont(_fontID);

	uint maxWidth = _fullSurface.w - _borderWidth * 2;
	uint lineDist = _lineHeight + _lineHeight / 4;

	for (uint lineID = 0; lineID < _textLines.size(); ++lineID) {
		Common::String currentLine = _textLines[lineID];
		currentLine.trim();

		uint horizontalOffset = 0;
		bool hasHotspot = false;
		Rect hotspot;

		// Trim the begin and end tokens from the line
		if (currentLine.hasPrefix(_CCBeginToken) && currentLine.hasSuffix(_CCEndToken)) {
			currentLine = currentLine.substr(ARRAYSIZE(_CCBeginToken) - 1, currentLine.size() - ARRAYSIZE(_CCBeginToken) - ARRAYSIZE(_CCEndToken) + 2);
		}

		// Replace every newline token with \n
		uint32 newLinePos;
		while (newLinePos = currentLine.find(_newLineToken), newLinePos != String::npos) {
			currentLine.replace(newLinePos, ARRAYSIZE(_newLineToken) - 1, "\n");
		}

		// Simply remove telephone end token
		if (currentLine.hasSuffix(_telephoneEndToken)) {
			currentLine = currentLine.substr(0, currentLine.size() - ARRAYSIZE(_telephoneEndToken) + 1);
		}

		// Remove hotspot token and mark that we need to calculate the bounds
		// Assumes a single text line has a single hotspot
		uint32 hotspotPos = currentLine.find(_hotspotToken);
		if (hotspotPos != String::npos) {
			currentLine.erase(hotspotPos, ARRAYSIZE(_hotspotToken) - 1);
			hasHotspot = true;
		}

		// Subdivide current line into sublines for proper handling of the tab and color tokens
		// Assumes the tab token is on a new line
		while (!currentLine.empty()) {
			if (currentLine.hasPrefix(_tabToken)) {
				horizontalOffset += font->getStringWidth("    "); // Replace tab with 4 spaces
				currentLine = currentLine.substr(ARRAYSIZE(_tabToken) - 1);
			}

			String currentSubLine;

			uint32 nextTabPos = currentLine.find(_tabToken);
			if (nextTabPos != String::npos) {
				currentSubLine = currentLine.substr(0, nextTabPos);
				currentLine = currentLine.substr(nextTabPos);
			} else {
				currentSubLine = currentLine;
				currentLine.clear();
			}

			// Assumes color token will be at the beginning of the line, and color string will not need wrapping
			if (currentSubLine.hasPrefix(_colorBeginToken)) {
				// Found color string, look for end token
				uint32 colorEndPos = currentSubLine.find(_colorEndToken);

				Common::String colorSubLine = currentSubLine.substr(ARRAYSIZE(_colorBeginToken) - 1, colorEndPos - ARRAYSIZE(_colorBeginToken) + 1);
				currentSubLine = currentSubLine.substr(ARRAYSIZE(_colorBeginToken) + ARRAYSIZE(_colorEndToken) + colorSubLine.size() - 2);

				// Draw the color line
				font->drawString(&_fullSurface, colorSubLine, _borderWidth + horizontalOffset, _firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 1);
				horizontalOffset += font->getStringWidth(colorSubLine);
			}

			Array<Common::String> wrappedLines;

			// Do word wrapping on the rest of the text
			font->wordWrapText(currentSubLine, maxWidth, wrappedLines, horizontalOffset);

			if (hasHotspot) {
				hotspot.left = _borderWidth;
				hotspot.top = _firstLineOffset - font->getFontHeight() + (_numLines + 1) * lineDist;
				hotspot.setHeight((wrappedLines.size() - 1) * lineDist + _lineHeight);
				hotspot.setWidth(0);
			}

			// Draw the wrapped lines
			for (uint i = 0; i < wrappedLines.size(); ++i) {
				font->drawString(&_fullSurface, wrappedLines[i], _borderWidth + (i == 0 ? horizontalOffset : 0), _firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 0);
				if (hasHotspot) {
					hotspot.setWidth(MAX<int16>(hotspot.width(), font->getStringWidth(wrappedLines[i]) + (i == 0 ? horizontalOffset : 0)));
				}
				++_numLines;
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
	_textLines.clear();
	_hotspots.clear();
	_scrollbar.resetPosition();
	_numLines = 0;
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
}

void Textbox::onScrollbarMove() {
	_scrollbarPos = CLIP<float>(_scrollbarPos, 0, 1);

	uint16 inner = getInnerHeight();
	uint16 outer = _screenPosition.height();

	if (inner > outer) {
		Common::Rect bounds = getBounds();
		bounds.moveTo(0, (inner - outer) * _scrollbarPos);
		_drawSurface.create(_fullSurface, bounds);
	} else {
		_drawSurface.create(_fullSurface, getBounds());
	}

	_needsRedraw = true;
}

uint16 Textbox::getInnerHeight() const {
	uint lineDist = _lineHeight + _lineHeight / 4;
	return _numLines * lineDist + _firstLineOffset + lineDist / 2;
}

void Textbox::TextboxScrollbar::init() {
	Common::Rect &srcBounds = _parent->_scrollbarSourceBounds;
	Common::Point &topPosition = _parent->_scrollbarDefaultDest;

	_drawSurface.create(g_nancy->_graphicsManager->_object0, srcBounds);

	_startPosition = topPosition;
	_startPosition.x -= srcBounds.width() / 2;

	_screenPosition = srcBounds;
	_screenPosition.moveTo(_startPosition);

	_maxDist = _parent->getBounds().height() - _drawSurface.h;

	Scrollbar::init();
}

} // End of namespace UI
} // End of namespace Nancy
