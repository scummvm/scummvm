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

#ifndef NANCY_TEXTBOX_H
#define NANCY_TEXTBOX_H

#include "engines/nancy/datatypes.h"

#include "common/str.h"
#include "common/array.h"

#include "graphics/managed_surface.h"

namespace Nancy {

class NancyEngine;

class Textbox {
    friend class GraphicsManager;
    friend class SceneManager;

public:
    Textbox(NancyEngine *engine) :
        _engine(engine),
        _needsUpdate(false),
        lineNr(0),
        _firstLineOffset(0),
        _lineHeight(0),
        _borderWidth(0) {}
    
    void init();
    void clear();

    Common::Rect processTextLine(const Common::String &text, uint16 fontID);
    void processResponse(const Common::String &text, uint16 fontID, uint16 id, Common::String soundName);

    int16 getHovered(Common::Point mousePos);
    void setPosition(float pos);

    const Graphics::Surface *getSurface();

private:
    uint16 getInnerHeight();
    Common::Rect getCharacterSourceRect(char c, uint16 fontID);
    uint16 getCharacterWidth(char c, uint16 fontID);
    void drawText(const Common::String &text, uint16 fontID, uint16 left, uint16 bottom, bool color = false);
    
private:
    struct Response {
        Common::String soundName;
        Common::Rect hotspot;
    };
    
    NancyEngine *_engine;
    Common::String _formattedText;
    bool _needsUpdate;
    Common::Array<Font> _fonts;
    Graphics::ManagedSurface _surface;
    uint16 lineNr;
    Common::Array<Response> _responses;
    
    uint16 _firstLineOffset;
    uint16 _lineHeight;
    uint16 _borderWidth;

    static const Common::String beginToken;
    static const Common::String endToken;
    static const Common::String colorBeginToken;
    static const Common::String colorEndToken;
    static const Common::String hotspotToken;
    static const Common::String newLineToken;

};

} // End of namespace Nancy

#endif // NANCY_TEXTBOX_H