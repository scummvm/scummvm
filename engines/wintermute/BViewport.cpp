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

#include "BGame.h"
#include "PlatformSDL.h"
#include "BViewport.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBViewport, false)

//////////////////////////////////////////////////////////////////////////
CBViewport::CBViewport(CBGame *inGame): CBBase(inGame) {
	CBPlatform::SetRectEmpty(&m_Rect);
	m_MainObject = NULL;
	m_OffsetX = m_OffsetY = 0;
}


//////////////////////////////////////////////////////////////////////////
CBViewport::~CBViewport() {

}


//////////////////////////////////////////////////////////////////////////
HRESULT CBViewport::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(m_MainObject));
	PersistMgr->Transfer(TMEMBER(m_OffsetX));
	PersistMgr->Transfer(TMEMBER(m_OffsetY));
	PersistMgr->Transfer(TMEMBER(m_Rect));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBViewport::SetRect(int left, int top, int right, int bottom, bool NoCheck) {
	if (!NoCheck) {
		left = std::max(left, 0);
		top = std::max(top, 0);
		right = std::min(right, Game->m_Renderer->m_Width);
		bottom = std::min(bottom, Game->m_Renderer->m_Height);
	}

	CBPlatform::SetRect(&m_Rect, left, top, right, bottom);
	m_OffsetX = left;
	m_OffsetY = top;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
RECT *CBViewport::GetRect() {
	return &m_Rect;
}


//////////////////////////////////////////////////////////////////////////
int CBViewport::GetWidth() {
	return m_Rect.right - m_Rect.left;
}


//////////////////////////////////////////////////////////////////////////
int CBViewport::GetHeight() {
	return m_Rect.bottom - m_Rect.top;
}

} // end of namespace WinterMute
