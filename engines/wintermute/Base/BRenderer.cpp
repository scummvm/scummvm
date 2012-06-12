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
#include "engines/wintermute/Base/BRenderer.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBRenderer::CBRenderer(CBGame *inGame): CBBase(inGame) {
	_instance = NULL;
	_window = NULL;
	_clipperWindow = NULL;
	_active = false;
	_ready = false;
	_windowed = true;
	_forceAlphaColor = 0x00;

	_width = _height = _bPP = 0;
	CBPlatform::SetRectEmpty(&_monitorRect);

	_realWidth = _realHeight = 0;
	_drawOffsetX = _drawOffsetY = 0;
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

	for (int i = _rectList.GetSize() - 1; i >= 0; i--) {
		if (CBPlatform::PtInRect(&_rectList[i]->_rect, point)) {
			if (_rectList[i]->_precise) {
				// frame
				if (_rectList[i]->_frame) {
					int XX = (int)((_rectList[i]->_frame->_rect.left + X - _rectList[i]->_rect.left + _rectList[i]->_offsetX) / (float)((float)_rectList[i]->_zoomX / (float)100));
					int YY = (int)((_rectList[i]->_frame->_rect.top  + Y - _rectList[i]->_rect.top  + _rectList[i]->_offsetY) / (float)((float)_rectList[i]->_zoomY / (float)100));

					if (_rectList[i]->_frame->_mirrorX) {
						int Width = _rectList[i]->_frame->_rect.right - _rectList[i]->_frame->_rect.left;
						XX = Width - XX;
					}

					if (_rectList[i]->_frame->_mirrorY) {
						int Height = _rectList[i]->_frame->_rect.bottom - _rectList[i]->_frame->_rect.top;
						YY = Height - YY;
					}

					if (!_rectList[i]->_frame->_surface->isTransparentAt(XX, YY)) return _rectList[i]->_owner;
				}
				// region
				else if (_rectList[i]->_region) {
					if (_rectList[i]->_region->PointInRegion(X + _rectList[i]->_offsetX, Y + _rectList[i]->_offsetY)) return _rectList[i]->_owner;
				}
			} else return _rectList[i]->_owner;
		}
	}

	return (CBObject *)NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBRenderer::DeleteRectList() {
	for (int i = 0; i < _rectList.GetSize(); i++) {
		delete _rectList[i];
	}
	_rectList.RemoveAll();
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
HRESULT CBRenderer::Fill(byte  r, byte g, byte b, Common::Rect *rect) {
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
HRESULT CBRenderer::Fade(uint16 Alpha) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::FadeToColor(uint32 Color, Common::Rect *rect) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetViewport(int left, int top, int right, int bottom) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetScreenViewport() {
	return SetViewport(_drawOffsetX, _drawOffsetY, _width + _drawOffsetX, _height + _drawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::SetViewport(RECT *Rect) {
	return SetViewport(Rect->left + _drawOffsetX,
	                   Rect->top + _drawOffsetY,
	                   Rect->right + _drawOffsetX,
	                   Rect->bottom + _drawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
CBImage *CBRenderer::TakeScreenshot() {
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::ClipCursor() {
	/*
	if (!_windowed) {
	    RECT rc;
	    GetWindowRect(_window, &rc);

	    // if "maintain aspect ratio" is in effect, lock mouse to visible area
	    rc.left = _drawOffsetX;
	    rc.top = _drawOffsetY;
	    rc.right = rc.left + _width;
	    rc.bottom = rc.top + _height;

	    ::ClipCursor(&rc);
	}
	 */
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::UnclipCursor() {
	/*
	if (!_windowed) ::ClipCursor(NULL);
	*/
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBRenderer::PointInViewport(POINT *P) {
	if (P->x < _drawOffsetX) return false;
	if (P->y < _drawOffsetY) return false;
	if (P->x > _drawOffsetX + _width) return false;
	if (P->y > _drawOffsetY + _height) return false;

	return true;
}

} // end of namespace WinterMute
