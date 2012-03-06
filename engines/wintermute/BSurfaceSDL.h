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

#ifndef WINTERMUTE_BSURFACESDL_H
#define WINTERMUTE_BSURFACESDL_H

#include "BSurface.h"
class SDL_Texture;
class SDL_Surface;
namespace WinterMute {

class CBSurfaceSDL : public CBSurface {
public:
	CBSurfaceSDL(CBGame *inGame);
	~CBSurfaceSDL();

	HRESULT Create(char *Filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int LifeTime = -1, bool KeepLoaded = false);
	HRESULT Create(int Width, int Height);

	HRESULT CreateFromSDLSurface(SDL_Surface *surface);

	bool IsTransparentAt(int X, int Y);
	bool IsTransparentAtLite(int X, int Y);

	HRESULT StartPixelOp();
	HRESULT EndPixelOp();


	HRESULT DisplayTransZoom(int X, int Y, RECT rect, float ZoomX, float ZoomY, uint32 Alpha = 0xFFFFFFFF, TSpriteBlendMode BlendMode = BLEND_NORMAL, bool MirrorX = false, bool MirrorY = false);
	HRESULT DisplayTrans(int X, int Y, RECT rect, uint32 Alpha = 0xFFFFFFFF, TSpriteBlendMode BlendMode = BLEND_NORMAL, bool MirrorX = false, bool MirrorY = false);
	HRESULT DisplayTransOffset(int X, int Y, RECT rect, uint32 Alpha = 0xFFFFFFFF, TSpriteBlendMode BlendMode = BLEND_NORMAL, bool MirrorX = false, bool MirrorY = false, int offsetX = 0, int offsetY = 0);
	HRESULT Display(int X, int Y, RECT rect, TSpriteBlendMode BlendMode = BLEND_NORMAL, bool MirrorX = false, bool MirrorY = false);
	HRESULT DisplayZoom(int X, int Y, RECT rect, float ZoomX, float ZoomY, uint32 Alpha = 0xFFFFFFFF, bool Transparent = false, TSpriteBlendMode BlendMode = BLEND_NORMAL, bool MirrorX = false, bool MirrorY = false);
	HRESULT DisplayTransform(int X, int Y, int HotX, int HotY, RECT Rect, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode = BLEND_NORMAL, bool MirrorX = false, bool MirrorY = false);

/*	static unsigned DLL_CALLCONV ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	static int DLL_CALLCONV SeekProc(fi_handle handle, long offset, int origin);
	static long DLL_CALLCONV TellProc(fi_handle handle);*/

private:
	SDL_Texture *m_Texture;

	HRESULT DrawSprite(int X, int Y, RECT *Rect, float ZoomX, float ZoomY, uint32 Alpha, bool AlphaDisable, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY, int offsetX = 0, int offsetY = 0);
	void GenAlphaMask(SDL_Surface *surface);
	uint32 GetPixel(SDL_Surface *surface, int x, int y);

	void *m_LockPixels;
	int m_LockPitch;
	byte *m_AlphaMask;
};

} // end of namespace WinterMute

#endif // __WmeBSurfaceSDL_H__
