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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/graphics/transparent_surface.h"
#include "engines/wintermute/base/gfx/osystem/render_ticket.h"

namespace Wintermute {

RenderTicket::RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY, bool disableAlpha) : _owner(owner),
_srcRect(*srcRect), _dstRect(*dstRect), _drawNum(0), _isValid(true), _wantsDraw(true), _hasAlpha(!disableAlpha) {
	_colorMod = 0;
	_batchNum = 0;
	_mirror = TransparentSurface::FLIP_NONE;
	if (mirrorX) {
		_mirror |= TransparentSurface::FLIP_V;
	}
	if (mirrorY) {
		_mirror |= TransparentSurface::FLIP_H;
	}
	if (surf) {
		_surface = new Graphics::Surface();
		_surface->create((uint16)srcRect->width(), (uint16)srcRect->height(), surf->format);
		assert(_surface->format.bytesPerPixel == 4);
		// Get a clipped copy of the surface
		for (int i = 0; i < _surface->h; i++) {
			memcpy(_surface->getBasePtr(0, i), surf->getBasePtr(srcRect->left, srcRect->top + i), srcRect->width() * _surface->format.bytesPerPixel);
		}
		// Then scale it if necessary
		if (dstRect->width() != srcRect->width() || dstRect->height() != srcRect->height()) {
			TransparentSurface src(*_surface, false);
			Graphics::Surface *temp = src.scale(dstRect->width(), dstRect->height());
			_surface->free();
			delete _surface;
			_surface = temp;
		}
	} else {
		_surface = nullptr;
	}
}

RenderTicket::~RenderTicket() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

bool RenderTicket::operator==(RenderTicket &t) {
	if ((t._owner != _owner) ||
		(t._batchNum != _batchNum) ||
		(t._hasAlpha != _hasAlpha) ||
		(t._mirror != _mirror) ||
		(t._colorMod != _colorMod) ||
		(t._dstRect != _dstRect) ||
		(t._srcRect != _srcRect)) {
		return false;
	}
	return true;
}

// Replacement for SDL2's SDL_RenderCopy
void RenderTicket::drawToSurface(Graphics::Surface *_targetSurface) {
	TransparentSurface src(*getSurface(), false);

	Common::Rect clipRect;
	clipRect.setWidth(getSurface()->w);
	clipRect.setHeight(getSurface()->h);

	src._enableAlphaBlit = _hasAlpha;
	src.blit(*_targetSurface, _dstRect.left, _dstRect.top, _mirror, &clipRect, _colorMod, clipRect.width(), clipRect.height());
}

void RenderTicket::drawToSurface(Graphics::Surface *_targetSurface, Common::Rect *dstRect, Common::Rect *clipRect) {
	TransparentSurface src(*getSurface(), false);
	bool doDelete = false;
	if (!clipRect) {
		doDelete = true;
		clipRect = new Common::Rect();
		clipRect->setWidth(getSurface()->w);
		clipRect->setHeight(getSurface()->h);
	}

	src._enableAlphaBlit = _hasAlpha;
	src.blit(*_targetSurface, dstRect->left, dstRect->top, _mirror, clipRect, _colorMod, clipRect->width(), clipRect->height());
	if (doDelete) {
		delete clipRect;
	}
}

} // end of namespace Wintermute
