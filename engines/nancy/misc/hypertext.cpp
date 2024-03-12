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
#include "engines/nancy/resource.h"

#include "engines/nancy/misc/hypertext.h"

namespace Nancy {
namespace Misc {

struct MetaInfo {
	enum Type { kColor, kFont, kMark, kHotspot };

	Type type;
	uint numChars;
	byte index;
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

void HypertextParser::addImage(uint16 lineID, const Common::Rect &src) {
	_imageLineIDs.push_back(lineID);
	_imageSrcs.push_back(src);
}

void HypertextParser::setImageName(const Common::Path &name) {
	_imageName = name;
}

void HypertextParser::drawAllText(const Common::Rect &textBounds, uint leftOffsetNonNewline, uint fontID, uint highlightFontID) {
	using namespace Common;

	const Font *font = nullptr;
	const Font *highlightFont = nullptr;
	Graphics::ManagedSurface image;

	_numDrawnLines = 0;

	if (!_imageName.empty()) {
		g_nancy->_resource->loadImage(_imageName, image);
	}

	for (uint lineID = 0; lineID < _textLines.size(); ++lineID) {
		Common::String currentLine;
		bool hasHotspot = false;
		Rect hotspot;
		Common::Queue<MetaInfo> metaInfo;
		Common::Queue<uint16> newlineTokens;
		newlineTokens.push(0);
		int curFontID = fontID;
		uint numNonSpaceChars = 0;

		// Token braces plus invalid characters that are known to appear in strings
		Common::StringTokenizer tokenizer(_textLines[lineID], "<>\"");

		Common::String curToken;
		bool reachedEndTag = false;
		while(!tokenizer.empty() && !reachedEndTag) {
			curToken = tokenizer.nextToken();

			if (tokenizer.delimitersAtTokenBegin().lastChar() == '<' && tokenizer.delimitersAtTokenEnd().firstChar() == '>') {
				switch (curToken.firstChar()) {
				case 'i' :
					// CC begin
					// fall through
				case 'o' :
					// CC end
					if (curToken.size() != 1) {
						break;
					}

					continue;
				case 'e' :
					// End conversation. Originally used for quickly ending dialogue when debugging, but
					// also marks the ending of the current text line.
					if (curToken.size() != 1) {
						break;
					}

					// Ignore the rest of the text. This fixes nancy7 scene 5770
					reachedEndTag = true;
					continue;
				case 'h' :
					// Hotspot
					if (curToken.size() != 1) {
						break;
					}

					if (hasHotspot) {
						// Replace duplicate hotspot token with a newline to copy the original behavior
						currentLine += '\n';
					}

					hasHotspot = true;
					continue;
				case 'H' :
					// Hotspot inside list, begin
					if (curToken.size() != 1) {
						break;
					}

					metaInfo.push({MetaInfo::kHotspot, numNonSpaceChars, 1});
					continue;
				case 'L' :
					// Hotspot inside list, end
					if (curToken.size() != 1) {
						break;
					}

					metaInfo.push({MetaInfo::kHotspot, numNonSpaceChars, 0});
					continue;
				case 'n' :
					// Newline
					if (curToken.size() != 1) {
						break;
					}

					currentLine += '\n';
					newlineTokens.push(numNonSpaceChars);
					continue;
				case 't' :
					// Tab
					if (curToken.size() != 1) {
						break;
					}

					currentLine += '\t';
					continue;
				case 'c' :
					// Color tokens
					// We keep the positions (excluding spaces) and colors of the color tokens in a queue
					if (curToken.size() != 2) {
						break;
					}

					metaInfo.push({MetaInfo::kColor, numNonSpaceChars, (byte)(curToken[1] - '0')});
					continue;
				case 'f' :
					// Font token
					// This selects a specific font ID for the following text
					if (curToken.size() != 2) {
						break;
					}

					metaInfo.push({MetaInfo::kFont, numNonSpaceChars, (byte)(curToken[1] - '0')});
					continue;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
					if (curToken.size() != 1) {
						break;
					}

					metaInfo.push({MetaInfo::kMark, numNonSpaceChars, (byte)(curToken[0] - '1')});
					continue;
				default:
					break;
				}

				// Ignore non-tokens when they're between braces. This fixes nancy6 scenes 1953 & 1954,
				// where some sound names slipped through into the text data.
				debugC(Nancy::kDebugHypertext, "Unrecognized hypertext tag <%s>", curToken.c_str());
				continue;
			}

			// Count the number of non-space characters. We use this to keep track
			// of where color changes should happen, since including whitespaces
			// presents a lot of edge cases when combined with word wrapping
			for (uint i = 0; i < curToken.size(); ++i) {
				if (curToken[i] != ' ') {
					++numNonSpaceChars;
				}
			}

			currentLine += curToken;
		}

		font = g_nancy->_graphics->getFont(curFontID);
		highlightFont = g_nancy->_graphics->getFont(highlightFontID);
		assert(font && highlightFont);

		// Do word wrapping on the text, sans tokens. This assumes
		// all text uses fonts of the same width
		Array<Common::String> wrappedLines;
		font->wordWrap(currentLine, textBounds.width(), wrappedLines, 0);

		// Setup most of the hotspot; textbox
		if (hasHotspot) {
			hotspot.left = textBounds.left;
			hotspot.top = textBounds.top + (_numDrawnLines * font->getFontHeight()) - 1;
			hotspot.setHeight(0);
			hotspot.setWidth(0);
		}

		// Go through the wrapped lines and draw them, making sure to
		// respect color tokens
		uint totalCharsDrawn = 0;
		byte colorID = _defaultTextColor;
		uint numNewlineTokens = 0;
		uint horizontalOffset = 0;
		bool newLineStart = false;
		for (uint lineNumber = 0; lineNumber < wrappedLines.size(); ++lineNumber) {
			Common::String &line = wrappedLines[lineNumber];
			horizontalOffset = 0;
			newLineStart = false;
			// Draw images
			if (newlineTokens.front() <= totalCharsDrawn) {
				newlineTokens.pop();
				newLineStart = true;

				for (uint i = 0; i < _imageLineIDs.size(); ++i) {
					if (numNewlineTokens == _imageLineIDs[i]) {
						// A lot of magic numbers that make sure we draw pixel-perfect. This is a mess for three reasons:
						// - The original engine draws strings with a bottom-left anchor, while ScummVM uses top-left
						// - The original engine uses inclusive rects, while ScummVM uses non-includive
						// - The original engine does some stupid stuff with spacing
						// This works correctly in nancy7, but might fail with different games/fonts
						if (lineNumber != 0) {
							_imageVerticalOffset += (font->getFontHeight() + 1) / 2 + 1;
						}

						_fullSurface.blitFrom(image, _imageSrcs[i],
							Common::Point(	textBounds.left + horizontalOffset + 1,
											textBounds.top + _numDrawnLines * highlightFont->getFontHeight() + _imageVerticalOffset));
						_imageVerticalOffset += _imageSrcs[i].height() - 1;

						if (lineNumber == 0) {
							_imageVerticalOffset += font->getFontHeight() / 2 - 1;
						} else {
							_imageVerticalOffset += (font->getFontHeight() + 1) / 2 + 3;
						}
					}
				}

				++numNewlineTokens;
			}

			// Trim whitespaces (only) at beginning and end of wrapped lines
			while (line.lastChar() == ' ') {
				line.deleteLastChar();
			}

			while (line.firstChar() == ' ') {
				line.deleteChar(0);
			}

			bool newWrappedLine = true; // Used to ensure color/font changes don't mess up hotspots
			while (!line.empty()) {
				Common::String subLine;

				while (metaInfo.size() && totalCharsDrawn >= metaInfo.front().numChars) {
					// We have a color/font change token, a hyperlink, or a mark at begginning of (what's left of) the current line
					MetaInfo change = metaInfo.pop();
					switch (change.type) {
					case MetaInfo::kFont:
						curFontID = change.index;
						font = g_nancy->_graphics->getFont(curFontID);
						break;
					case MetaInfo::kColor:
						colorID = change.index;
						break;
					case MetaInfo::kMark: {
						auto *mark = GetEngineData(MARK);
						assert(mark);

						if (lineNumber == 0) {
							// A mark on the first line pushes up all text
							if (textBounds.top - _imageVerticalOffset > 3) {
								_imageVerticalOffset -= 3;
							} else {
								_imageVerticalOffset = -textBounds.top;
							}
						}

						Common::Rect markSrc = mark->_markSrcs[change.index];
						Common::Rect markDest = markSrc;
						markDest.moveTo(textBounds.left + horizontalOffset + (newLineStart ? 0 : leftOffsetNonNewline) + 1,
							lineNumber == 0 ?
								textBounds.top - ((font->getFontHeight() + 1) / 2) + _imageVerticalOffset + 4 :
								textBounds.top + _numDrawnLines * font->getFontHeight() + _imageVerticalOffset - 4);

						// For now we do not check if we need to go to new line; neither does the original
						_fullSurface.blitFrom(g_nancy->_graphics->_object0, markSrc, markDest);

						horizontalOffset += markDest.width() + 2;
						break;
					}
					case MetaInfo::kHotspot:
						// List only
						hasHotspot = change.index;

						if (hasHotspot) {
							hotspot.left = textBounds.left + (newLineStart ? 0 : horizontalOffset + leftOffsetNonNewline);
							hotspot.top = textBounds.top + _numDrawnLines * font->getFontHeight() + _imageVerticalOffset - 1;
							hotspot.setHeight(0);
							hotspot.setWidth(0);
						} else {
							_hotspots.push_back(hotspot);
							hotspot = { 0, 0, 0, 0 };
						}

						break;
					}
				}

				uint lineSizeNoSpace = 0;
				for (uint i = 0; i < line.size(); ++i) {
					if (!isSpace(line[i])) {
						++lineSizeNoSpace;
					}
				}

				if (metaInfo.size() && totalCharsDrawn < metaInfo.front().numChars && metaInfo.front().numChars <= (totalCharsDrawn + lineSizeNoSpace)) {
					// There's a token inside the current line, so split off the part before it
					uint subSize = metaInfo.front().numChars - totalCharsDrawn;
					for (uint i = 0; i < subSize; ++i) {
						if (isSpace(line[i])) {
							++subSize;
						}
					}
					subLine = line.substr(0, subSize);
					line = line.substr(subLine.size());
				}

				// Choose whether to draw the subLine, or the full line
				Common::String &stringToDraw = subLine.size() ? subLine : line;

				// Draw the normal text
				font->drawString(				&_fullSurface,
												stringToDraw,
												textBounds.left + horizontalOffset + (newLineStart ? 0 : leftOffsetNonNewline),
												textBounds.top + _numDrawnLines * font->getFontHeight() + _imageVerticalOffset,
												textBounds.width(),
												colorID);

				// Then, draw the highlight
				if (hasHotspot && !_textHighlightSurface.empty()) {
					highlightFont->drawString(	&_textHighlightSurface,
												stringToDraw,
												textBounds.left + horizontalOffset + (newLineStart ? leftOffsetNonNewline : 0),
												textBounds.top + _numDrawnLines * highlightFont->getFontHeight() + _imageVerticalOffset,
												textBounds.width(),
												colorID);
				}

				// Count number of non-space characters drawn. Used for color.
				// Note that we use isSpace() specifically to exclude the tab character
				for (uint i = 0; i < stringToDraw.size(); ++i) {
					if (!isSpace(stringToDraw[i])) {
						++totalCharsDrawn;
					}
				}

				// Add to the width/height of the hotspot
				if (hasHotspot) {
					hotspot.setWidth(MAX<int16>(hotspot.width(), font->getStringWidth(stringToDraw)));

					if (!stringToDraw.empty() && newWrappedLine) {
						hotspot.setHeight(hotspot.height() + font->getFontHeight());
					}
				}

				newWrappedLine = false;

				if (subLine.size()) {
					horizontalOffset += font->getStringWidth(subLine);
				} else {
					break;
				}
			}

			++_numDrawnLines;

			// Record the height of the text currently drawn. Used for textbox scrolling
			_drawnTextHeight = (_numDrawnLines - 1) * font->getFontHeight() + _imageVerticalOffset;
		}

		// Draw the footer image(s)
		for (uint i = 0; i < _imageLineIDs.size(); ++i) {
			if (numNewlineTokens <= _imageLineIDs[i]) {
				_imageVerticalOffset += (font->getFontHeight() + 1) / 2 + 1;

				_fullSurface.blitFrom(image, _imageSrcs[i],
					Common::Point(	textBounds.left + horizontalOffset + 1,
									textBounds.top + _numDrawnLines * highlightFont->getFontHeight() + _imageVerticalOffset));
				_imageVerticalOffset += _imageSrcs[i].height() - 1;

				if (i < _imageLineIDs.size() - 1) {
					_imageVerticalOffset += (font->getFontHeight() + 1) / 2 + 3;
				}

				_drawnTextHeight = (_numDrawnLines - 1) * font->getFontHeight() + _imageVerticalOffset;
			}
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
		_drawnTextHeight += font->getFontHeight();
	}

	// Add a line's height at end of text to replicate original behavior
	if (font) {
		_drawnTextHeight += font->getFontHeight();
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
