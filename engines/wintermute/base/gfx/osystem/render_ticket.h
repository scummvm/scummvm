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

#ifndef WINTERMUTE_RENDER_TICKET_H
#define WINTERMUTE_RENDER_TICKET_H

#include "graphics/surface.h"
#include "common/rect.h"

namespace Wintermute {

class BaseSurfaceOSystem;
class RenderTicket {
public:
	RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRest, bool mirrorX = false, bool mirrorY = false, bool disableAlpha = false);
	RenderTicket() : _isValid(true), _wantsDraw(false), _drawNum(0) {}
	~RenderTicket();
	const Graphics::Surface *getSurface() { return _surface; }
	// Non-dirty-rects:
	void drawToSurface(Graphics::Surface *_targetSurface);
	// Dirty-rects:
	void drawToSurface(Graphics::Surface *_targetSurface, Common::Rect *dstRect, Common::Rect *clipRect);

	Common::Rect _dstRect;
	uint32 _batchNum;

	bool _isValid;
	bool _wantsDraw;
	uint32 _drawNum;
	uint32 _colorMod;

	BaseSurfaceOSystem *_owner;
	bool operator==(RenderTicket &a);
	const Common::Rect *getSrcRect() { return &_srcRect; }
private:
	Graphics::Surface *_surface;
	Common::Rect _srcRect;
	bool _hasAlpha;
	uint32 _mirror;
};

} // end of namespace Wintermute

#endif
