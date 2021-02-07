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

#include "engines/nancy/textbox.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"

#include "common/error.h"
#include "common/util.h"

namespace Nancy {

const Common::String Textbox::beginToken = Common::String("<i>");
const Common::String Textbox::endToken = Common::String("<o>");
const Common::String Textbox::colorBeginToken = Common::String("<c1>");
const Common::String Textbox::colorEndToken = Common::String("<c0>");
const Common::String Textbox::hotspotToken = Common::String("<h>");
const Common::String Textbox::newLineToken = Common::String("<n>");

void Textbox::init() {
    Common::SeekableReadStream *chunk = _engine->getBootChunkStream("FONT");
    
    chunk->seek(0);
    while (chunk->pos() < chunk->size() - 1) {
        _fonts.push_back(Font());
        _fonts.back().read(*chunk);

        _engine->_res->loadImage("ciftree", _fonts.back().imageName, _fonts.back().image);
    }
    
    chunk = _engine->getBootChunkStream("TBOX");
    chunk->seek(0x28);
    uint width = chunk->readUint32LE();
    uint height = chunk->readUint32LE();
    chunk->seek(0x20);
    width -= chunk->readUint32LE();
    height -= chunk->readUint32LE();
    _surface.create(width, height, GraphicsManager::pixelFormat);

    chunk->seek(0x36);
    _firstLineOffset = chunk->readUint16LE();
    _lineHeight = chunk->readUint16LE();
    _borderWidth = chunk->readUint16LE();
}

void Textbox::clear() {
    _surface.clear();
    lineNr = 0;
    _responses.clear();
}

Common::Rect Textbox::processTextLine(const Common::String &text, uint16 fontID) {
    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("FRAME TB SURF");
    uint textWidth = 0;
    Common::String line;
    uint maxWidth = zr.destRect.width() - _borderWidth;
    uint lineDist = _lineHeight + _lineHeight / 4;
    Common::Rect ret;
    ret.left = _borderWidth;
    ret.top = _firstLineOffset - _lineHeight + lineNr * lineDist;
    ret.right = ret.left;
    ret.bottom = ret.top + _lineHeight;
    
    bool colorTokenIsOpen = false;
    uint16 colorOffset = 0;

    for (uint i = 0; i < text.size(); ++i) {
        if (text[i] == '<') {
            Common::String token = text.substr(i, 4);
            if (token.hasPrefix(beginToken)) {
                // Ignore and continue
                i += beginToken.size() - 1;
            } else if (token.hasPrefix(endToken)) {
                // Caption has ended, return
                return ret;
            } else if (token.hasPrefix(colorBeginToken)) {
                // Assumes color is always at beginning of line
                colorTokenIsOpen = true;
                i += colorBeginToken.size() - 1;
                colorOffset = 0;
            } else if (token.hasPrefix(colorEndToken)) {
                // Draw color line (always a single letter?)
                colorTokenIsOpen = false;
                drawText(line, 1, _borderWidth, _firstLineOffset + lineNr * lineDist, true);
                line.clear();
                i += colorEndToken.size() - 1;
            } else if (token.hasPrefix(hotspotToken)) {
                // TODO ADD HOTSPOT
                i += hotspotToken.size() - 1;
            } else if (token.hasPrefix(newLineToken)) {
                // Draw finished line
                drawText(line, fontID, colorOffset + _borderWidth, _firstLineOffset + lineNr * lineDist);
                line.clear();
                ret.setWidth(MAX<int16>(ret.width(), textWidth));
                textWidth = 0;
                ++lineNr;
                i += newLineToken.size() - 1;
            }
        } else {
            if (colorTokenIsOpen) {
                colorOffset += getCharacterWidth(text[i], fontID) + 1;
            }
            line += text[i];
            textWidth += getCharacterWidth(text[i], fontID) + 1;

            // Wrap text
            if (textWidth > maxWidth) {
                while (line.size() > 0) {
                    if (line.lastChar() != ' ') {
                        line.deleteLastChar();
                        --i;
                    } else {
                        drawText(line, fontID, colorOffset + _borderWidth, _firstLineOffset + lineNr * lineDist);
                        line.clear();
                        ret.setWidth(MAX<int16>(ret.width(), textWidth));
                        ret.bottom += lineDist;
                        textWidth = 0;
                        colorOffset = 0;
                        ++lineNr;
                        break;
                    }
        }
            }
        }
    }
    
    if (line.size()) {
        drawText(line, fontID, colorOffset + _borderWidth, _firstLineOffset + lineNr * lineDist);
        ret.setWidth(MAX<int16>(ret.width(), textWidth));
    }

    return ret;
}


void Textbox::processResponse(const Common::String &text, uint16 fontID, uint16 id, Common::String soundName) {
    ++lineNr;
    _responses.push_back(Response());
    Response &res = _responses.back();

    res.hotspot = processTextLine(text, fontID);
    res.soundName = soundName;
}

int16 Textbox::getHovered(Common::Point mousePos) {
    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("FRAME TB SURF");
    int16 pickedResponse = -1;

    // Adjust the mouse to local coordinates
    mousePos.x -= zr.destRect.left;
    mousePos.y -= zr.destRect.top;

    // Adjust for scroll
    mousePos.y += zr.sourceRect.top;

    for (uint i = 0; i < _responses.size(); ++i) {
        if (_responses[i].hotspot.contains(mousePos)) {
            pickedResponse = i;
        }
    }

    return pickedResponse;
}

uint16 Textbox::getInnerHeight() {
    uint lineDist = _lineHeight + _lineHeight / 4;
    return lineNr * lineDist + 2 * _firstLineOffset;
}

Common::Rect Textbox::getCharacterSourceRect(char c, uint16 fontID) {
    using namespace Common;
    Font &font = _fonts[fontID];
    uint offset = 0;
    Common::Rect ret;

    if (isUpper(c)) {
        offset = c + font.uppercaseOffset - 0x41;
    } else if (isLower(c)) {
        offset = c + font.lowercaseOffset - 0x61;
    } else if (isDigit(c)) {
        offset = c + font.digitOffset - 0x30;
    } else if (isSpace(c)) {
        ret.setWidth(font.spaceWidth);
        return ret;
    } else if (isPunct(c)) {
        switch (c) {
            case '.':
                offset = font.periodOffset;
                break;
            case ',':
                offset = font.commaOffset;
                break;
            case '=':
                offset = font.equalitySignOffset;
                break;
            case ':':
                offset = font.colonOffset;
                break;
            case '-':
                offset = font.dashOffset;
                break;
            case '?':
                offset = font.questionMarkOffset;
                break;
            case '!':
                offset = font.exclamationMarkOffset;
                break;
            case '%':
                offset = font.percentOffset;
                break;
            case '&':
                offset = font.ampersandOffset;
                break;
            case '*':
                offset = font.asteriskOffset;
                break;
            case '(':
                offset = font.leftBracketOffset;
                break;
            case ')':
                offset = font.rightBracketOffset;
                break;
            case '+':
                offset = font.plusOffset;
                break;
            case '\'':
                offset = font.apostropheOffset;
                break;
            case ';':
                offset = font.semicolonOffset;
                break;
            case '/':
                offset = font.slashOffset;
                break;
            default:
                error("Unsupported FONT character: %c", c);
        }
    }
    ret = font.symbolRects[offset];
    ret.right += 1;
    return ret;
}

uint16 Textbox::getCharacterWidth(char c, uint16 fontID) {
    return getCharacterSourceRect(c, fontID).width();
}

void Textbox::drawText(const Common::String &text, uint16 fontID, uint16 left, uint16 bottom, bool color) {
    Common::Rect source;
    Common::Rect dest;
    dest.left = left;
    dest.bottom = bottom;

    for (char c : text) {
        source = getCharacterSourceRect(c, fontID);
        if (color) {
            source.left += _fonts[fontID].colorCoordsOffset.x;
            source.right += _fonts[fontID].colorCoordsOffset.x;
            source.top += _fonts[fontID].colorCoordsOffset.y;
            source.bottom += _fonts[fontID].colorCoordsOffset.y;
        }

        if (Common::isSpace(c)) {
            dest.left += source.width();
            continue;
        }

        dest.right = dest.left + source.width();
        dest.top = dest.bottom - source.height();

        _surface.transBlitFrom(_fonts[fontID].image, source, dest, GraphicsManager::transColor);
        dest.left = dest.right + 1;
    }
}

} // End of namespace Nancy