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
#include "BRenderer.h"
#include "BSurface.h"
#include "BSubFrame.h"
#include "BRegion.h"
#include "PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBRenderer::CBRenderer(CBGame *inGame): CBBase(inGame) {
	m_Instance = NULL;
	m_Window = NULL;
	m_ClipperWindow = NULL;
	m_Active = false;
	m_Ready = false;
	m_Windowed = true;
	m_ForceAlphaColor = 0x00;

	m_Width = m_Height = m_BPP = 0;
	CBPlatform::SetRectEmpty(&m_MonitorRect);

	m_RealWidth = m_RealHeight = 0;
	m_DrawOffsetX = m_DrawOffsetY = 0;
}


//////////////////////////////////////////////////////////////////////
CBRenderer::~CBRenderer() {
	DeleteRectList();
	UnclipCursor();
}


//////////////////////////////////////////////////////////////////////
void CBRenderer::InitLoop() {
	DeleteRectList();
}


//////////////////////////////////////////////////////////////////////
CBObject *CBRenderer::GetObjectAt(int X, int Y) {
	POINT point;
	point.x = X;
	point.y = Y;

	for (int i = m_RectList.GetSize() - 1; i >= 0; i--) {
		if (CBPlatform::PtInRect(&m_RectList[i]->m_Rect, point)) {
			if (m_RectList[i]->m_Precise) {
				// frame
				if (m_RectList[i]->m_Frame) {
					int XX = (int)((m_RectList[i]->m_Frame->m_Rect.left + X - m_RectList[i]->m_Rect.left + m_RectList[i]->m_OffsetX) / (float)((float)m_RectList[i]->m_ZoomX / (float)100));
					int YY = (int)((m_RectList[i]->m_Frame->m_Rect.top  + Y - m_RectList[i]->m_Rect.top  + m_RectList[i]->m_OffsetY) / (float)((float)m_RectList[i]->m_ZoomY / (float)100));

					if (m_RectList[i]->m_Frame->m_MirrorX) {
						int Width = m_RectList[i]->m_Frame->m_Rect.right - m_RectList[i]->m_Frame->m_Rect.left;
						XX = Width - XX;
					}

					if (m_RectList[i]->m_Frame->m_MirrorY) {
						int Height = m_RectList[i]->m_Frame->m_Rect.bottom - m_RectList[i]->m_Frame->m_Rect.top;
						YY = Height - YY;
					}

					if (!m_RectList[i]->m_Frame->m_Surface->IsTransparentAt(XX, YY)) return m_RectList[i]->m_Owner;
				}
				// region
				else if (m_RectList[i]->m_Region) {
					if (m_RectList[i]->m_Region->PointInRegion(X + m_RectList[i]->m_OffsetX, Y + m_RectList[i]->m_OffsetY)) return m_RectList[i]->m_Owner;
				}
			} else return m_RectList[i]->m_Owner;
		}
	}

	return (CBObject *)NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBRenderer::DeleteRectList() {
	for (int i = 0; i < m_RectList.GetSize(); i++) {
		delete m_RectList[i];
	}
	m_RectList.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SwitchFullscreen() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::Flip() {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::InitRenderer(int width, int height, bool windowed) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
void CBRenderer::OnWindowChange() {
}


//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::Fill(byte  r, byte g, byte b, RECT *rect) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::WindowedBlt() {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::Setup2D(bool Force) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetupLines() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::DrawLine(int X1, int Y1, int X2, int Y2, uint32 Color) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::DrawRect(int X1, int Y1, int X2, int Y2, uint32 Color, int Width) {
	for (int i = 0; i < Width; i++) {
		DrawLine(X1 + i, Y1 + i, X2 - i,   Y1 + i,   Color); // up
		DrawLine(X1 + i, Y2 - i, X2 - i + 1, Y2 - i, Color); // down

		DrawLine(X1 + i, Y1 + i, X1 + i, Y2 - i,   Color); // left
		DrawLine(X2 - i, Y1 + i, X2 - i, Y2 - i + 1, Color); // right
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::Fade(WORD Alpha) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::FadeToColor(uint32 Color, RECT *rect) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetViewport(int left, int top, int right, int bottom) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetScreenViewport() {
	return SetViewport(m_DrawOffsetX, m_DrawOffsetY, m_Width + m_DrawOffsetX, m_Height + m_DrawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetViewport(RECT *Rect) {
	return SetViewport(Rect->left + m_DrawOffsetX,
	                   Rect->top + m_DrawOffsetY,
	                   Rect->right + m_DrawOffsetX,
	                   Rect->bottom + m_DrawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
CBImage *CBRenderer::TakeScreenshot() {
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::ClipCursor() {
	/*
	if(!m_Windowed)
	{
	    RECT rc;
	    GetWindowRect(m_Window, &rc);

	    // if "maintain aspect ratio" is in effect, lock mouse to visible area
	    rc.left = m_DrawOffsetX;
	    rc.top = m_DrawOffsetY;
	    rc.right = rc.left + m_Width;
	    rc.bottom = rc.top + m_Height;

	    ::ClipCursor(&rc);
	}
	 */
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::UnclipCursor() {
	/*
	if(!m_Windowed) ::ClipCursor(NULL);
	*/
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBRenderer::PointInViewport(POINT *P) {
	if (P->x < m_DrawOffsetX) return false;
	if (P->y < m_DrawOffsetY) return false;
	if (P->x > m_DrawOffsetX + m_Width) return false;
	if (P->y > m_DrawOffsetY + m_Height) return false;

	return true;
}

} // end of namespace WinterMute
