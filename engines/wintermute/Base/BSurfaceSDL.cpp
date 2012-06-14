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

#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/Base/BRenderSDL.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/PlatformSDL.h"
#include "graphics/decoders/png.h"
#include "graphics/decoders/bmp.h"
#include "graphics/decoders/jpeg.h"
#include "engines/wintermute/graphics/transparentSurface.h"
#include "engines/wintermute/graphics/tga.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "common/stream.h"
#include "common/system.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBSurfaceSDL::CBSurfaceSDL(CBGame *inGame) : CBSurface(inGame) {
	_surface = new Graphics::Surface();
	_scaledSurface = NULL;
	_alphaMask = NULL;
	_hasAlpha = true;

	_lockPixels = NULL;
	_lockPitch = 0;
}

//////////////////////////////////////////////////////////////////////////
CBSurfaceSDL::~CBSurfaceSDL() {
	//TODO
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = NULL;
	}

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = NULL;
	}
#if 0
	if (_texture) SDL_DestroyTexture(_texture);
	delete[] _alphaMask;
	_alphaMask = NULL;
#endif
	Game->AddMem(-_width * _height * 4);
}

bool hasTransparency(Graphics::Surface *surf) {
	if (surf->format.bytesPerPixel != 4) {
		warning("hasTransparency:: non 32 bpp surface passed as argument");
		return false;
	}
	uint8 r,g,b,a;
	for (int i = 0; i < surf->h; i++) {
		for (int j = 0; j < surf->w; j++) {
			uint32 pix = *(uint32*)surf->getBasePtr(j, i);
			surf->format.colorToARGB(pix, a, r, g, b);
			if (a != 255) {
				return true;
			}	
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::create(const char *filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int lifeTime, bool keepLoaded) {
/*	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer); */
	Common::String strFileName(filename);
	warning("CBSurfaceSDL::create(%s, %d, %d, %d, %d, %d, %d", filename, default_ck, ck_red, ck_green, ck_blue, lifeTime, keepLoaded);
	CBImage *image = new CBImage(Game);
	image->loadFile(strFileName);
//	const Graphics::Surface *surface = image->getSurface();

	if (default_ck) {
		ck_red   = 255;
		ck_green = 0;
		ck_blue  = 255;
	}

	_width = image->getSurface()->w;
	_height = image->getSurface()->h;

	bool isSaveGameGrayscale = scumm_strnicmp(filename, "savegame:", 9) == 0 && (filename[strFileName.size() - 1] == 'g' || filename[strFileName.size() - 1] == 'G');
	if (isSaveGameGrayscale) {
		warning("grayscaleConversion not yet implemented");
		/*      FIBITMAP *newImg = FreeImage_ConvertToGreyscale(img);
		        if (newImg) {
		            FreeImage_Unload(img);
		            img = newImg;
		        }*/
	}

	// convert 32-bit BMPs to 24-bit or they appear totally transparent (does any app actually write alpha in BMP properly?)
	/*  if (FreeImage_GetBPP(img) != 32 || (imgFormat == FIF_BMP && FreeImage_GetBPP(img) != 24)) {
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
/*	uint32 rmask = surface->format.rMax() << surface->format.rShift;
	uint32 gmask = surface->format.gMax() << surface->format.gShift;
	uint32 bmask = surface->format.bMax() << surface->format.bShift;
	uint32 amask = surface->format.aMax();*/

//	SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(surface->pixels, _width, _height, surface->format.bytesPerPixel * 8, surface->pitch, rmask, gmask, bmask, amask);

	// no alpha, set color key
	/*  if (surface->format.bytesPerPixel != 4)
	        SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, ck_red, ck_green, ck_blue));*/

	// convert 32-bit BMPs to 24-bit or they appear totally transparent (does any app actually write alpha in BMP properly?)
	// Well, actually, we don't convert via 24-bit as the color-key application overwrites the Alpha-channel anyhow.
	if (strFileName.hasSuffix(".bmp") && image->getSurface()->format.bytesPerPixel == 4) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat(), image->getPalette());
		TransparentSurface trans(*_surface);
		trans.applyColorKey(ck_red, ck_green, ck_blue);
	} else if (image->getSurface()->format.bytesPerPixel == 1 && image->getPalette()) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat(), image->getPalette());
		TransparentSurface trans(*_surface);
		trans.applyColorKey(ck_red, ck_green, ck_blue, true);
	} else if (image->getSurface()->format.bytesPerPixel == 4 && image->getSurface()->format != g_system->getScreenFormat()) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat());
	} else {
		_surface = new Graphics::Surface();
		_surface->copyFrom(*image->getSurface());
	}
	
	_hasAlpha = hasTransparency(_surface);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best"); //TODO
	//_texture = SdlUtil::CreateTextureFromSurface(renderer->GetSdlRenderer(), surf);

	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("Surface-textures not fully ported yet");
		hasWarned = true;
	}
	//delete imgDecoder;
