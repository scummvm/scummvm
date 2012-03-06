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
#include "BFile.h"
#include "BGame.h"
#include "BSurfaceSDL.h"
#include "BRenderSDL.h"
#include "SdlUtil.h"
#include "graphics/decoders/png.h"
#include "graphics/decoders/bmp.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "stream.h"
#include "BFileManager.h"
#include "PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBSurfaceSDL::CBSurfaceSDL(CBGame *inGame) : CBSurface(inGame) {
	m_Texture = NULL;
	m_AlphaMask = NULL;

	m_LockPixels = NULL;
	m_LockPitch = 0;
}

//////////////////////////////////////////////////////////////////////////
CBSurfaceSDL::~CBSurfaceSDL() {
	if (m_Texture) SDL_DestroyTexture(m_Texture);
	delete[] m_AlphaMask;
	m_AlphaMask = NULL;

	Game->AddMem(-m_Width * m_Height * 4);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::Create(char *Filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int LifeTime, bool KeepLoaded) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->m_Renderer);
	Common::String strFileName(Filename);
	
	Graphics::ImageDecoder *imgDecoder;
	
	if (strFileName.hasSuffix(".png")) {
		imgDecoder = new Graphics::PNGDecoder();
	} else if (strFileName.hasSuffix(".bmp")) {
		imgDecoder = new Graphics::BitmapDecoder();
	} else {
		error("CBSurfaceSDL::Create : Unsupported fileformat %s", Filename);
	}
	
	CBFile *file = Game->m_FileManager->OpenFile(Filename);
	if (!file) return E_FAIL;
	
	imgDecoder->loadStream(*file->getMemStream());
	const Graphics::Surface *surface = imgDecoder->getSurface();
	Game->m_FileManager->CloseFile(file);

	if (default_ck) {
		ck_red   = 255;
		ck_green = 0;
		ck_blue  = 255;
	}

	m_Width = surface->w;
	m_Height = surface->h;

	bool isSaveGameGrayscale = scumm_strnicmp(Filename, "savegame:", 9) == 0 && (Filename[strFileName.size() - 1] == 'g' || Filename[strFileName.size() - 1] == 'G');
	if (isSaveGameGrayscale) {
		warning("grayscaleConversion not yet implemented");
/*		FIBITMAP *newImg = FreeImage_ConvertToGreyscale(img);
		if (newImg) {
			FreeImage_Unload(img);
			img = newImg;
		}*/
	}

	// convert 32-bit BMPs to 24-bit or they appear totally transparent (does any app actually write alpha in BMP properly?)
