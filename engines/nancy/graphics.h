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

#ifndef NANCY_GRAPHICS_H
#define NANCY_GRAPHICS_H

#include "engines/nancy/renderobject.h"
#include "engines/nancy/font.h"

#include "common/array.h"

#include "graphics/screen.h"

namespace Nancy {

class NancyEngine;

// Graphics class that handles multilayered surface rendering with minimal redraw
class GraphicsManager {
public:
    GraphicsManager() : _objects(objectComparator) {}

    void init();
    void draw();

    void addObject(RenderObject *object);
    void removeObject(RenderObject *object);
    void clearObjects();

    void redrawAll();

    Font *getFont(uint id) { return id < _fonts.size() ? &_fonts[id] : nullptr; }

    Graphics::ManagedSurface object0;
    
    static const Graphics::PixelFormat pixelFormat;
    static const uint transColor;

private:
    void loadFonts();
    void blitToScreen(const RenderObject &src, Common::Rect dest);

    static int objectComparator(const void *a, const void *b);

    Common::SortedArray<RenderObject *> _objects;

    Graphics::Screen _screen;
    Common::Array<Font> _fonts;
};

} // End of namespace Nancy

#endif // NANCY_GRAPHICS_H