#if 0
	_texture = SDL_CreateTextureFromSurface(renderer->GetSdlRenderer(), surf);
	if (!_texture) {
		SDL_FreeSurface(surf);
		delete imgDecoder;
		return E_FAIL;
	}

	GenAlphaMask(surf);

	SDL_FreeSurface(surf);
	delete imgDecoder; // TODO: Update this if ImageDecoder doesn't end up owning the surface.

	_cKDefault = default_ck;
	_cKRed = ck_red;
	_cKGreen = ck_green;
	_cKBlue = ck_blue;
#endif

	if (!_filename || scumm_stricmp(_filename, filename) != 0) {
		setFilename(filename);
	}

	if (_lifeTime == 0 || lifeTime == -1 || lifeTime > _lifeTime)
		_lifeTime = lifeTime;

	_keepLoaded = keepLoaded;
	if (_keepLoaded) _lifeTime = -1;

	_valid = true;

	Game->AddMem(_width * _height * 4);
	
	delete image;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBSurfaceSDL::genAlphaMask(Graphics::Surface *surface) {
	warning("CBSurfaceSDL::GenAlphaMask - Not ported yet");
	return;

	delete[] _alphaMask;
	_alphaMask = NULL;
	if (!surface) return;
#if 0
	SDL_LockSurface(surface);
#endif
	bool hasColorKey;
	uint32 colorKey;
	uint8 ckRed, ckGreen, ckBlue;
	/*  if (SDL_GetColorKey(surface, &colorKey) == 0) {
	        hasColorKey = true;
	        SDL_GetRGB(colorKey, surface->format, &ckRed, &ckGreen, &ckBlue);
	    } else hasColorKey = false;
	*/ //TODO
	_alphaMask = new byte[surface->w * surface->h];

	bool hasTransparency = false;
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			uint32 pixel = getPixel(surface, x, y);

			uint8 r, g, b, a;
			surface->format.colorToARGB(pixel, a, r, g, b);
			//SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

			if (hasColorKey && r == ckRed && g == ckGreen && b == ckBlue)
				a = 0;

			_alphaMask[y * surface->w + x] = a;
			if (a < 255) hasTransparency = true;
		}
	}
#if 0
	SDL_UnlockSurface(surface);