/*	if (FreeImage_GetBPP(img) != 32 || (imgFormat == FIF_BMP && FreeImage_GetBPP(img) != 24)) {
		FIBITMAP *newImg = FreeImage_ConvertTo24Bits(img);
		if (newImg) {
			FreeImage_Unload(img);
			img = newImg;
		} else {
			FreeImage_Unload(img);
			return -1;
		}
	}

	FreeImage_FlipVertical(img);*/

	//TODO: This is rather endian-specific, but should be replaced by non-SDL-code anyhow:
	uint32 rmask = surface->format.rMax() << surface->format.rShift;
	uint32 gmask = surface->format.gMax() << surface->format.gShift;
	uint32 bmask = surface->format.bMax() << surface->format.bShift;
	uint32 amask = surface->format.aMax();
	
	SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(surface->pixels, m_Width, m_Height, surface->format.bytesPerPixel * 8, surface->pitch, rmask, gmask, bmask, amask);

	// no alpha, set color key
	if (surface->format.bytesPerPixel != 4)
		SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, ck_red, ck_green, ck_blue));

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
	//m_Texture = SdlUtil::CreateTextureFromSurface(renderer->GetSdlRenderer(), surf);
	m_Texture = SDL_CreateTextureFromSurface(renderer->GetSdlRenderer(), surf);
	if (!m_Texture) {
		SDL_FreeSurface(surf);
		delete imgDecoder;
		return E_FAIL;
	}

	GenAlphaMask(surf);

	SDL_FreeSurface(surf);
	delete imgDecoder; // TODO: Update this if ImageDecoder doesn't end up owning the surface.

	m_CKDefault = default_ck;
	m_CKRed = ck_red;
	m_CKGreen = ck_green;
	m_CKBlue = ck_blue;


	if (!m_Filename || scumm_stricmp(m_Filename, Filename) != 0) {
		SetFilename(Filename);
	}

	if (m_LifeTime == 0 || LifeTime == -1 || LifeTime > m_LifeTime)
		m_LifeTime = LifeTime;

	m_KeepLoaded = KeepLoaded;
	if (m_KeepLoaded) m_LifeTime = -1;

	m_Valid = true;

	Game->AddMem(m_Width * m_Height * 4);


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBSurfaceSDL::GenAlphaMask(SDL_Surface *surface) {
	delete[] m_AlphaMask;
	m_AlphaMask = NULL;
	if (!surface) return;

	SDL_LockSurface(surface);

	bool hasColorKey;
	Uint32 colorKey;
	Uint8 ckRed, ckGreen, ckBlue;
	if (SDL_GetColorKey(surface, &colorKey) == 0) {
		hasColorKey = true;
		SDL_GetRGB(colorKey, surface->format, &ckRed, &ckGreen, &ckBlue);
	} else hasColorKey = false;

	m_AlphaMask = new byte[surface->w * surface->h];

	bool hasTransparency = false;
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			Uint32 pixel = GetPixel(surface, x, y);

			Uint8 r, g, b, a;
			SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

			if (hasColorKey && r == ckRed && g == ckGreen && b == ckBlue)
				a = 0;

			m_AlphaMask[y * surface->w + x] = a;
			if (a < 255) hasTransparency = true;
		}
	}

	SDL_UnlockSurface(surface);

	if (!hasTransparency) {
		delete[] m_AlphaMask;
		m_AlphaMask = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
Uint32 CBSurfaceSDL::GetPixel(SDL_Surface *surface, int x, int y) {
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *)p;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::Create(int Width, int Height) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->m_Renderer);
	m_Texture = SDL_CreateTexture(renderer->GetSdlRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);

	m_Width = Width;
	m_Height = Height;

	Game->AddMem(m_Width * m_Height * 4);

	m_Valid = true;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::CreateFromSDLSurface(SDL_Surface *surface) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->m_Renderer);
	m_Texture = SDL_CreateTextureFromSurface(renderer->GetSdlRenderer(), surface);

	m_Width = surface->w;
	m_Height = surface->h;

	Game->AddMem(m_Width * m_Height * 4);

	m_Valid = true;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSurfaceSDL::IsTransparentAt(int X, int Y) {
	int access;
	int width, height;
	SDL_QueryTexture(m_Texture, NULL, &access, &width, &height);
	//if (access != SDL_TEXTUREACCESS_STREAMING) return false;
	if (X < 0 || X >= width || Y < 0 || Y >= height) return true;


	StartPixelOp();
	bool ret = IsTransparentAtLite(X, Y);
	EndPixelOp();

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool CBSurfaceSDL::IsTransparentAtLite(int X, int Y) {
	//if (!m_LockPixels) return false;

	Uint32 format;
	int access;
	int width, height;
	SDL_QueryTexture(m_Texture, &format, &access, &width, &height);
	//if (access != SDL_TEXTUREACCESS_STREAMING) return false;
	if (X < 0 || X >= width || Y < 0 || Y >= height) return true;

	if (!m_AlphaMask) return false;
	else return m_AlphaMask[Y * width + X] <= 128;

	/*
	Uint32* dst = (Uint32*)((Uint8*)m_LockPixels + Y * m_LockPitch);
	Uint32 pixel = dst[X];

	SDL_PixelFormat* pixelFormat = SDL_AllocFormat(format);
	Uint8 r, g, b, a;
	SDL_GetRGBA(pixel, pixelFormat, &r, &g, &b, &a);
	SDL_FreeFormat(pixelFormat);

	return a <= 128;
	*/
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::StartPixelOp() {
	//SDL_LockTexture(m_Texture, NULL, &m_LockPixels, &m_LockPitch);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::EndPixelOp() {
	//SDL_UnlockTexture(m_Texture);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::Display(int X, int Y, RECT rect, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return DrawSprite(X, Y, &rect, 100, 100, 0xFFFFFFFF, true, BlendMode, MirrorX, MirrorY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::DisplayTrans(int X, int Y, RECT rect, uint32 Alpha, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return DrawSprite(X, Y, &rect, 100, 100, Alpha, false, BlendMode, MirrorX, MirrorY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::DisplayTransOffset(int X, int Y, RECT rect, uint32 Alpha, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY, int offsetX, int offsetY) {
	return DrawSprite(X, Y, &rect, 100, 100, Alpha, false, BlendMode, MirrorX, MirrorY, offsetX, offsetY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::DisplayTransZoom(int X, int Y, RECT rect, float ZoomX, float ZoomY, uint32 Alpha, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return DrawSprite(X, Y, &rect, ZoomX, ZoomY, Alpha, false, BlendMode, MirrorX, MirrorY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::DisplayZoom(int X, int Y, RECT rect, float ZoomX, float ZoomY, uint32 Alpha, bool Transparent, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return DrawSprite(X, Y, &rect, ZoomX, ZoomY, Alpha, !Transparent, BlendMode, MirrorX, MirrorY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::DisplayTransform(int X, int Y, int HotX, int HotY, RECT Rect, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY) {
	return DrawSprite(X, Y, &Rect, ZoomX, ZoomY, Alpha, false, BlendMode, MirrorX, MirrorY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::DrawSprite(int X, int Y, RECT *Rect, float ZoomX, float ZoomY, uint32 Alpha, bool AlphaDisable, TSpriteBlendMode BlendMode, bool MirrorX, bool MirrorY, int offsetX, int offsetY) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->m_Renderer);

	if (renderer->m_ForceAlphaColor != 0) Alpha = renderer->m_ForceAlphaColor;

	byte r = D3DCOLGetR(Alpha);
	byte g = D3DCOLGetG(Alpha);
	byte b = D3DCOLGetB(Alpha);
	byte a = D3DCOLGetA(Alpha);

	SDL_SetTextureColorMod(m_Texture, r, g, b);
	SDL_SetTextureAlphaMod(m_Texture, a);

	if (AlphaDisable)
		SDL_SetTextureBlendMode(m_Texture, SDL_BLENDMODE_NONE);
	else
		SDL_SetTextureBlendMode(m_Texture, SDL_BLENDMODE_BLEND);

	SDL_Rect srcRect;
	srcRect.x = Rect->left;
	srcRect.y = Rect->top;
	srcRect.w = Rect->right - Rect->left;
	srcRect.h = Rect->bottom - Rect->top;

	SDL_Rect position;
	position.x = X;
	position.y = Y;
	position.w = (float)srcRect.w * ZoomX / 100.f;
	position.h = (float)srcRect.h * ZoomX / 100.f;

	renderer->ModTargetRect(&position);

	position.x += offsetX;
	position.y += offsetY;

	SDL_RenderCopy(renderer->GetSdlRenderer(), m_Texture, &srcRect, &position);


	return S_OK;
}

} // end of namespace WinterMute
