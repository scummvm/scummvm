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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imagemgr.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/view.h"

namespace Ultima {
namespace Ultima4 {

Image *View::screen = NULL;

View::View(int x, int y, int width, int height)
: x(x), y(y), width(width), height(height), highlighted(false), highlightX(0), highlightY(0), highlightW(0), highlightH(0)
{
    if (screen == NULL)
        screen = imageMgr->get("screen")->image;
}

/**
 * Hook for reinitializing when graphics reloaded.
 */
void View::reinit() {
    screen = imageMgr->get("screen")->image;
}

/**
 * Clear the view to black.
 */
void View::clear() {
    unhighlight();
    screen->fillRect(SCALED(x), SCALED(y), SCALED(width), SCALED(height), 0, 0, 0);
}

/**
 * Update the view to the screen.
 */
void View::update() {
    if (highlighted)
        drawHighlighted();
#ifdef IOS
    U4IOS::updateView();
#endif
}

/**
 * Update a piece of the view to the screen.
 */
void View::update(int x, int y, int width, int height) {
    if (highlighted)
        drawHighlighted();
#ifdef IOS
    U4IOS::updateRectInView(x, y, width, height);
#endif
}

/**
 * Highlight a piece of the screen by drawing it in inverted colors.
 */ 
void View::highlight(int x, int y, int width, int height) {
    highlighted = true;
    highlightX = x;
    highlightY = y;
    highlightW = width;
    highlightH = height;
    
    update(x, y, width, height);
}

void View::unhighlight() {
    highlighted = false;
    update(highlightX, highlightY, highlightW, highlightH);
    highlightX = highlightY = highlightW = highlightH = 0;
}

void View::drawHighlighted() {
    Image *screen = imageMgr->get("screen")->image;
    
    Image *tmp = Image::create(SCALED(highlightW), SCALED(highlightH), false, Image::SOFTWARE);
    if (!tmp)
        return;
    
    screen->drawSubRectOn(tmp, 0, 0, SCALED(this->x + highlightX), SCALED(this->y + highlightY), SCALED(highlightW), SCALED(highlightH));
    tmp->drawHighlighted();
    tmp->draw(SCALED(this->x + highlightX), SCALED(this->y + highlightY));
    delete tmp;    
}

} // End of namespace Ultima4
} // End of namespace Ultima
