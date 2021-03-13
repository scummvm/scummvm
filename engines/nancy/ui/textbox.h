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

#ifndef NANCY_UI_TEXTBOX_H
#define NANCY_UI_TEXTBOX_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/ui/scrollbar.h"

#include "common/str.h"
#include "common/array.h"

#include "graphics/managed_surface.h"

namespace Nancy {

class NancyEngine;
class Scene;
struct NancyInput;

namespace UI {

class Textbox : public Nancy::RenderObject {
public:
    Textbox(RenderObject &redrawFrom) :
        RenderObject(redrawFrom),
        _firstLineOffset(0),
        _lineHeight(0),
        _borderWidth(0),
        _needsTextRedraw(false),
        _scrollbar(redrawFrom, this),
        _scrollbarPos(0) {}

    virtual ~Textbox() { _fullSurface.free(); }
    
    virtual void init() override;
    virtual void registerGraphics() override;
    virtual void updateGraphics() override;
    void handleInput(NancyInput &input);

    void drawTextbox();
    void clear();

    void addTextLine(const Common::String &text);
    void onScrollbarPositionChanged(float data);

    static void assembleTextLine(char *rawCaption, Common::String &output, uint size);

protected:
    virtual uint16 getZOrder() const override { return 6; }

private:
    uint16 getInnerHeight() const;
    void onScrollbarMove();

    struct Response {
        Common::String text;
        Common::Rect hotspot;
    };

    class TextboxScrollbar : public Scrollbar {
    public:
        TextboxScrollbar(RenderObject &redrawFrom, Textbox *parent) :
            Scrollbar(redrawFrom),
            _parent(parent) {}
        ~TextboxScrollbar() =default;

        virtual void init() override;
        Textbox *_parent;
    };

    Graphics::ManagedSurface _fullSurface;

    TextboxScrollbar _scrollbar;

    Common::Array<Common::String> _textLines;
    Common::Array<Common::Rect> _hotspots;

    Common::Rect _scrollbarSourceBounds;
    Common::Point _scrollbarDefaultDest;
    uint16 _firstLineOffset;
    uint16 _lineHeight;
    uint16 _borderWidth;
    uint16 _numLines;
    uint16 _fontID;

    bool _needsTextRedraw;
    float _scrollbarPos;

    static const char CCBeginToken[];
    static const char CCEndToken[];
    static const char colorBeginToken[];
    static const char colorEndToken[];
    static const char hotspotToken[];
    static const char newLineToken[];
    static const char tabToken[];
    static const char telephoneEndToken[];

protected:
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_TEXTBOX_H
