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

#include "wintypes.h"
#include "BGame.h"
#include "BSurface.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBSurface::CBSurface(CBGame *inGame): CBBase(inGame) {
	m_ReferenceCount = 0;

	m_Width = m_Height = 0;

	m_Filename = NULL;

	m_PixelOpReady = false;

	m_CKDefault = true;
	m_CKRed = m_CKGreen = m_CKBlue = 0;
	m_LifeTime = 0;
	m_KeepLoaded = false;

	m_LastUsedTime = 0;
	m_Valid = false;
}


//////////////////////////////////////////////////////////////////////
CBSurface::~CBSurface() {
	if (m_PixelOpReady) EndPixelOp();
	if (m_Filename) delete [] m_Filename;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::Create(char *Filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int LifeTime, bool KeepLoaded) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::Restore() {
	return E_FAIL;
}




//////////////////////////////////////////////////////////////////////
bool CBSurface::IsTransparentAt(int X, int Y) {
	return false;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::Display(int X, int Y, RECT rect, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::DisplayTrans(int X, int Y, RECT rect, uint32 Alpha, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::DisplayTransOffset(int X, int Y, RECT rect, uint32 Alpha, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY, int offsetX, int offsetY) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::DisplayTransZoom(int X, int Y, RECT rect, float ZoomX, float ZoomY, uint32 Alpha, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::DisplayZoom(int X, int Y, RECT rect, float ZoomX, float ZoomY, uint32 Alpha, bool Transparent, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurface::DisplayHalfTrans(int X, int Y, RECT rect) {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::DisplayTransform(int X, int Y, int HotX, int HotY, RECT Rect, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return DisplayTransZoom(X, Y, Rect, ZoomX, ZoomY, Alpha, BlendMode, MirrorX, MirrorY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::Create(int Width, int Height) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::StartPixelOp() {
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::EndPixelOp() {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::GetPixel(int X, int Y, byte *R, byte *G, byte *B, byte *A) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::PutPixel(int X, int Y, byte R, byte G, byte B, int A) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
bool CBSurface::ComparePixel(int X, int Y, byte R, byte G, byte B, int A) {
	return false;
}


//////////////////////////////////////////////////////////////////////
bool CBSurface::IsTransparentAtLite(int X, int Y) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::Invalidate() {
	return E_FAIL;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBSurface::PrepareToDraw() {
	m_LastUsedTime = Game->m_LiveTimer;

	if (!m_Valid) {
		//Game->LOG(0, "Reviving: %s", m_Filename);
		return Create(m_Filename, m_CKDefault, m_CKRed, m_CKGreen, m_CKBlue, m_LifeTime, m_KeepLoaded);
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBSurface::SetFilename(const char *Filename) {
	delete[] m_Filename;
	m_Filename = NULL;
	if (!Filename) return;

	m_Filename = new char[strlen(Filename) + 1];
	if (m_Filename) strcpy(m_Filename, Filename);
}

//////////////////////////////////////////////////////////////////////////
void CBSurface::SetSize(int Width, int Height) {
	m_Width = Width;
	m_Height = Height;
}

} // end of namespace WinterMute
