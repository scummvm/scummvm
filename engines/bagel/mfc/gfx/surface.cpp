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
	const CPoint org = _surface->getViewportOrg();
	_xMin = _surface->_clipRect.left - org.x;
	_xMax = _surface->_clipRect.right - org.x;
}

Surface::XIterator &Surface::XIterator::operator=(int x) {
	//const CPoint org = _surface->getViewportOrg();
	_x = x;
	int y = _rowIter->_y;

	if (x < _xMin || x >= _xMax ||
			y < _rowIter->_yMin || y >= _rowIter->_yMax)
		_pixelP = &_dummyPixel;
	else
		_pixelP = _surface->getBasePtr(_x, _rowIter->_y);

	return *this;
}

Surface::XIterator &Surface::XIterator::operator++() {
	++_x;
	int y = _rowIter->_y;

	if (y < _rowIter->_yMin || y >= _rowIter->_yMax)
		_pixelP = &_dummyPixel;
	else if (_x == _xMin)
		_pixelP = _surface->getBasePtr(_x, y);
	else if (_x >= _xMax)
		_pixelP = &_dummyPixel;
	else if (_x >= _xMin)
		++_pixelP;

	return *this;
}

bool Surface::XIterator::operator<(int xEnd) const {
	return _x < xEnd;
}

Surface::XIterator::operator byte *() {
	// Keep resetting the dummy pixel, in case
	// the pixel pointer is pointing to it
	_dummyPixel = 0;

	// Return the pixel pointer
	return _pixelP;
}


/*--------------------------------------------*/

Surface::YIterator::YIterator(Surface *surface) : _surface(surface) {
	assert(surface && surface->format.bytesPerPixel <= 1);
	CPoint org = surface->getViewportOrg();

	_yMin = surface->_clipRect.top - org.y;
	_yMax = surface->_clipRect.bottom - org.y;
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
	return _y < yEnd;
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
}

Common::Rect Surface::getClipRect() const {
	return _clipRect;
}

void Surface::resetClip() {
	setClipRect(Common::Rect(0, 0, _surface.w, _surface.h));
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

byte *Surface::getBasePtr(int x, int y) {
	assert(format.bytesPerPixel == 1);

	x += _viewportOrg.x;
	y += _viewportOrg.y;
	assert(x >= 0 && y >= 0 && x < this->w && y < this->h);
	return (byte *)_surface.getBasePtr(x, y);
}

void Surface::addDirtyRect(const Common::Rect &r) {
	Common::Rect tmp = r;
	tmp.translate(_viewportOrg.x, _viewportOrg.y);
	tmp = tmp.findIntersectingRect(_clipRect);

	_surface.addDirtyRect(tmp);
}

void Surface::fillRect(const Common::Rect &r, uint color) {
	Common::Rect tmp = r;
	tmp.translate(_viewportOrg.x, _viewportOrg.y);
	tmp = tmp.findIntersectingRect(_clipRect);

	if (!tmp.isEmpty())
		_surface.fillRect(tmp, color);
}

void Surface::frameRect(const Common::Rect &r, uint color) {
	Common::Rect tmp = r;
	tmp.translate(_viewportOrg.x, _viewportOrg.y);
	tmp = tmp.findIntersectingRect(_clipRect);

	if (!tmp.isEmpty())
		_surface.frameRect(r, color);
}

void Surface::drawEllipse(int x0, int y0, int x1, int y1, uint32 color, bool filled) {
	x0 += _viewportOrg.x;
	x1 += _viewportOrg.x;
	y0 += _viewportOrg.y;
	y1 += _viewportOrg.y;
	assert(x0 >= 0 && y0 >= 0 && x1 <= _surface.w && y1 <= _surface.h);

	_surface.drawEllipse(x0, y0, x1, y1, color, filled);
}

void Surface::drawLine(int x0, int y0, int x1, int y1, uint32 color) {
	x0 += _viewportOrg.x;
	x1 += _viewportOrg.x;
	y0 += _viewportOrg.y;
	y1 += _viewportOrg.y;
	assert(x0 >= 0 && y0 >= 0 && x1 <= _surface.w && y1 <= _surface.h);

	_surface.drawLine(x0, y0, x1, y1, color);
}

void Surface::hLine(int x0, int y, int x1, uint32 color) {
	x0 += _viewportOrg.x;
	x1 += _viewportOrg.x;
	y += _viewportOrg.y;
	assert(x0 >= 0 && y >= 0 && x1 <= _surface.w && y <= _surface.h);

	_surface.hLine(x0, y, x1, color);
}

uint32 Surface::getPixel(int x, int y) {
	x += _viewportOrg.x;
	y += _viewportOrg.y;
	assert(x >= 0 && y >= 0 && x <= _surface.w && y <= _surface.h);
	assert(format.bytesPerPixel == 1);

	return *(byte *)getBasePtr(x, y);
}

Graphics::ManagedSurface Surface::getSubArea(const Common::Rect &r) {
	Common::Rect tmp = r;
	tmp.translate(_viewportOrg.x, _viewportOrg.y);
	assert(tmp.left >= 0 && tmp.top >= 0 &&
		tmp.right <= _surface.w && tmp.bottom <= _surface.h);

	return Graphics::ManagedSurface(_surface, tmp);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
