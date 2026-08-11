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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */


#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/osystem/render_ticket.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"

#include "graphics/managed_surface.h"

#include "common/textconsole.h"

namespace Wintermute {

RenderTicket::RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf,
                           Common::Rect *srcRect, Common::Rect *dstRect, Graphics::TransformStruct transform) :
	        _owner(owner),
	        _srcRect(*srcRect),
	        _dstRect(*dstRect),
	        _isValid(true),
	        _wantsDraw(true),
	        _transform(transform) {
	if (surf) {
		assert(surf->format.bytesPerPixel == 4);

		// Get a clipped view of the surface
		const Graphics::Surface temp = surf->getSubArea(*srcRect);

		// Then copy and scale it as necessary
		//
		// NB: The numTimesX/numTimesY properties don't yet mix well with
		// scaling and rotation, but there is no need for that functionality at
		// the moment.
		// NB: Mirroring and rotation are probably done in the wrong order.
		// (Mirroring should most likely be done before rotation. See also
		// TransformTools.)
		if (_transform._angle != Graphics::kDefaultAngle) {
			_surface = temp.rotoscale(transform, owner->_game->getBilinearFiltering());
		} else if ((dstRect->width() != srcRect->width() ||
			    dstRect->height() != srcRect->height()) &&
			    _transform._numTimesX * _transform._numTimesY == 1) {
			_surface = temp.scale(dstRect->width(), dstRect->height(), owner->_game->getBilinearFiltering());
		} else {
			_surface = new Graphics::Surface();
			_surface->copyFrom(temp);
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

bool RenderTicket::operator==(const RenderTicket &t) const {
	if ((t._owner != _owner) ||
		(t._transform != _transform)  ||
		(t._dstRect != _dstRect) ||
		(t._srcRect != _srcRect)
	) {
		return false;
	}
	return true;
}

// Replacement for SDL2's SDL_RenderCopy
void RenderTicket::drawToSurface(Graphics::ManagedSurface *_targetSurface) const {
	if (!getSurface()) {
		_targetSurface->blendFillRect(_dstRect, _transform._rgbaMod, Graphics::BLEND_NORMAL);
		return;
	}

	Common::Rect clipRect;
	clipRect.setWidth(getSurface()->w);
	clipRect.setHeight(getSurface()->h);

	Graphics::AlphaType alphaMode = Graphics::ALPHA_FULL;

	if (_owner) {
		if (_transform._alphaDisable) {
			alphaMode = Graphics::ALPHA_OPAQUE;
		} else if (_transform._angle) {
			alphaMode = Graphics::ALPHA_FULL;
		} else {
			alphaMode = _owner->getAlphaType();
		}
	}

	int y = _dstRect.top;
	int w = _dstRect.width() / _transform._numTimesX;
	int h = _dstRect.height() / _transform._numTimesY;

	for (int ry = 0; ry < _transform._numTimesY; ++ry) {
		int x = _dstRect.left;
		for (int rx = 0; rx < _transform._numTimesX; ++rx) {
			_targetSurface->blendBlitFrom(*getSurface(), clipRect, Common::Point(x, y),
				_transform._flip, _transform._rgbaMod, Graphics::BLEND_NORMAL, alphaMode);
			x += w;
		}
		y += h;
	}
}

void RenderTicket::drawToSurface(Graphics::ManagedSurface *_targetSurface, Common::Rect *dstRect, Common::Rect *clipRect) const {
	if (!getSurface()) {
		_targetSurface->blendFillRect(*dstRect, _transform._rgbaMod, _transform._blendMode);
		return;
	}

	bool doDelete = false;
	if (!clipRect) {
		doDelete = true;
		clipRect = new Common::Rect();
		clipRect->setWidth(getSurface()->w * _transform._numTimesX);
		clipRect->setHeight(getSurface()->h * _transform._numTimesY);
	}

	Graphics::AlphaType alphaMode = Graphics::ALPHA_FULL;

	if (_owner) {
		if (_transform._alphaDisable) {
			alphaMode = Graphics::ALPHA_OPAQUE;
		} else if (_transform._angle) {
			alphaMode = Graphics::ALPHA_FULL;
		} else {
			alphaMode = _owner->getAlphaType();
		}
	}

	if (_transform._numTimesX * _transform._numTimesY == 1) {
		_targetSurface->blendBlitFrom(*getSurface(), *clipRect, Common::Point(dstRect->left, dstRect->top),
			_transform._flip, _transform._rgbaMod, _transform._blendMode, alphaMode);
	} else {
		// clipRect is a subrect of the full numTimesX*numTimesY rect
		Common::Rect subRect;

		int y = 0;
		int w = getSurface()->w;
		int h = getSurface()->h;
		assert(w == _dstRect.width() / _transform._numTimesX);
		assert(h == _dstRect.height() / _transform._numTimesY);

		int basex = dstRect->left - clipRect->left;
		int basey = dstRect->top - clipRect->top;

		for (int ry = 0; ry < _transform._numTimesY; ++ry) {
			int x = 0;
			for (int rx = 0; rx < _transform._numTimesX; ++rx) {
				subRect.left = x;
				subRect.top = y;
				subRect.setWidth(w);
				subRect.setHeight(h);

				if (subRect.intersects(*clipRect)) {
					subRect.clip(*clipRect);
					subRect.translate(-x, -y);
					_targetSurface->blendBlitFrom(*getSurface(), subRect,
						Common::Point(basex + x + subRect.left, basey + y + subRect.top),
						_transform._flip, _transform._rgbaMod, _transform._blendMode, alphaMode);
				}

				x += w;
			}
			y += h;
		}
	}

	if (doDelete) {
		delete clipRect;
	}
}

} // End of namespace Wintermute
