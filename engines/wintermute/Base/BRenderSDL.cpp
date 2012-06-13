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
#include "engines/wintermute/Base/BRenderSDL.h"
#include "engines/wintermute/Base/BRegistry.h"
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/math/MathUtil.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSprite.h"
#include "common/system.h"
#include "engines/wintermute/graphics/transparentSurface.h"

namespace WinterMute {

// TODO: Redo everything here.

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::CBRenderSDL(CBGame *inGame) : CBRenderer(inGame) {
	/*  _renderer = NULL;
	    _win = NULL;*/
	_renderSurface = new Graphics::Surface();

	_borderLeft = _borderRight = _borderTop = _borderBottom = 0;
	_ratioX = _ratioY = 1.0f;
	setAlphaMod(255);
	setColorMod(255, 255, 255);
}

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::~CBRenderSDL() {
	_renderSurface->free();
	delete _renderSurface;
#if 0
	if (_renderer) SDL_DestroyRenderer(_renderer);
	if (_win) SDL_DestroyWindow(_win);
	SDL_Quit();
#endif
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::InitRenderer(int width, int height, bool windowed) {
	//if (SDL_Init(SDL_INIT_VIDEO) < 0) return E_FAIL;

#if 0
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
#endif

	_width = width;
	_height = height;

	_realWidth = width;
	_realHeight = height;


	// find suitable resolution
#ifdef __IPHONEOS__
	_realWidth = 480;
	_realHeight = 320;

	int numModes = SDL_GetNumDisplayModes(0);
	for (int i = 0; i < numModes; i++) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, i, &mode);

		if (mode.w > mode.h) {
			_realWidth = mode.w;
			_realHeight = mode.h;
			break;
		}
	}
#else
	_realWidth = Game->_registry->ReadInt("Debug", "ForceResWidth", _width);
	_realHeight = Game->_registry->ReadInt("Debug", "ForceResHeight", _height);
#endif

	/*
	_realWidth = 480;
	_realHeight = 320;
	*/


	float origAspect = (float)_width / (float)_height;
	float realAspect = (float)_realWidth / (float)_realHeight;

	float ratio;
	if (origAspect < realAspect) {
		// normal to wide
		ratio = (float)_realHeight / (float)_height;
	} else {
		// wide to normal
		ratio = (float)_realWidth / (float)_width;
	}

	_borderLeft = (_realWidth - (_width * ratio)) / 2;
	_borderRight = _realWidth - (_width * ratio) - _borderLeft;

	_borderTop = (_realHeight - (_height * ratio)) / 2;
	_borderBottom = _realHeight - (_height * ratio) - _borderTop;



	_ratioX = (float)(_realWidth - _borderLeft - _borderRight) / (float)_width;
	_ratioY = (float)(_realHeight - _borderTop - _borderBottom) / (float)_height;

#if 0
	Uint32 flags = SDL_WINDOW_SHOWN;
#endif
#ifdef __IPHONEOS__
	flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
#endif

	//_windowed = Game->_registry->ReadBool("Video", "Windowed", true);
//	if (!windowed) flags |= SDL_WINDOW_FULLSCREEN;

	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	g_system->beginGFXTransaction();
	g_system->initSize(_width, _height, &format);
	OSystem::TransactionError gfxError = g_system->endGFXTransaction();
	
	if (gfxError != OSystem::kTransactionSuccess) {
		warning("Couldn't setup GFX-backend for %dx%dx%d", _width, _height, format.bytesPerPixel * 8);
		return E_FAIL;
	}
#if 0
	_win = SDL_CreateWindow("WME Lite",
	                        SDL_WINDOWPOS_UNDEFINED,
	                        SDL_WINDOWPOS_UNDEFINED,
	                        _realWidth, _realHeight,
	                        flags);

	if (!_win) return E_FAIL;
#endif

