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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BActiveRect.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBActiveRect::CBActiveRect(CBGame *inGame): CBBase(inGame) {
	CBPlatform::SetRectEmpty(&_rect);
	_owner = NULL;
	_frame = NULL;
	_region = NULL;
	_zoomX = 100;
	_zoomY = 100;
	_offsetX = _offsetY = 0;
	ClipRect();
}


//////////////////////////////////////////////////////////////////////
CBActiveRect::CBActiveRect(CBGame *inGame, CBObject *Owner, CBSubFrame *Frame, int X, int Y, int Width, int Height, float ZoomX, float ZoomY, bool Precise): CBBase(inGame) {
	_owner = Owner;
	_frame = Frame;
	CBPlatform::SetRect(&_rect, X, Y, X + Width, Y + Height);
	_zoomX = ZoomX;
	_zoomY = ZoomY;
	_precise = Precise;
	_region = NULL;
	_offsetX = _offsetY = 0;
	ClipRect();
}

//////////////////////////////////////////////////////////////////////
CBActiveRect::CBActiveRect(CBGame *inGame, CBObject *Owner, CBRegion *Region, int OffsetX, int OffsetY): CBBase(inGame) {
	_owner = Owner;
	_region = Region;
	CBPlatform::CopyRect(&_rect, &Region->_rect);
	CBPlatform::OffsetRect(&_rect, -OffsetX, -OffsetY);
	_zoomX = 100;
	_zoomY = 100;
	_precise = true;
	_frame = NULL;
	ClipRect();
	_offsetX = OffsetX;
	_offsetY = OffsetY;
}


//////////////////////////////////////////////////////////////////////
CBActiveRect::~CBActiveRect() {
	_owner = NULL;
	_frame = NULL;
	_region = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBActiveRect::ClipRect() {
	RECT rc;
	bool CustomViewport;
	Game->GetCurrentViewportRect(&rc, &CustomViewport);
	CBRenderer *Rend = Game->_renderer;

	if (!CustomViewport) {
		rc.left -= Rend->_drawOffsetX;
		rc.right -= Rend->_drawOffsetX;
		rc.top -= Rend->_drawOffsetY;
		rc.bottom -= Rend->_drawOffsetY;
	}

	if (rc.left > _rect.left) _offsetX = rc.left - _rect.left;
	if (rc.top  > _rect.top)  _offsetY = rc.top  - _rect.top;

	CBPlatform::IntersectRect(&_rect, &_rect, &rc);
}

} // end of namespace WinterMute
