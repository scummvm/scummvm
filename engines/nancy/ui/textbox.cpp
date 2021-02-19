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

namespace Nancy {
namespace UI {

const Common::String Textbox::beginToken = Common::String("<i>");
const Common::String Textbox::endToken = Common::String("<o>");
const Common::String Textbox::colorBeginToken = Common::String("<c1>");
const Common::String Textbox::colorEndToken = Common::String("<c0>");
const Common::String Textbox::hotspotToken = Common::String("<h>");
const Common::String Textbox::newLineToken = Common::String("<n>");

void Textbox::init() {    
    Common::SeekableReadStream *chunk = _engine->getBootChunkStream("TBOX");
    chunk->seek(0);
    readRect(*chunk, _scrollbarSourceBounds);

    chunk->seek(0x20);
    Common::Rect innerBoundingBox;
    readRect(*chunk, innerBoundingBox);
    _fullSurface.create(innerBoundingBox.width(), innerBoundingBox.height(), GraphicsManager::pixelFormat);
    
    _scrollbarDefaultDest.x = chunk->readUint16LE();
    _scrollbarDefaultDest.y = chunk->readUint16LE();

    chunk->seek(0x36);
    _firstLineOffset = chunk->readUint16LE();
    _lineHeight = chunk->readUint16LE();
    // Not sure why but to get exact results we subtract 1
    _borderWidth = chunk->readUint16LE() - 1;

    chunk = _engine->getBootChunkStream("BSUM");
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

    for (uint i = 0; i < _responses.size(); ++i) {
        Common::Rect hotspot = _responses[i].hotspot;
        hotspot.translate(0, -_drawSurface.getOffsetFromOwner().y);
        if (convertToScreen(hotspot).findIntersectingRect(_screenPosition).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(CursorManager::kHotspotArrow);

            if (input.input & NancyInput::kLeftMouseButtonUp) {
                input.input &= ~NancyInput::kLeftMouseButtonUp;
                _engine->scene->clearLogicConditions();
                _engine->scene->setLogicCondition(i);
            }
            
            break;
        }
    }
}

void Textbox::drawTextbox() {
    Common::Array<Common::String> wrappedLines;
    Common::String tokenlessString;
    _numLines = 0;
    // Hardcode to 1 until I figure out the proper logic
    Font *font = _engine->graphicsManager->getFont(1);

    uint maxWidth = _fullSurface.w - _borderWidth;
    uint lineDist = _lineHeight + _lineHeight / 4;

    Common::String line;

    _mainString.trim();

    // Scan for and remove tokens from main string
    if (_mainString.hasPrefix(beginToken) && _mainString.hasSuffix(endToken)) {
        tokenlessString = _mainString.substr(beginToken.size(), _mainString.size() - beginToken.size() - endToken.size());
        if (tokenlessString.hasSuffix(newLineToken)) {
            tokenlessString = tokenlessString.substr(0, tokenlessString.size() - newLineToken.size());
        }
    }

    // Wrap text for main string
    font->wordWrapText(tokenlessString, maxWidth, wrappedLines);

    // Draw main string
    for (auto &str : wrappedLines) {
        font->drawString(&_fullSurface, str, _borderWidth, _firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 0);
        ++_numLines;
    }

    for (auto &res : _responses) {
        // Scan for tokens
        ++_numLines;
        bool newLineAtEnd = false;
        uint colorCharOffset = 0;
        uint colorPixelOffset = 0;

        res.text.trim();

        tokenlessString.clear();
        wrappedLines.clear();

        if (res.text.hasPrefix(colorBeginToken)) {
            // Create a substring with all (usually just one) of the colored characters
            for (uint i = colorBeginToken.size(); res.text.substr(i, colorEndToken.size()) != colorEndToken; ++i) {
                tokenlessString += res.text[i];
            }

            // Draw the color string
            font->drawString(&_fullSurface, tokenlessString, _borderWidth, _firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 1);
            colorCharOffset = tokenlessString.size() + colorBeginToken.size() + colorEndToken.size();
            colorPixelOffset = font->getStringWidth(tokenlessString);
        }

        tokenlessString = res.text.substr(colorCharOffset, res.text.size() - colorCharOffset);


        // Remove the new line token and add a line break
        if (tokenlessString.hasSuffix(newLineToken)) {
            newLineAtEnd = true;
            tokenlessString = tokenlessString.substr(0, tokenlessString.size() - newLineToken.size());
        }

        // Remove the hotspot token
        if (tokenlessString.hasSuffix(hotspotToken)) {
            tokenlessString = tokenlessString.substr(0, tokenlessString.size() - hotspotToken.size());
        }

        // Word wrap the response
        font->wordWrapText(tokenlessString, maxWidth, wrappedLines, colorPixelOffset);

        res.hotspot.left = _borderWidth;
        res.hotspot.top = _firstLineOffset - font->getFontHeight() + (_numLines + 1) * lineDist;
        res.hotspot.setHeight((wrappedLines.size() - 1) * lineDist + _lineHeight);
        res.hotspot.setWidth(0);

        // Draw the response
        for (uint i = 0; i < wrappedLines.size(); ++i) {
            font->drawString(&_fullSurface, wrappedLines[i], _borderWidth + (i == 0 ? colorPixelOffset : 0), _firstLineOffset - font->getFontHeight() + _numLines * lineDist, maxWidth, 0);
            // Set the response's hotspot width
            res.hotspot.setWidth(MAX<int16>(res.hotspot.width(), font->getStringWidth(wrappedLines[i]) + (i == 0 ? colorPixelOffset : 0)));
            ++_numLines;
        }

        // Add a line break if there was a new line token
        if (!newLineAtEnd) {
            --_numLines;
        }
    }

    setVisible(true);
    _needsTextRedraw = false;
}

void Textbox::clear() {
    _fullSurface.clear();
    _mainString.clear();
    _responses.clear();
    _scrollbar.resetPosition();
    _numLines = 0;
    onScrollbarMove();
    _needsRedraw = true;
}

void Textbox::addTextLine(const Common::String &text) {
    // Scan for the hotspot token and assume the text is the main text if not found
    if (text.contains(hotspotToken)) {
        _responses.push_back(Response());
        _responses.back().text = text;
    } else {
        _mainString = text;
    }
    
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

uint16 Textbox::getInnerHeight() {
    uint lineDist = _lineHeight + _lineHeight / 4;
    return _numLines * lineDist + 2 * _firstLineOffset;
}

void Textbox::TextboxScrollbar::init() {
    Common::Rect &srcBounds = _parent->_scrollbarSourceBounds;
    Common::Point &topPosition = _parent->_scrollbarDefaultDest;

    _drawSurface.create(_engine->graphicsManager->object0, srcBounds);

    _startPosition = topPosition;
    _startPosition.x -= srcBounds.width() / 2;

    _screenPosition = srcBounds;
    _screenPosition.moveTo(_startPosition);

    _maxDist = _parent->getBounds().height() - _drawSurface.h;

    RenderObject::init();
}

} // End of namespace UI
} // End of namespace Nancy
