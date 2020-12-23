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

#include "glk/window_graphics.h"
#include "glk/conf.h"
#include "glk/glk.h"
#include "glk/screen.h"

namespace Glk {

GraphicsWindow::GraphicsWindow(Windows *windows, uint rock) : Window(windows, rock),
	_w(0), _h(0), _dirty(false), _surface(nullptr) {
	_type = wintype_Graphics;
	_bgnd = _bgColor;
}

GraphicsWindow::~GraphicsWindow() {
	delete _surface;
}

void GraphicsWindow::rearrange(const Rect &box) {
	int newwid, newhgt;
	int bothwid, bothhgt;
	Graphics::ManagedSurface *newSurface;

	_bbox = box;

	newwid = box.width();
	newhgt = box.height();

	if (newwid <= 0 || newhgt <= 0) {
		_w = 0;
		_h = 0;
		delete _surface;
		_surface = nullptr;
		return;
	}

	bothwid = _w;
	if (newwid < bothwid)
		bothwid = newwid;
	bothhgt = _h;
	if (newhgt < bothhgt)
		bothhgt = newhgt;

	// Create it
	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	newSurface = new Graphics::ManagedSurface(newwid, newhgt, pixelFormat);
	newSurface->clear(_bgnd);

	// If the new surface is equal or bigger than the old one, copy it over
	if (_surface && bothwid && bothhgt)
		newSurface->blitFrom(*_surface);

	delete _surface;
	_surface = newSurface;
	_w = newwid;
	_h = newhgt;

	touch();
}

void GraphicsWindow::touch() {
	_dirty = true;
	_windows->repaint(_bbox);
}

void GraphicsWindow::redraw() {
	Screen &screen = *g_vm->_screen;
	Window::redraw();

	if (_dirty || Windows::_forceRedraw) {
		_dirty = 0;

		if (_surface)
			screen.blitFrom(*_surface, Point(_bbox.left, _bbox.top));
	}
}

bool GraphicsWindow::drawPicture(const Common::String &image, int xpos, int ypos, bool scale,
                                   uint imagewidth, uint imageheight) {
	Picture *pic = g_vm->_pictures->load(image);
	uint hyperlink = _attr.hyper;

	if (!pic)
		return false;

	if (!_imageLoaded) {
		g_vm->_pictures->increment();
		_imageLoaded = true;
	}

	if (!scale) {
		imagewidth = pic->w;
		imageheight = pic->h;
	}

	drawPicture(pic, xpos, ypos, imagewidth, imageheight, hyperlink);
	touch();

	return true;
}

void GraphicsWindow::eraseRect(bool whole, const Rect &box) {
	int x0 = box.left, y0 = box.top, x1 = box.right, y1 = box.bottom;
	int hx0, hx1, hy0, hy1;

	if (whole) {
		x0 = 0;
		y0 = 0;
		x1 = _w;
		y1 = _h;
	}

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if ((uint)x0 >= _w) x0 = _w;
	if ((uint)y0 >= _h) y0 = _h;
	if ((uint)x1 >= _w) x1 = _w;
	if ((uint)y1 >= _h) y1 = _h;

	hx0 = _bbox.left + x0;
	hx1 = _bbox.left + x1;
	hy0 = _bbox.top + y0;
	hy1 = _bbox.top + y1;

	// zero out hyperlinks for these coordinates
	g_vm->_selection->putHyperlink(0, hx0, hy0, hx1, hy1);

	_surface->fillRect(Rect(x0, y0, x1, y1), _bgnd);
	touch();
}

void GraphicsWindow::fillRect(uint color, const Rect &box) {
	int x0 = box.left, y0 = box.top, x1 = box.right, y1 = box.bottom;
	int hx0, hx1, hy0, hy1;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if ((uint)x0 > _w) x0 = _w;
	if ((uint)y0 > _h) y0 = _h;
	if ((uint)x1 > _w) x1 = _w;
	if ((uint)y1 > _h) y1 = _h;

	hx0 = _bbox.left + x0;
	hx1 = _bbox.left + x1;
	hy0 = _bbox.top + y0;
	hy1 = _bbox.top + y1;

	// zero out hyperlinks for these coordinates
	g_vm->_selection->putHyperlink(0, hx0, hy0, hx1, hy1);

	Rect r(x0, y0, x1, y1);
	if (!r.isEmpty())
		_surface->fillRect(r, color);
	touch();
}

void GraphicsWindow::clear() {
	fillRect(_bgnd, Rect(0, 0, _bbox.width(), _bbox.width()));
}

void GraphicsWindow::frameRect(uint color, const Rect &box) {
	_surface->frameRect(box, color);
	touch();
}

void GraphicsWindow::drawLine(uint color, const Point &from, const Point &to) {
	_surface->drawLine(from.x, from.y, to.x, to.y, color);
	touch();
}

void GraphicsWindow::drawPicture(Picture *src, int x0, int y0, int width, int height, uint linkval) {
	if (width != src->w || height != src->h) {
		src = g_vm->_pictures->scale(src, width, height);
		if (!src)
			return;
	}

	drawPicture(*src, src->getTransparentColor(), x0, y0, width, height, linkval);
}

void GraphicsWindow::drawPicture(const Graphics::Surface &image, uint transColor, int x0, int y0,
		int width, int height, uint linkval) {
	int dx1, dy1, x1, y1, sx0, sy0, sx1, sy1;
	int hx0, hx1, hy0, hy1;
	int w, h;

	sx0 = 0;
	sy0 = 0;
	sx1 = image.w;
	sy1 = image.h;
	dx1 = _w;
	dy1 = _h;

	x1 = x0 + image.w;
	y1 = y0 + image.h;

	if (x1 <= 0 || x0 >= dx1) return;
	if (y1 <= 0 || y0 >= dy1) return;
	if (x0 < 0) {
		sx0 -= x0;
		x0 = 0;
	}
	if (y0 < 0) {
		sy0 -= y0;
		y0 = 0;
	}
	if (x1 > dx1) {
		sx1 += dx1 - x1;
		x1 = dx1;
	}
	if (y1 > dy1) {
		sy1 += dy1 - y1;
		y1 = dy1;
	}

	hx0 = _bbox.left + x0;
	hx1 = _bbox.left + x1;
	hy0 = _bbox.top + y0;
	hy1 = _bbox.top + y1;

	// zero out or set hyperlink for these coordinates
	g_vm->_selection->putHyperlink(linkval, hx0, hy0, hx1, hy1);

	w = sx1 - sx0;
	h = sy1 - sy0;

	_surface->transBlitFrom(image, Rect(sx0, sy0, sx0 + w, sy0 + h), Point(x0, y0), transColor);
}

void GraphicsWindow::getSize(uint *width, uint *height) const {
	if (width)
		*width = _bbox.width();
	if (height)
		*height = _bbox.height();
}

void GraphicsWindow::setBackgroundColor(uint color) {
	_bgnd = color;
}

void GraphicsWindow::click(const Point &newPos) {
	Point diff = newPos - Point(_bbox.left, _bbox.top);

	if (_mouseRequest) {
		g_vm->_events->store(evtype_MouseInput, this, diff.x, diff.y);
		_mouseRequest = false;
		if (g_conf->_safeClicks)
			g_vm->_events->_forceClick = true;
	}

	if (_hyperRequest) {
		uint linkval = g_vm->_selection->getHyperlink(newPos);
		if (linkval) {
			g_vm->_events->store(evtype_Hyperlink, this, linkval, 0);
			_hyperRequest = false;
			if (g_conf->_safeClicks)
				g_vm->_events->_forceClick = 1;
		}
	}
}

} // End of namespace Glk
