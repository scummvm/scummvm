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


#include "engines/wintermute/base/gfx/osystem/render_ticket.h"
#include "engines/wintermute/graphics/transform_tools.h"
#include "common/textconsole.h"

namespace Wintermute {

RenderTicket::RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, TransformStruct transform) :
	_owner(owner),
	_srcRect(*srcRect),
	_dstRect(*dstRect),
	_drawNum(0),
	_isValid(true),
	_wantsDraw(true),
	_transform(transform) {
	_batchNum = 0;
	if (surf) {
		_surface = new Graphics::Surface();
		_surface->create((uint16)srcRect->width(), (uint16)srcRect->height(), surf->format);
		assert(_surface->format.bytesPerPixel == 4);
		// Get a clipped copy of the surface
		for (int i = 0; i < _surface->h; i++) {
			memcpy(_surface->getBasePtr(0, i), surf->getBasePtr(srcRect->left, srcRect->top + i), srcRect->width() * _surface->format.bytesPerPixel);
		}
		// Then scale it if necessary
		if (_transform._angle != kDefaultAngle) {
			TransparentSurface src(*_surface, false);
			Graphics::Surface *temp = src.rotoscale(transform);
			_surface->free();
			delete _surface;
			_surface = temp;
		} else if (dstRect->width() != srcRect->width() || dstRect->height() != srcRect->height()) { 
			TransparentSurface src(*_surface, false);
			Graphics::Surface *temp = src.scale(dstRect->width(), dstRect->height());
			_surface->free();
			delete _surface;
			_surface = temp;
		}
	} else {
		_surface = nullptr;
		
		if (transform._angle != kDefaultAngle) { // Make sure comparison-tickets get the correct width
			Rect32 newDstRect;
			Point32 newHotspot;
			newDstRect = TransformTools::newRect(_srcRect, transform, &newHotspot);
			_dstRect.setWidth(newDstRect.right - newDstRect.left);
			_dstRect.setHeight(newDstRect.bottom - newDstRect.top);
		}
	}
}

RenderTicket::~RenderTicket() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

bool RenderTicket::operator==(const RenderTicket &t) const {
	if ((t._owner != _owner) ||
		(t._batchNum != _batchNum) ||
		(t._transform != _transform)  || 
		(t._dstRect != _dstRect) ||
		(t._srcRect != _srcRect) 
	) {
		return false;
	}
	return true;
}

// Replacement for SDL2's SDL_RenderCopy
void RenderTicket::drawToSurface(Graphics::Surface *_targetSurface) const {
	TransparentSurface src(*getSurface(), false);

	Common::Rect clipRect;
	clipRect.setWidth(getSurface()->w);
	clipRect.setHeight(getSurface()->h);

	src._enableAlphaBlit = !_transform._alphaDisable;
	src.blit(*_targetSurface, _dstRect.left, _dstRect.top, _transform._flip, &clipRect, _transform._rgbaMod, clipRect.width(), clipRect.height(), _transform._blendMode);
}

void RenderTicket::drawToSurface(Graphics::Surface *_targetSurface, Common::Rect *dstRect, Common::Rect *clipRect) const {
	TransparentSurface src(*getSurface(), false);
	bool doDelete = false;
	if (!clipRect) {
		doDelete = true;
		clipRect = new Common::Rect();
		clipRect->setWidth(getSurface()->w);
		clipRect->setHeight(getSurface()->h);
	}
	src._enableAlphaBlit = !_transform._alphaDisable; 
	src.blit(*_targetSurface, dstRect->left, dstRect->top, _transform._flip, clipRect, _transform._rgbaMod, clipRect->width(), clipRect->height());
	if (doDelete) {
		delete clipRect;
	}
}

} // End of namespace Wintermute
