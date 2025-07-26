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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/mfc/gfx/surface.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

Surface::XIterator::XIterator(YIterator *rowIter) :
	_rowIter(rowIter), _surface(rowIter->_surface) {
}

Surface::XIterator &Surface::XIterator::operator=(int x) {
	const CPoint org = _surface->getViewportOrg();
	_x = x;
	_xMax = org.x + _surface->w;

	if (_x < org.x) {
		// Before left edge of viewport
		_pixelP = &_dummyPixel;
		_xOffset = org.x - x;
	} else if (_rowIter->_y < org.y ||
			_rowIter->_y >= _rowIter->_yMax) {
		// On a row entirely outside viewport
		_xMax = x;
		_pixelP = &_dummyPixel;
	} else {
		_pixelP = _surface->getBasePtr(_x, _rowIter->_y);
		_xOffset = 0;
	}

	return *this;
}

Surface::XIterator &Surface::XIterator::operator++() {
	++_x;

	if (_xOffset > 0) {
		if (--_xOffset == 0)
			_pixelP = _surface->getBasePtr(_x, _rowIter->_y);
	} else {
		++_pixelP;
	}

	return *this;
}

bool Surface::XIterator::operator<(int xEnd) const {
	return _x < _xMax && _x < xEnd;
}

/*--------------------------------------------*/

Surface::YIterator::YIterator(Surface *surface) : _surface(surface) {
	assert(surface && surface->format.bytesPerPixel == 1);
	CPoint org = surface->getViewportOrg();
	_yMax = org.y + surface->h;
}

Surface::YIterator &Surface::YIterator::operator=(int y) {
	_y = y;
	return *this;
}

Surface::YIterator &Surface::YIterator::operator++() {
	++_y;
	return *this;
}

bool Surface::YIterator::operator<(int yEnd) const {
	return _y < _yMax && _y < yEnd;
}

/*--------------------------------------------*/

void Surface::create(int16 width, int16 height) {
	_surface.create(width, height);
	_viewportOrg = CPoint(0, 0);
	resetClip();
}

void Surface::create(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat) {
	_surface.create(width, height, pixelFormat);
	_viewportOrg = CPoint(0, 0);
	resetClip();
}

void Surface::create(Graphics::ManagedSurface &surf, const Common::Rect &bounds) {
	_surface.create(surf, bounds);
	_viewportOrg = CPoint(0, 0);
	resetClip();
}

void Surface::setClipRect(const Common::Rect &r) {
	_clipRect = r;

	Common::Rect tmp = r;
	tmp.translate(-_viewportOrg.x, -_viewportOrg.y);
	_surface._clipRect = tmp;
}

Common::Rect Surface::getClipRect() const {
	return _clipRect;
}

void Surface::resetClip() {
	Common::Rect r(0, 0, _surface.w, _surface.h);
	r.translate(_viewportOrg.x, _viewportOrg.y);
	setClipRect(r);
}

int Surface::intersectClipRect(const Common::Rect &r) {
	setClipRect(_clipRect.findIntersectingRect(r));
	return _clipRect.isEmpty() ? NULLREGION : SIMPLEREGION;
}

int Surface::offsetClipRect(int x, int y) {
	if (_clipRect.isEmpty()) {
		return NULLREGION;
	} else {
		Common::Rect r = _clipRect;
		r.translate(x, y);
		setClipRect(r);
		return SIMPLEREGION;
	}
}

CPoint Surface::getViewportOrg() const {
	return _viewportOrg;
}

void Surface::setViewportOrg(const CPoint &pt) {
	_viewportOrg = pt;
}

void Surface::offsetViewportOrg(int x, int y) {
	_viewportOrg.x += x;
	_viewportOrg.y += y;
}

byte *Surface::getBasePtr(int x, int y) const {
	assert(format.bytesPerPixel == 1);
	assert(x >= _viewportOrg.x &&
		y >= _viewportOrg.y &&
		x < (_viewportOrg.x + this->w) &&
		y < (_viewportOrg.y + this->h));

	return (byte *)_surface.getBasePtr(
		x - _viewportOrg.x,
		y - _viewportOrg.y
	);
}

void Surface::fillRect(const Common::Rect &r, uint color) {
	Common::Rect tmp = r;
	tmp.translate(-_viewportOrg.x, -_viewportOrg.y);
	tmp = tmp.findIntersectingRect(Common::Rect(0, 0, _surface.w, _surface.h));

	if (!tmp.isEmpty())
		_surface.fillRect(tmp, color);
}

void Surface::frameRect(const Common::Rect &r, uint color) {
	assert(r.left >= _viewportOrg.x &&
		r.right <= (_viewportOrg.x + _surface.w));
	assert(r.top >= _viewportOrg.y &&
		r.bottom <= (_viewportOrg.y + _surface.h));

	_surface.frameRect(r, color);
}

void Surface::drawEllipse(int x0, int y0, int x1, int y1, uint32 color, bool filled) {
	assert(x0 >= _viewportOrg.x &&
		x1 <= (_viewportOrg.x + _surface.w));
	assert(y0 >= _viewportOrg.y &&
		y1 <= (_viewportOrg.y + _surface.h));

	x0 -= _viewportOrg.x;
	x1 -= _viewportOrg.x;
	y0 -= _viewportOrg.y;
	y1 -= _viewportOrg.y;

	_surface.drawEllipse(x0, y0, x1, y1, color, filled);
}

void Surface::drawLine(int x0, int y0, int x1, int y1, uint32 color) {
	assert(x0 >= _viewportOrg.x &&
		x1 <= (_viewportOrg.x + _surface.w));
	assert(y0 >= _viewportOrg.y &&
		y1 <= (_viewportOrg.y + _surface.h));

	x0 -= _viewportOrg.x;
	x1 -= _viewportOrg.x;
	y0 -= _viewportOrg.y;
	y1 -= _viewportOrg.y;

	_surface.drawLine(x0, y0, x1, y1, color);
}

void Surface::hLine(int x0, int y, int x1, uint32 color) {
	assert(x0 >= _viewportOrg.x &&
		x1 <= (_viewportOrg.x + _surface.w));
	assert(y >= _viewportOrg.y &&
		y <= (_viewportOrg.y + _surface.h));

	_surface.hLine(x0, y, x1, color);
}

uint32 Surface::getPixel(int x, int y) {
	assert(x >= _viewportOrg.x &&
		x <= (_viewportOrg.x + _surface.w));
	assert(y >= _viewportOrg.y &&
		y <= (_viewportOrg.y + _surface.h));

	assert(format.bytesPerPixel == 1);
	return *(byte *)getBasePtr(x, y);
}

Graphics::ManagedSurface Surface::getSubArea(const Common::Rect &r) {
	Common::Rect tmp = r;
	tmp.translate(-_viewportOrg.x, -_viewportOrg.y);
	return Graphics::ManagedSurface(_surface, tmp);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
