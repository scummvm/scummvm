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
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/textbox.h"
#include "engines/nancy/ui/scrollbar.h"

namespace Nancy {
namespace UI {

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
				NancySceneState.setLogicCondition(i, g_nancy->_true);
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

	uint maxWidth = _fullSurface.w - tbox->maxWidthDifference - tbox->borderWidth - 2;
	uint lineDist = tbox->lineHeight + tbox->lineHeight / 4;

	for (uint lineID = 0; lineID < _textLines.size(); ++lineID) {
		Common::String currentLine;
		bool hasHotspot = false;
		Rect hotspot;
		Common::Queue<uint> colorTokens;
		int fontID = _fontIDOverride;

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
					currentLine += "    ";
					continue;
				case 'c' :
					// Color tokens
					// We keep the positions of the color tokens in a queue
					if (curToken.size() != 2) {
						break;
					}

					if (curToken[1] == '0' && colorTokens.size() == 0) {
						// Found a color end token ("c0") without a corresponding begin ("c1"),
						// insert a fake one at the beginning of the queue to make the color logic work.
						// This happens in nancy5's intro
						colorTokens.push(0);
					}

					if (curToken[1] == '1' && colorTokens.size() % 2 == 1) {
						// Found a color begin token ("c1") following another color begin token.
						// This is invalid, so we just skip it
						// This probably also happens somewhere
						continue;
					}

					if (curToken[1] == '0' && colorTokens.size() % 2 == 0) {
						// Found a color end token ("c0") following another color end token.
						// This is invalid, so we just skip it
						// This happens in nancy5's intro
						continue;
					}
					
					colorTokens.push(currentLine.size());
					continue;
				case 'f' :
					// Font token
					// This selects a specific font ID for the current line
					if (curToken.size() != 2) {
						break;
					}

					fontID = (int)Common::String(curToken[1]).asUint64();

					continue;
				}
			}

			currentLine += curToken;
		}
		
		const Font *font = g_nancy->_graphicsManager->getFont(fontID == -1 ? tbox->conversationFontID : fontID);
		const Font *highlightFont = g_nancy->_graphicsManager->getFont(tbox->highlightConversationFontID);

		// Do word wrapping on the text, sans tokens
		Array<Common::String> wrappedLines;
		font->wordWrap(currentLine, maxWidth, wrappedLines, 0);

		// Setup most of the hotspot
		if (hasHotspot) {
			hotspot.left = tbox->borderWidth;
			hotspot.top = tbox->firstLineOffset - tbox->lineHeight + (_numLines * lineDist) - 1;
			hotspot.setHeight((wrappedLines.size() * lineDist) - (lineDist - tbox->lineHeight));
			hotspot.setWidth(0);
		}

		// Go through the wrapped lines and draw them, making sure to
		// respect color tokens
		uint totalCharsDrawn = 0;
		bool isColor = false;
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

				if (colorTokens.size()) {
					// Text contains color part

					if (totalCharsDrawn >= colorTokens.front()) {
						// Token is at begginning of (what's left of) the current line
						uint val = colorTokens.pop();

						if (!colorTokens.empty() && colorTokens.front() == val) {
							// Two tokens with the same position just get ignored
							colorTokens.pop();
						} else {
							isColor = !isColor;
						}
					}

					if (totalCharsDrawn < colorTokens.front() && colorTokens.front() < (totalCharsDrawn + line.size())) {
						// There's a token inside the current line, so split off the part before it
						subLine = line.substr(0, colorTokens.front() - totalCharsDrawn);
						line = line.substr(subLine.size());
					}
				}

				// Choose whether to draw the subLine, or the full line
				Common::String &stringToDraw = subLine.size() ? subLine : line;

				// Draw the normal text
				font->drawString(				&_fullSurface,
												stringToDraw,
												tbox->borderWidth + horizontalOffset,
												tbox->firstLineOffset - font->getFontHeight() + _numLines * lineDist,
												maxWidth,
												isColor);

				// Then, draw the highlight
				if (hasHotspot) {
					highlightFont->drawString(	&_textHighlightSurface,
												stringToDraw,
												tbox->borderWidth + horizontalOffset,
												tbox->firstLineOffset - font->getFontHeight() + _numLines * lineDist,
												maxWidth,
												isColor);
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
			++_numLines;
		}

		// Add the hotspot to the list
		if (hasHotspot) {
			_hotspots.push_back(hotspot);
		}

		// Simulate a bug in the original engine where player text longer than
		// a single line gets a double newline afterwards
		if (wrappedLines.size() > 1 && hasHotspot) {
			++_numLines;

			if (lineID == _textLines.size() - 1) {
				_lastResponseisMultiline = true;
			}
		}

		// Add a newline after every full piece of text
		++_numLines;
	}

	setVisible(true);
	_needsTextRedraw = false;
}

void Textbox::clear() {
	if (_textLines.size()) {
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
