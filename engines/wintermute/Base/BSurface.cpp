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
#include "engines/wintermute/Base/BSurface.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBSurface::CBSurface(CBGame *inGame): CBBase(inGame) {
	_referenceCount = 0;

	_width = _height = 0;

	_filename = NULL;

	_pixelOpReady = false;

	_cKDefault = true;
	_cKRed = _cKGreen = _cKBlue = 0;
	_lifeTime = 0;
	_keepLoaded = false;

	_lastUsedTime = 0;
	_valid = false;
}


//////////////////////////////////////////////////////////////////////
CBSurface::~CBSurface() {
	if (_pixelOpReady) endPixelOp();
	if (_filename) delete [] _filename;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::create(const char *filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int lifeTime, bool keepLoaded) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::restore() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////
bool CBSurface::isTransparentAt(int x, int y) {
	return false;
}

//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::displayHalfTrans(int x, int y, RECT rect) {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::displayTransform(int x, int y, int hotX, int hotY, RECT rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return displayTransZoom(x, y, rect, zoomX, zoomY, alpha, blendMode, mirrorX, mirrorY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::create(int Width, int Height) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::startPixelOp() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::endPixelOp() {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::putPixel(int x, int y, byte r, byte g, byte b, int a) {
	return E_FAIL;
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
HRESULT CBSurface::invalidate() {
	return E_FAIL;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::prepareToDraw() {
	_lastUsedTime = Game->_liveTimer;

	if (!_valid) {
		//Game->LOG(0, "Reviving: %s", _filename);
		return create(_filename, _cKDefault, _cKRed, _cKGreen, _cKBlue, _lifeTime, _keepLoaded);
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBSurface::setFilename(const char *filename) {
	delete[] _filename;
	_filename = NULL;
	if (!filename) return;

	_filename = new char[strlen(filename) + 1];
	if (_filename) strcpy(_filename, filename);
}

//////////////////////////////////////////////////////////////////////////
void CBSurface::setSize(int Width, int Height) {
	_width = Width;
	_height = Height;
}

} // end of namespace WinterMute
