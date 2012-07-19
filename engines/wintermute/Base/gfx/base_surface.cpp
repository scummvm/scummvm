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

#include "engines/wintermute/wintypes.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/gfx/base_surface.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBSurface::CBSurface(CBGame *inGame): CBBase(inGame) {
	_referenceCount = 0;

	_width = _height = 0;

	_filename = "";

	_pixelOpReady = false;

	_ckDefault = true;
	_ckRed = _ckGreen = _ckBlue = 0;
	_lifeTime = 0;
	_keepLoaded = false;

	_lastUsedTime = 0;
	_valid = false;
}


//////////////////////////////////////////////////////////////////////
CBSurface::~CBSurface() {
	if (_pixelOpReady) endPixelOp();
}


//////////////////////////////////////////////////////////////////////
bool CBSurface::create(const char *filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////
bool CBSurface::restore() {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////
bool CBSurface::isTransparentAt(int x, int y) {
	return false;
}

//////////////////////////////////////////////////////////////////////
bool CBSurface::displayHalfTrans(int x, int y, Rect32 rect) {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool CBSurface::displayTransform(int x, int y, int hotX, int hotY, Rect32 rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return displayTransZoom(x, y, rect, zoomX, zoomY, alpha, blendMode, mirrorX, mirrorY);
}

//////////////////////////////////////////////////////////////////////////
bool CBSurface::create(int Width, int Height) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool CBSurface::startPixelOp() {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool CBSurface::endPixelOp() {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool CBSurface::getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool CBSurface::putPixel(int x, int y, byte r, byte g, byte b, int a) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool CBSurface::comparePixel(int x, int y, byte r, byte g, byte b, int a) {
	return false;
}


//////////////////////////////////////////////////////////////////////
bool CBSurface::isTransparentAtLite(int x, int y) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBSurface::invalidate() {
	return STATUS_FAILED;
}



//////////////////////////////////////////////////////////////////////////
bool CBSurface::prepareToDraw() {
	_lastUsedTime = _gameRef->_liveTimer;

	if (!_valid) {
		//_gameRef->LOG(0, "Reviving: %s", _filename);
		return create(_filename.c_str(), _ckDefault, _ckRed, _ckGreen, _ckBlue, _lifeTime, _keepLoaded);
	} else return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBSurface::setSize(int width, int height) {
	_width = width;
	_height = height;
}

} // end of namespace WinterMute
