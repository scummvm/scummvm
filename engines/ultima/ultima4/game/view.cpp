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

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/game/view.h"

namespace Ultima {
namespace Ultima4 {

Image *View::_screen = nullptr;

View::View(int x, int y, int width, int height) : _x(x), _y(y), _width(width), _height(height),
	_highlighted(false), _highlightX(0), _highlightY(0), _highlightW(0), _highlightH(0) {
	if (_screen == nullptr)
		_screen = imageMgr->get("screen")->_image;
}

void View::reinit() {
	_screen = imageMgr->get("screen")->_image;
}

void View::clear() {
	unhighlight();
	_screen->fillRect(SCALED(_x), SCALED(_y), SCALED(_width), SCALED(_height), 0, 0, 0);
}

void View::update() {
	if (_highlighted)
		drawHighlighted();
#ifdef IOS_ULTIMA4
	U4IOS::updateView();
#endif
}

void View::update(int x, int y, int width, int height) {
	if (_highlighted)
		drawHighlighted();
#ifdef IOS_ULTIMA4
	U4IOS::updateRectInView(x, y, width, height);
#endif
}

void View::highlight(int x, int y, int width, int height) {
	_highlighted = true;
	_highlightX = x;
	_highlightY = y;
	_highlightW = width;
	_highlightH = height;

	update(x, y, width, height);
}

void View::unhighlight() {
	_highlighted = false;
	update(_highlightX, _highlightY, _highlightW, _highlightH);
	_highlightX = _highlightY = _highlightW = _highlightH = 0;
}

void View::drawHighlighted() {
	Image *screen = imageMgr->get("screen")->_image;

	Image *tmp = Image::create(SCALED(_highlightW), SCALED(_highlightH), false, Image::SOFTWARE);
	if (!tmp)
		return;

	screen->drawSubRectOn(tmp, 0, 0, SCALED(this->_x + _highlightX), SCALED(this->_y + _highlightY), SCALED(_highlightW), SCALED(_highlightH));
	tmp->drawHighlighted();
	tmp->draw(SCALED(this->_x + _highlightX), SCALED(this->_y + _highlightY));
	delete tmp;
}

} // End of namespace Ultima4
} // End of namespace Ultima
