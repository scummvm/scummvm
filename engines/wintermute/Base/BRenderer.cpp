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
	_instance = 0;
	_window = 0;
	_clipperWindow = 0;
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
	deleteRectList();
	unclipCursor();
}


//////////////////////////////////////////////////////////////////////
void CBRenderer::initLoop() {
	deleteRectList();
}


//////////////////////////////////////////////////////////////////////
CBObject *CBRenderer::getObjectAt(int x, int y) {
	POINT point;
	point.x = x;
	point.y = y;

	for (int i = _rectList.GetSize() - 1; i >= 0; i--) {
		if (CBPlatform::PtInRect(&_rectList[i]->_rect, point)) {
			if (_rectList[i]->_precise) {
				// frame
				if (_rectList[i]->_frame) {
					int xx = (int)((_rectList[i]->_frame->_rect.left + x - _rectList[i]->_rect.left + _rectList[i]->_offsetX) / (float)((float)_rectList[i]->_zoomX / (float)100));
					int yy = (int)((_rectList[i]->_frame->_rect.top  + y - _rectList[i]->_rect.top  + _rectList[i]->_offsetY) / (float)((float)_rectList[i]->_zoomY / (float)100));

					if (_rectList[i]->_frame->_mirrorX) {
						int width = _rectList[i]->_frame->_rect.right - _rectList[i]->_frame->_rect.left;
						xx = width - xx;
					}

					if (_rectList[i]->_frame->_mirrorY) {
						int height = _rectList[i]->_frame->_rect.bottom - _rectList[i]->_frame->_rect.top;
						yy = height - yy;
					}

					if (!_rectList[i]->_frame->_surface->isTransparentAt(xx, yy)) return _rectList[i]->_owner;
				}
				// region
				else if (_rectList[i]->_region) {
					if (_rectList[i]->_region->pointInRegion(x + _rectList[i]->_offsetX, y + _rectList[i]->_offsetY)) return _rectList[i]->_owner;
				}
			} else return _rectList[i]->_owner;
		}
	}

	return (CBObject *)NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBRenderer::deleteRectList() {
	for (int i = 0; i < _rectList.GetSize(); i++) {
		delete _rectList[i];
	}
	_rectList.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::switchFullscreen() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::flip() {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::initRenderer(int width, int height, bool windowed) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
void CBRenderer::onWindowChange() {
}


//////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::fill(byte  r, byte g, byte b, Common::Rect *rect) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::windowedBlt() {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::setup2D(bool Force) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::setupLines() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::drawRect(int x1, int y1, int x2, int y2, uint32 color, int width) {
	for (int i = 0; i < width; i++) {
		drawLine(x1 + i, y1 + i, x2 - i,   y1 + i,   color); // up
		drawLine(x1 + i, y2 - i, x2 - i + 1, y2 - i, color); // down

		drawLine(x1 + i, y1 + i, x1 + i, y2 - i,   color); // left
		drawLine(x2 - i, y1 + i, x2 - i, y2 - i + 1, color); // right
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::fade(uint16 alpha) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::fadeToColor(uint32 color, Common::Rect *rect) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::setViewport(int left, int top, int right, int bottom) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::setScreenViewport() {
	return setViewport(_drawOffsetX, _drawOffsetY, _width + _drawOffsetX, _height + _drawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::setViewport(RECT *rect) {
	return setViewport(rect->left + _drawOffsetX,
	                   rect->top + _drawOffsetY,
	                   rect->right + _drawOffsetX,
	                   rect->bottom + _drawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
CBImage *CBRenderer::takeScreenshot() {
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderer::clipCursor() {
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
HRESULT CBRenderer::unclipCursor() {
	/*
	if (!_windowed) ::ClipCursor(NULL);
	*/
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBRenderer::pointInViewport(POINT *p) {
	if (p->x < _drawOffsetX) return false;
	if (p->y < _drawOffsetY) return false;
	if (p->x > _drawOffsetX + _width) return false;
	if (p->y > _drawOffsetY + _height) return false;

	return true;
}

} // end of namespace WinterMute
