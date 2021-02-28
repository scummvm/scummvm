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

#include "engines/nancy/graphics.h"

#include "engines/nancy/renderobject.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/viewport.h"

namespace Nancy {

const Graphics::PixelFormat GraphicsManager::pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
const uint GraphicsManager::transColor = 0x3E0;

void GraphicsManager::init() {
    _screen.create(640, 480, pixelFormat);
    _screen.setTransparentColor(transColor); 

    Graphics::Surface surf;
    _engine->_res->loadImage("ciftree", "OBJECT0", surf);
    object0.create(surf.w, surf.h, surf.format);
    object0.blitFrom(surf, Common::Point(0, 0));
    surf.free();

    Common::SeekableReadStream *fontChunk = _engine->getBootChunkStream("FONT");
    while(fontChunk->pos() != fontChunk->size()) {
        _fonts.push_back(Font());
        _fonts.back().read(*fontChunk, _engine);
    }
}

void GraphicsManager::draw() {
    // First go through all objects and update them
    // Then add dirty rects to layers below if transparent
    for (auto it : _objects) {
        RenderObject &current = *it;
        current.updateGraphics();
    }

    Common::Array<Common::Rect> drawn;

    for (auto it : _objects) {
        RenderObject &current = *it;

        if (current._isVisible && current._needsRedraw) {
            // object is visible and updated

            if (current._redrawFrom) {
                if (current.hasMoved() && !current.getPreviousScreenPosition().isEmpty()) {
                    // Redraw previous location if moved
                    blitToScreen(*current._redrawFrom, current.getPreviousScreenPosition());
                }

                if (current.getBlitType() == RenderObject::kTrans) {
                    // Redraw below if transparent
                    blitToScreen(*current._redrawFrom, current.getScreenPosition());
                }
            }

            // Draw the object itself
            blitToScreen(current, current.getScreenPosition());
        } else if (!current._isVisible && current._needsRedraw && current._redrawFrom && !current.getPreviousScreenPosition().isEmpty()) {
            // Object just turned invisible, redraw below
            blitToScreen(*current._redrawFrom, current.getPreviousScreenPosition());
        }

        current._needsRedraw = false;
        current._previousScreenPosition = current._screenPosition;
    }

    // Draw the screen
    _screen.update();
}

void GraphicsManager::onPause(bool pause) {
    for (auto &object : _objects) {
        object->onPause(pause);
    }
}

void GraphicsManager::addObject(RenderObject *object) {
    for (auto &r : _objects) {
        if (r == object) {
            return;
        }

        if (r->getZOrder() > object->getZOrder()) {
            break;
        }
    }
    
    _objects.insert(object);
}

void GraphicsManager::removeObject(RenderObject *object) {
    for (auto &r : _objects) {
        if (r == object) {
            _objects.erase(&r);
            break;
        }
    }
}

void GraphicsManager::clearObjects() {
    _objects.clear();
}

void GraphicsManager::redrawAll() {
    for (auto &obj : _objects) {
        obj->_needsRedraw = true;
    }
}

void GraphicsManager::loadFonts() {
    Common::SeekableReadStream *chunk = _engine->getBootChunkStream("FONT");
    
    chunk->seek(0);
    while (chunk->pos() < chunk->size() - 1) {
        _fonts.push_back(Font());
        _fonts.back().read(*chunk, _engine);
    }
}

// Draw a given screen-space rectangle to the screen
void GraphicsManager::blitToScreen(const RenderObject &src, Common::Rect screenRect) {
    Common::Point pointDest(screenRect.left, screenRect.top);
    if (src.getBlitType() == RenderObject::kNoTrans) {
        _screen.blitFrom(src._drawSurface, src.convertToLocal(screenRect), pointDest);
    } else if (src.getBlitType() == RenderObject::kTrans) {
        _screen.transBlitFrom(src._drawSurface, src.convertToLocal(screenRect), pointDest, transColor);
    }
}

int GraphicsManager::objectComparator(const void *a, const void *b) {
	if (((const RenderObject*)a)->getZOrder() < ((const RenderObject*)b)->getZOrder()) {
        return -1;
    } else if (((const RenderObject*)a)->getZOrder() > ((const RenderObject*)b)->getZOrder()) {
        return 1;
    } else {
        return 0;
    }
}

} // End of namespace Nancy
