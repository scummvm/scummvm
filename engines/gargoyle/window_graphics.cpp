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

namespace Gargoyle {

GraphicsWindow::GraphicsWindow(Windows *windows, uint32 rock) : Window(windows, rock),
_w(0), _h(0), _dirty(false), _surface(nullptr) {
	_type = wintype_Graphics;
	Common::copy(&_bgColor[0], &_bgColor[3], _bgnd);
}

GraphicsWindow::~GraphicsWindow() {
	delete _surface;
}

void GraphicsWindow::rearrange(const Common::Rect &box) {
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
		_surface = NULL;
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
	// TODO
}

} // End of namespace Gargoyle
