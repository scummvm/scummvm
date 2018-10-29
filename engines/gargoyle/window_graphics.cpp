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

#include "gargoyle/window_graphics.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/screen.h"

namespace Gargoyle {

GraphicsWindow::GraphicsWindow(Windows *windows, uint32 rock) : Window(windows, rock),
_w(0), _h(0), _dirty(false), _surface(nullptr) {
	_type = wintype_Graphics;
	Common::copy(&_bgColor[0], &_bgColor[3], _bgnd);
}

GraphicsWindow::~GraphicsWindow() {
	delete _surface;
}

void GraphicsWindow::rearrange(const Rect &box) {
	int newwid, newhgt;
	int bothwid, bothhgt;
	int oldw, oldh;
	Graphics::ManagedSurface *newSurface;

	_bbox = box;

	newwid = box.width();
	newhgt = box.height();
	oldw = _w;
	oldh = _h;

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

	newSurface = new Graphics::ManagedSurface(newwid, newhgt,
		Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

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

glui32 GraphicsWindow::drawPicture(glui32 image, glsi32 xpos, glsi32 ypos, int scale,
		glui32 imagewidth, glui32 imageheight) {
	Picture *pic = Picture::load(image);
	glui32 hyperlink = _attr.hyper;

	if (!pic)
		return false;

	if (!_imageLoaded) {
		g_vm->_picList->increment();
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
	if ((glui32)x0 >= _w) x0 = _w;
	if ((glui32)y0 >= _h) y0 = _h;
	if ((glui32)x1 >= _w) x1 = _w;
	if ((glui32)y1 >= _h) y1 = _h;

	hx0 = _bbox.left + x0;
	hx1 = _bbox.left + x1;
	hy0 = _bbox.top + y0;
	hy1 = _bbox.top + y1;

	/* zero out hyperlinks for these coordinates */
	g_vm->_windowMask->putHyperlink(0, hx0, hy0, hx1, hy1);

	_surface->fillRect(Rect(x0, y0, x1, y1), MKTAG(_bgnd[0], _bgnd[1], _bgnd[2], 0));
	touch();
}

void GraphicsWindow::fillRect(glui32 color, const Rect &box) {
	unsigned char col[3];
	int x0 = box.left, y0 = box.top, x1 = box.right, y1 = box.bottom;
	int hx0, hx1, hy0, hy1;

	col[0] = (color >> 16) & 0xff;
	col[1] = (color >> 8) & 0xff;
	col[2] = (color >> 0) & 0xff;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if ((glui32)x0 > _w) x0 = _w;
	if ((glui32)y0 > _h) y0 = _h;
	if ((glui32)x1 > _w) x1 = _w;
	if ((glui32)y1 > _h) y1 = _h;

	hx0 = _bbox.left + x0;
	hx1 = _bbox.left + x1;
	hy0 = _bbox.top + y0;
	hy1 = _bbox.top + y1;

	/* zero out hyperlinks for these coordinates */
	g_vm->_windowMask->putHyperlink(0, hx0, hy0, hx1, hy1);

	_surface->fillRect(Rect(x0, y0, x1, y1), MKTAG(col[0], col[1], col[2], 0));
	touch();
}

void GraphicsWindow::drawPicture(Picture *src,  int x0, int y0, int width, int height, glui32 linkval) {
	int dx1, dy1, x1, y1, sx0, sy0, sx1, sy1;
	int hx0, hx1, hy0, hy1;
	int w, h;

	if (width != src->w || height != src->h) {
		src = src->scale(width, height);
		if (!src)
			return;
	}

	sx0 = 0;
	sy0 = 0;
	sx1 = src->w;
	sy1 = src->h;
	dx1 = _w;
	dy1 = _h;

	x1 = x0 + src->w;
	y1 = y0 + src->h;

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

	/* zero out or set hyperlink for these coordinates */
	g_vm->_windowMask->putHyperlink(linkval, hx0, hy0, hx1, hy1);

	w = sx1 - sx0;
	h = sy1 - sy0;

	_surface->blitFrom(*g_vm->_screen, Rect(sx0, sy0, sx0 + w, sy0 + h), Point(0, 0));
}

void GraphicsWindow::getSize(glui32 *width, glui32 *height) const {
	*width = _bbox.width();
	*height = _bbox.height();
}

void GraphicsWindow::setBackgroundColor(glui32 color) {
	_bgnd[0] = (color >> 16) & 0xff;
	_bgnd[1] = (color >> 8) & 0xff;
	_bgnd[2] = (color >> 0) & 0xff;
}

} // End of namespace Gargoyle