#endif
	if (!hasTransparency) {
		delete[] _alphaMask;
		_alphaMask = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
uint32 CBSurfaceSDL::getPixel(Graphics::Surface *surface, int x, int y) {
	warning("CBSurfaceSDL::GetPixel - Not ported yet");
	int bpp = surface->format.bytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	uint8 *p = (uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(uint16 *)p;
		break;

	case 3:
#ifdef SCUMM_BIG_ENDIAN
	//	if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
#else
		//else
			return p[0] | p[1] << 8 | p[2] << 16;
#endif
		break;

	case 4:
		return *(uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::create(int width, int height) {
	warning("SurfaceSDL::Create not ported yet"); //TODO
#if 0
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);
	_texture = SDL_CreateTexture(renderer->GetSdlRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);
#endif
	_width = width;
	_height = height;

	Game->AddMem(_width * _height * 4);

	_valid = true;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::createFromSDLSurface(Graphics::Surface *surface) {
	warning("CBSurfaceSDL::CreateFromSDLSurface not ported yet"); //TODO
#if 0
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);
	_texture = SDL_CreateTextureFromSurface(renderer->GetSdlRenderer(), surface);
#endif
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = NULL;
	}
	_surface = new Graphics::Surface();
	_surface->copyFrom(*surface);
	_width = surface->w;
	_height = surface->h;
#if 0
	Game->AddMem(_width * _height * 4);
#endif
	_valid = true;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBSurfaceSDL::isTransparentAt(int x, int y) {
	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("CBSurfaceSDL::IsTransparentAt not ported yet");
		hasWarned = true;
	}
#if 0
	int access;
	int width, height;
	//SDL_QueryTexture(_texture, NULL, &access, &width, &height); //TODO
	//if (access != SDL_TEXTUREACCESS_STREAMING) return false;
	if (X < 0 || X >= width || Y < 0 || Y >= height) return true;


	StartPixelOp();
	bool ret = IsTransparentAtLite(X, Y);
	EndPixelOp();

	return ret;
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CBSurfaceSDL::isTransparentAtLite(int x, int y) {
	//if (!_lockPixels) return false;

	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("CBSurfaceSDL::IsTransparentAtLite not ported yet");
		hasWarned = true;
	}
	if (_surface->format.bytesPerPixel == 4) {
		uint32 pixel = *(uint32*)_surface->getBasePtr(x, y);
		uint8 r,g,b,a;
		_surface->format.colorToARGB(pixel, a, r, g, b);
		if (a <= 128) {
			return true;
		} else {
			return false;
		}
	}
#if 0
	uint32 format;
	int access;
	int width, height;

	//SDL_QueryTexture(_texture, &format, &access, &width, &height);
	//if (access != SDL_TEXTUREACCESS_STREAMING) return false;
	if (X < 0 || X >= width || Y < 0 || Y >= height) return true;

	if (!_alphaMask) return false;
	else return _alphaMask[Y * width + X] <= 128;
#endif
	return false;
	/*
	Uint32* dst = (Uint32*)((Uint8*)_lockPixels + Y * _lockPitch);
	Uint32 pixel = dst[X];

	SDL_PixelFormat* pixelFormat = SDL_AllocFormat(format);
	Uint8 r, g, b, a;
	SDL_GetRGBA(pixel, pixelFormat, &r, &g, &b, &a);
	SDL_FreeFormat(pixelFormat);

	return a <= 128;
	*/
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::startPixelOp() {
	//SDL_LockTexture(_texture, NULL, &_lockPixels, &_lockPitch);
	// Any pixel-op makes the caching useless:
	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = NULL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::endPixelOp() {
	//SDL_UnlockTexture(_texture);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::display(int x, int y, RECT rect, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, 100, 100, 0xFFFFFFFF, true, blendMode, mirrorX, mirrorY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::displayTrans(int x, int y, RECT rect, uint32 alpha, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, 100, 100, alpha, false, blendMode, mirrorX, mirrorY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::displayTransOffset(int x, int y, RECT rect, uint32 alpha, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	return drawSprite(x, y, &rect, 100, 100, alpha, false, blendMode, mirrorX, mirrorY, offsetX, offsetY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::displayTransZoom(int x, int y, RECT rect, float zoomX, float zoomY, uint32 alpha, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, zoomX, zoomY, alpha, false, blendMode, mirrorX, mirrorY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::displayZoom(int x, int y, RECT rect, float zoomX, float zoomY, uint32 alpha, bool Transparent, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, zoomX, zoomY, alpha, !Transparent, blendMode, mirrorX, mirrorY);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::displayTransform(int x, int y, int HotX, int HotY, RECT Rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &Rect, zoomX, zoomY, alpha, false, blendMode, mirrorX, mirrorY);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceSDL::drawSprite(int x, int y, RECT *Rect, float ZoomX, float ZoomY, uint32 Alpha, bool AlphaDisable, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);

	if (renderer->_forceAlphaColor != 0) Alpha = renderer->_forceAlphaColor;

	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("CBSurfaceSDL::DrawSprite not fully ported yet"); // TODO.
		hasWarned = true;
	}

	byte r = D3DCOLGetR(Alpha);
	byte g = D3DCOLGetG(Alpha);
	byte b = D3DCOLGetB(Alpha);
	byte a = D3DCOLGetA(Alpha);
	
	renderer->setAlphaMod(a);
	renderer->setColorMod(r, g, b);
#if 0
	SDL_SetTextureColorMod(_texture, r, g, b);
	SDL_SetTextureAlphaMod(_texture, a);

	if (AlphaDisable)
		SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
	else
		SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
#endif
	// TODO: This _might_ miss the intended behaviour by 1 in each direction
	// But I think it fits the model used in Wintermute.
	Common::Rect srcRect;
	srcRect.left = Rect->left;
	srcRect.top = Rect->top;
	srcRect.setWidth(Rect->right - Rect->left);
	srcRect.setHeight(Rect->bottom - Rect->top);

	Common::Rect position;
	position.left = x + offsetX;
	position.top = y + offsetY;
	// TODO: Scaling...
	
	position.setWidth((float)srcRect.width() * ZoomX / 100.f);
	position.setHeight((float)srcRect.height() * ZoomX / 100.f);

	renderer->ModTargetRect(&position);

/*	position.left += offsetX;
	position.top += offsetY;*/

	// TODO: This actually requires us to have the SAME source-offsets every time,
	// But no checking is in place for that yet.
	Graphics::Surface drawSrc;
	drawSrc.w = position.width();
	drawSrc.h = position.height();
	drawSrc.format = _surface->format;

	if (position.width() != srcRect.width() || position.height() != srcRect.height()) {
		if (_scaledSurface && position.width() == _scaledSurface->w && position.height() == _scaledSurface->h) {
			drawSrc.pixels = _scaledSurface->pixels;
			drawSrc.pitch = _scaledSurface->pitch;
		} else {
			if (_scaledSurface) {
				_scaledSurface->free();
				delete _scaledSurface;
				_scaledSurface = NULL;
			}
			TransparentSurface src(*_surface, false);
			_scaledSurface = src.scale(position.width(), position.height());
			drawSrc.pixels = _scaledSurface->pixels;
			drawSrc.pitch = _scaledSurface->pitch;
		}
	} else { // No scaling
		drawSrc.pitch = _surface->pitch;
		drawSrc.pixels = &((char *)_surface->pixels)[srcRect.top * _surface->pitch + srcRect.left * 4];
	}
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.setWidth(drawSrc.w);
	srcRect.setHeight(drawSrc.h);

	if (_hasAlpha && !AlphaDisable) {
		renderer->drawFromSurface(&drawSrc, &srcRect, &position, mirrorX, mirrorY);
	} else {
		renderer->drawOpaqueFromSurface(&drawSrc, &srcRect, &position, mirrorX, mirrorY);
	}
#if 0
	SDL_RenderCopy(renderer->GetSdlRenderer(), _texture, &srcRect, &position);
#endif

	return S_OK;
}

HRESULT CBSurfaceSDL::putSurface(const Graphics::Surface &surface, bool hasAlpha) {
	_surface->copyFrom(surface);
	_hasAlpha = hasAlpha;
	return S_OK;
}

} // end of namespace WinterMute