	g_system->showMouse(false);

#ifdef __IPHONEOS__
	// SDL defaults to OGL ES2, which doesn't work on old devices
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles");
#else
	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
#if 0
	_renderer = SDL_CreateRenderer(_win, -1, 0);

	if (!_renderer) return E_FAIL;
#endif
	_renderSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_active = true;


	return S_OK;
}

void CBRenderSDL::setAlphaMod(byte alpha) {
	byte r = D3DCOLGetR(_colorMod);
	byte g = D3DCOLGetB(_colorMod);
	byte b = D3DCOLGetB(_colorMod);
	_colorMod = BS_ARGB(alpha, r, g, b);
}

void CBRenderSDL::setColorMod(byte r, byte g, byte b) {
	byte alpha = D3DCOLGetA(_colorMod);
	_colorMod = BS_ARGB(alpha, r, g, b);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Flip() {

#ifdef __IPHONEOS__
	// hack: until viewports work correctly, we just paint black bars instead
	SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0xFF);

	static bool firstRefresh = true; // prevents a weird color glitch
	if (firstRefresh) {
		firstRefresh = false;
	} else {
		SDL_Rect rect;
		if (_borderLeft > 0) {
			rect.x = 0;
			rect.y = 0;
			rect.w = _borderLeft;
			rect.h = _realHeight;
			SDL_RenderFillRect(_renderer, &rect);
		}
		if (_borderRight > 0) {
			rect.x = (_realWidth - _borderRight);
			rect.y = 0;
			rect.w = _borderRight;
			rect.h = _realHeight;
			SDL_RenderFillRect(_renderer, &rect);
		}
		if (_borderTop > 0) {
			rect.x = 0;
			rect.y = 0;
			rect.w = _realWidth;
			rect.h = _borderTop;
			SDL_RenderFillRect(_renderer, &rect);
		}
		if (_borderBottom > 0) {
			rect.x = 0;
			rect.y = _realHeight - _borderBottom;
			rect.w = _realWidth;
			rect.h = _borderBottom;
			SDL_RenderFillRect(_renderer, &rect);
		}
	}
#endif
	g_system->copyRectToScreen((byte *)_renderSurface->pixels, _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
	g_system->updateScreen();
	//SDL_RenderPresent(_renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fill(byte r, byte g, byte b, Common::Rect *rect) {
	//SDL_SetRenderDrawColor(_renderer, r, g, b, 0xFF);
	//SDL_RenderClear(_renderer);
	uint32 color = _renderSurface->format.ARGBToColor(0xFF, r, g, b);
	if (!rect) {
		rect = new Common::Rect();
		rect->setWidth(_renderSurface->w);
		rect->setHeight(_renderSurface->h);
		_renderSurface->fillRect(*rect, color);
		delete rect;
	} else {
		_renderSurface->fillRect(*rect, color);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fade(uint16 Alpha) {
	uint32 dwAlpha = 255 - Alpha;
	return FadeToColor(dwAlpha << 24);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::FadeToColor(uint32 Color, Common::Rect *rect) {
	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("Implement CBRenderSDL::FadeToColor"); // TODO.
		hasWarned = true;
	}
	
	Common::Rect fillRect;

	if (rect) {
		fillRect.left = rect->left;
		fillRect.top = rect->top;
		fillRect.setWidth(rect->width());
		fillRect.setHeight(rect->height());
	} else {
		RECT rc;
		Game->GetCurrentViewportRect(&rc);
		fillRect.left = rc.left;
		fillRect.top = rc.top;
		fillRect.setWidth(rc.right - rc.left);
		fillRect.setHeight(rc.bottom - rc.top);
	}
	ModTargetRect(&fillRect);

	byte r = D3DCOLGetR(Color);
	byte g = D3DCOLGetG(Color);
	byte b = D3DCOLGetB(Color);
	byte a = D3DCOLGetA(Color);

	//TODO: This is only here until I'm sure about the final pixelformat
	uint32 col = _renderSurface->format.ARGBToColor(a, r, g, b);
	_renderSurface->fillRect(fillRect, col);

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	//SDL_RenderFillRect(_renderer, &fillRect);

	return S_OK;
}

// Replacement for SDL2's SDL_RenderCopy
void CBRenderSDL::drawFromSurface(Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY) {
	TransparentSurface src(*surf, false);
	int mirror = TransparentSurface::FLIP_NONE;
	if (mirrorX)
		mirror |= TransparentSurface::FLIP_V;
	if (mirrorY)
		mirror |= TransparentSurface::FLIP_H;
	src.blit(*_renderSurface, dstRect->left, dstRect->top, mirror, srcRect, _colorMod, dstRect->width(), dstRect->height() );
}

void CBRenderSDL::drawOpaqueFromSurface(Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY) {
	TransparentSurface src(*surf, false);
	TransparentSurface *img = NULL;
	TransparentSurface *imgScaled = NULL;
	byte *savedPixels = NULL;
	if ((dstRect->width() != surf->w) || (dstRect->height() != surf->h)) {
		img = imgScaled = src.scale(dstRect->width(), dstRect->height());
		savedPixels = (byte *)img->pixels;
	} else {
		img = &src;
	}

	int posX = dstRect->left;	
	int posY = dstRect->top;
	
	// Handle off-screen clipping
	if (posY < 0) {
		img->h = MAX(0, (int)img->h - -posY);
		img->pixels = (byte *)img->pixels + img->pitch * -posY;
		posY = 0;
	}
	
	if (posX < 0) {
		img->w = MAX(0, (int)img->w - -posX);
		img->pixels = (byte *)img->pixels + (-posX * 4);
		posX = 0;
	}
	
	img->w = CLIP((int)img->w, 0, (int)MAX((int)_renderSurface->w - posX, 0));
	img->h = CLIP((int)img->h, 0, (int)MAX((int)_renderSurface->h - posY, 0));
	
	for (int i = 0; i < img->h; i++) {
		void *destPtr = _renderSurface->getBasePtr(posX, posY + i);
		void *srcPtr = img->getBasePtr(0, i);
		memcpy(destPtr, srcPtr, _renderSurface->format.bytesPerPixel * img->w);
	}
	
	if (imgScaled) {
		imgScaled->pixels = savedPixels;
		imgScaled->free();
		delete imgScaled;
		imgScaled = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::DrawLine(int X1, int Y1, int X2, int Y2, uint32 Color) {
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("CBRenderSDL::DrawLine - not fully ported yet");
		hasWarned = true;
	}
	byte r = D3DCOLGetR(Color);
	byte g = D3DCOLGetG(Color);
	byte b = D3DCOLGetB(Color);
	byte a = D3DCOLGetA(Color);

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

	POINT point1, point2;
	point1.x = X1;
	point1.y = Y1;
	PointToScreen(&point1);

	point2.x = X2;
	point2.y = Y2;
	PointToScreen(&point2);

	// TODO: This thing is mostly here until I'm sure about the final color-format.
	uint32 color = _renderSurface->format.ARGBToColor(a, r, g, b);
	_renderSurface->drawLine(point1.x, point1.y, point2.x, point2.y, color);
	//SDL_RenderDrawLine(_renderer, point1.x, point1.y, point2.x, point2.y);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBImage *CBRenderSDL::TakeScreenshot() {
// TODO: Fix this
	warning("CBRenderSDL::TakeScreenshot() - not ported yet");
#if 0
	SDL_Rect viewport;

	SDL_RenderGetViewport(_renderer, &viewport);

	SDL_Surface *surface = SDL_CreateRGBSurface(0, viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0x00000000);
	if (!surface) return NULL;

	if (SDL_RenderReadPixels(_renderer, NULL, surface->format->format, surface->pixels, surface->pitch) < 0) return NULL;

	FIBITMAP *dib = FreeImage_Allocate(viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);

	for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
		byte *bits = FreeImage_GetScanLine(dib, y);
		byte *src = (byte *)surface->pixels + (viewport.h - y - 1) * surface->pitch;
		memcpy(bits, src, bytespp * viewport.w);
	}

	return new CBImage(Game, dib);
#endif
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::SwitchFullscreen() {
	/*if (_windowed) SDL_SetWindowFullscreen(_win, SDL_TRUE);
	else SDL_SetWindowFullscreen(_win, SDL_FALSE);

	_windowed = !_windowed;
	*/
	Game->_registry->WriteBool("Video", "Windowed", _windowed);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
const char *CBRenderSDL::GetName() {
	if (_name.empty()) {
#if 0
		if (_renderer) {
			SDL_RendererInfo info;
			SDL_GetRendererInfo(_renderer, &info);
			_name = AnsiString(info.name);
		}
#endif
	}
	return _name.c_str();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::SetViewport(int left, int top, int right, int bottom) {
	Common::Rect rect;
	// TODO: Hopefully this is the same logic that ScummVM uses.
	rect.left = left + _borderLeft;
	rect.top = top + _borderTop;
	rect.right = (right - left) * _ratioX;
	rect.bottom = (bottom - top) * _ratioY;

	// TODO fix this once viewports work correctly in SDL/landscape
#ifndef __IPHONEOS__
	//SDL_RenderSetViewport(GetSdlRenderer(), &rect);
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::ModTargetRect(Common::Rect *rect) {
#if 0
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	rect->x = MathUtil::Round(rect->x * _ratioX + _borderLeft - viewportRect.x);
	rect->y = MathUtil::Round(rect->y * _ratioY + _borderTop - viewportRect.y);
	rect->w = MathUtil::RoundUp(rect->w * _ratioX);
	rect->h = MathUtil::RoundUp(rect->h * _ratioY);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::PointFromScreen(POINT *point) {
#if 0
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	point->x = point->x / _ratioX - _borderLeft / _ratioX + viewportRect.x;
	point->y = point->y / _ratioY - _borderTop / _ratioY + viewportRect.y;
#endif
}


//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::PointToScreen(POINT *point) {
#if 0
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	point->x = MathUtil::RoundUp(point->x * _ratioX) + _borderLeft - viewportRect.x;
	point->y = MathUtil::RoundUp(point->y * _ratioY) + _borderTop - viewportRect.y;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::DumpData(const char *Filename) {
	warning("CBRenderSDL::DumpData(%s) - not reimplemented yet", Filename); // TODO
#if 0
	FILE *f = fopen(Filename, "wt");
	if (!f) return;

	CBSurfaceStorage *Mgr = Game->_surfaceStorage;

	int TotalKB = 0;
	int TotalLoss = 0;
	fprintf(f, "Filename;Usage;Size;KBytes\n");
	for (int i = 0; i < Mgr->_surfaces.GetSize(); i++) {
		CBSurfaceSDL *Surf = (CBSurfaceSDL *)Mgr->_surfaces[i];
		if (!Surf->_filename) continue;
		if (!Surf->_valid) continue;

		fprintf(f, "%s;%d;", Surf->_filename, Surf->_referenceCount);
		fprintf(f, "%dx%d;", Surf->getWidth(), Surf->getHeight());

		int kb = Surf->getWidth() * Surf->getHeight() * 4 / 1024;

		TotalKB += kb;
		fprintf(f, "%d;", kb);
		fprintf(f, "\n");
	}
	fprintf(f, "Total %d;;;%d\n", Mgr->_surfaces.GetSize(), TotalKB);


	fclose(f);
	Game->LOG(0, "Texture Stats Dump completed.");
	Game->QuickMessage("Texture Stats Dump completed.");
#endif
}

} // end of namespace WinterMute
