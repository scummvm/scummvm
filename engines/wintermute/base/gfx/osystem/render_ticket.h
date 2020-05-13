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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_RENDER_TICKET_H
#define WINTERMUTE_RENDER_TICKET_H

#include "graphics/transparent_surface.h"
#include "graphics/surface.h"
#include "common/rect.h"

namespace Wintermute {

class BaseSurfaceOSystem;
/**
 * A single RenderTicket.
 * A render ticket is a collection of the data and draw specifications made
 * for a single draw-call in the OSystem-backend for WME. The ticket additionally
 * holds the order in which this call was made, so that it can be detected if
 * the same call is done in the following frame. Thus allowing us to potentially
 * skip drawing the same region again, unless anything has changed. Since a surface
 * can have a potentially large amount of draw-calls made to it, at varying rotation,
 * zoom, and crop-levels we also need to hold a copy of the necessary data.
 * (Video-surfaces may even change their data). The promise that is made when a ticket
 * is created is that what the state was of the surface at THAT point, is what will end
 * up on screen at flip() time.
 */
class RenderTicket {
public:
	RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRest, Graphics::TransformStruct transform);
	RenderTicket() : _isValid(true), _wantsDraw(false), _transform(Graphics::TransformStruct()) {}
	~RenderTicket();
	const Graphics::Surface *getSurface() const { return _surface; }
	// Non-dirty-rects:
	void drawToSurface(Graphics::Surface *_targetSurface) const;
	// Dirty-rects:
	void drawToSurface(Graphics::Surface *_targetSurface, Common::Rect *dstRect, Common::Rect *clipRect) const;

	Common::Rect _dstRect;

	bool _isValid;
	bool _wantsDraw;

	Graphics::TransformStruct _transform;

	BaseSurfaceOSystem *_owner;
	bool operator==(const RenderTicket &a) const;
	const Common::Rect *getSrcRect() const { return &_srcRect; }
private:
	Graphics::Surface *_surface;
	Common::Rect _srcRect;
};

} // End of namespace Wintermute

#endif
