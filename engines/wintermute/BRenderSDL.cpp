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
#include "engines/wintermute/BRenderSDL.h"
#include "engines/wintermute/BRegistry.h"
#include "engines/wintermute/BSurfaceSDL.h"
#include "engines/wintermute/BSurfaceStorage.h"
#include "engines/wintermute/BImage.h"
#include "engines/wintermute/MathUtil.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BSprite.h"
#include "common/system.h"

namespace WinterMute {

// TODO: Redo everything here.	

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::CBRenderSDL(CBGame *inGame) : CBRenderer(inGame) {
/*	_renderer = NULL;
	_win = NULL;*/
	_renderSurface = NULL;

	_borderLeft = _borderRight = _borderTop = _borderBottom = 0;
	_ratioX = _ratioY = 1.0f;
}

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::~CBRenderSDL() {
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

#if 0
	_win = SDL_CreateWindow("WME Lite",
	                         SDL_WINDOWPOS_UNDEFINED,
	                         SDL_WINDOWPOS_UNDEFINED,
	                         _realWidth, _realHeight,
	                         flags);

	if (!_win) return E_FAIL;
#endif

	warning("TODO: Hide cursor");
	//SDL_ShowCursor(SDL_DISABLE);

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
	_renderSurface = new Graphics::Surface();
	_renderSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_active = true;


	return S_OK;
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
	g_system->copyRectToScreen((byte*)_renderSurface->pixels, _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
	g_system->updateScreen();
	//SDL_RenderPresent(_renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fill(byte  r, byte g, byte b, RECT *rect) {
	//SDL_SetRenderDrawColor(_renderer, r, g, b, 0xFF);
	//SDL_RenderClear(_renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fade(WORD Alpha) {
	uint32 dwAlpha = 255 - Alpha;
	return FadeToColor(dwAlpha << 24);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::FadeToColor(uint32 Color, RECT *rect) {
	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.
	static bool hasWarned = false;
	if (!hasWarned) {
		warning("Implement CBRenderSDL::FadeToColor"); // TODO.
		hasWarned = true;
	}
#if 0
	SDL_Rect fillRect;

	if (rect) {
		fillRect.x = rect->left;
		fillRect.y = rect->top;
		fillRect.w = rect->right - rect->left;
		fillRect.h = rect->bottom - rect->top;
	} else {
		RECT rc;
		Game->GetCurrentViewportRect(&rc);
		fillRect.x = rc.left;
		fillRect.y = rc.top;
		fillRect.w = rc.right - rc.left;
		fillRect.h = rc.bottom - rc.top;
	}
	ModTargetRect(&fillRect);

	byte r = D3DCOLGetR(Color);
	byte g = D3DCOLGetG(Color);
	byte b = D3DCOLGetB(Color);
	byte a = D3DCOLGetA(Color);

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	//SDL_RenderFillRect(_renderer, &fillRect);
#endif
	return S_OK;
}

// Replacement for SDL2's SDL_RenderCopy	
void CBRenderSDL::drawFromSurface(Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect) {
	for (int i = 0; i < srcRect->height(); i++) {
		void *destPtr = _renderSurface->getBasePtr(dstRect->left, dstRect->top + i);
		void *srcPtr = surf->getBasePtr(srcRect->left, srcRect->top + i);
		for (int j = 0; j < srcRect->width(); j++) {
			// TODO: Replace this with something less ugly, and more portable.
			if (((byte*)srcPtr)[0] == 255) {
				memcpy(destPtr, srcPtr, _renderSurface->format.bytesPerPixel);	
			}
			((byte*)srcPtr)+=_renderSurface->format.bytesPerPixel;
			((byte*)destPtr)+=_renderSurface->format.bytesPerPixel;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::DrawLine(int X1, int Y1, int X2, int Y2, uint32 Color) {
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


	//SDL_RenderDrawLine(_renderer, point1.x, point1.y, point2.x, point2.y);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBImage *CBRenderSDL::TakeScreenshot() {
// TODO: Fix this
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
		byte *src = (byte  *)surface->pixels + (viewport.h - y - 1) * surface->pitch;
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
		fprintf(f, "%dx%d;", Surf->GetWidth(), Surf->GetHeight());

		int kb = Surf->GetWidth() * Surf->GetHeight() * 4 / 1024;

		TotalKB += kb;
		fprintf(f, "%d;", kb);
		fprintf(f, "\n");
	}
	fprintf(f, "Total %d;;;%d\n", Mgr->_surfaces.GetSize(), TotalKB);


	fclose(f);
	Game->LOG(0, "Texture Stats Dump completed.");
	Game->QuickMessage("Texture Stats Dump completed.");
}

} // end of namespace WinterMute
