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

#include "dcgf.h"
#include "BActiveRect.h"
#include "BGame.h"
#include "BRegion.h"
#include "PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBActiveRect::CBActiveRect(CBGame *inGame): CBBase(inGame) {
	CBPlatform::SetRectEmpty(&m_Rect);
	m_Owner = NULL;
	m_Frame = NULL;
	m_Region = NULL;
	m_ZoomX = 100;
	m_ZoomY = 100;
	m_OffsetX = m_OffsetY = 0;
	ClipRect();
}


//////////////////////////////////////////////////////////////////////
CBActiveRect::CBActiveRect(CBGame *inGame, CBObject *Owner, CBSubFrame *Frame, int X, int Y, int Width, int Height, float ZoomX, float ZoomY, bool Precise): CBBase(inGame) {
	m_Owner = Owner;
	m_Frame = Frame;
	CBPlatform::SetRect(&m_Rect, X, Y, X + Width, Y + Height);
	m_ZoomX = ZoomX;
	m_ZoomY = ZoomY;
	m_Precise = Precise;
	m_Region = NULL;
	m_OffsetX = m_OffsetY = 0;
	ClipRect();
}

//////////////////////////////////////////////////////////////////////
CBActiveRect::CBActiveRect(CBGame *inGame, CBObject *Owner, CBRegion *Region, int OffsetX, int OffsetY): CBBase(inGame) {
	m_Owner = Owner;
	m_Region = Region;
	CBPlatform::CopyRect(&m_Rect, &Region->m_Rect);
	CBPlatform::OffsetRect(&m_Rect, -OffsetX, -OffsetY);
	m_ZoomX = 100;
	m_ZoomY = 100;
	m_Precise = true;
	m_Frame = NULL;
	ClipRect();
	m_OffsetX = OffsetX;
	m_OffsetY = OffsetY;
}


//////////////////////////////////////////////////////////////////////
CBActiveRect::~CBActiveRect() {
	m_Owner = NULL;
	m_Frame = NULL;
	m_Region = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBActiveRect::ClipRect() {
	RECT rc;
	bool CustomViewport;
	Game->GetCurrentViewportRect(&rc, &CustomViewport);
	CBRenderer *Rend = Game->m_Renderer;

	if (!CustomViewport) {
		rc.left -= Rend->m_DrawOffsetX;
		rc.right -= Rend->m_DrawOffsetX;
		rc.top -= Rend->m_DrawOffsetY;
		rc.bottom -= Rend->m_DrawOffsetY;
	}

	if (rc.left > m_Rect.left) m_OffsetX = rc.left - m_Rect.left;
	if (rc.top  > m_Rect.top)  m_OffsetY = rc.top  - m_Rect.top;

	CBPlatform::IntersectRect(&m_Rect, &m_Rect, &rc);
}

} // end of namespace WinterMute
