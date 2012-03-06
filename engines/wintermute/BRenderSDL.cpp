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
#include "BRenderSDL.h"
#include "BRegistry.h"
#include "BSurfaceSDL.h"
#include "BImage.h"
#include "MathUtil.h"
#include "BGame.h"
#include "BSprite.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::CBRenderSDL(CBGame *inGame) : CBRenderer(inGame) {
	m_Renderer = NULL;
	m_Win = NULL;

	m_BorderLeft = m_BorderRight = m_BorderTop = m_BorderBottom = 0;
	m_RatioX = m_RatioY = 1.0f;
}

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::~CBRenderSDL() {
	if (m_Renderer) SDL_DestroyRenderer(m_Renderer);
	if (m_Win) SDL_DestroyWindow(m_Win);

	SDL_Quit();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::InitRenderer(int width, int height, bool windowed) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) return E_FAIL;


	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);


	m_Width = width;
	m_Height = height;

	m_RealWidth = width;
	m_RealHeight = height;


	// find suitable resolution
#ifdef __IPHONEOS__
	m_RealWidth = 480;
	m_RealHeight = 320;

	int numModes = SDL_GetNumDisplayModes(0);
	for (int i = 0; i < numModes; i++) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, i, &mode);

		if (mode.w > mode.h) {
			m_RealWidth = mode.w;
			m_RealHeight = mode.h;
			break;
		}
	}
#else
	m_RealWidth = Game->m_Registry->ReadInt("Debug", "ForceResWidth", m_Width);
	m_RealHeight = Game->m_Registry->ReadInt("Debug", "ForceResHeight", m_Height);
#endif

	/*
	m_RealWidth = 480;
	m_RealHeight = 320;
	*/


	float origAspect = (float)m_Width / (float)m_Height;
	float realAspect = (float)m_RealWidth / (float)m_RealHeight;

	float ratio;
	if (origAspect < realAspect) {
		// normal to wide
		ratio = (float)m_RealHeight / (float)m_Height;
	} else {
		// wide to normal
		ratio = (float)m_RealWidth / (float)m_Width;
	}

	m_BorderLeft = (m_RealWidth - (m_Width * ratio)) / 2;
	m_BorderRight = m_RealWidth - (m_Width * ratio) - m_BorderLeft;

	m_BorderTop = (m_RealHeight - (m_Height * ratio)) / 2;
	m_BorderBottom = m_RealHeight - (m_Height * ratio) - m_BorderTop;



	m_RatioX = (float)(m_RealWidth - m_BorderLeft - m_BorderRight) / (float)m_Width;
	m_RatioY = (float)(m_RealHeight - m_BorderTop - m_BorderBottom) / (float)m_Height;


	Uint32 flags = SDL_WINDOW_SHOWN;
#ifdef __IPHONEOS__
	flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
#endif

	//m_Windowed = Game->m_Registry->ReadBool("Video", "Windowed", true);
	if (!windowed) flags |= SDL_WINDOW_FULLSCREEN;


	m_Win = SDL_CreateWindow("WME Lite",
	                         SDL_WINDOWPOS_UNDEFINED,
	                         SDL_WINDOWPOS_UNDEFINED,
	                         m_RealWidth, m_RealHeight,
	                         flags);

	if (!m_Win) return E_FAIL;

	SDL_ShowCursor(SDL_DISABLE);

#ifdef __IPHONEOS__
	// SDL defaults to OGL ES2, which doesn't work on old devices
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles");
#else
	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif

	m_Renderer = SDL_CreateRenderer(m_Win, -1, 0);
	if (!m_Renderer) return E_FAIL;

	m_Active = true;


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Flip() {

#ifdef __IPHONEOS__
	// hack: until viewports work correctly, we just paint black bars instead
	SDL_SetRenderDrawColor(m_Renderer, 0x00, 0x00, 0x00, 0xFF);

	static bool firstRefresh = true; // prevents a weird color glitch
	if (firstRefresh) {
		firstRefresh = false;
	} else {
		SDL_Rect rect;
		if (m_BorderLeft > 0) {
			rect.x = 0;
			rect.y = 0;
			rect.w = m_BorderLeft;
			rect.h = m_RealHeight;
			SDL_RenderFillRect(m_Renderer, &rect);
		}
		if (m_BorderRight > 0) {
			rect.x = (m_RealWidth - m_BorderRight);
			rect.y = 0;
			rect.w = m_BorderRight;
			rect.h = m_RealHeight;
			SDL_RenderFillRect(m_Renderer, &rect);
		}
		if (m_BorderTop > 0) {
			rect.x = 0;
			rect.y = 0;
			rect.w = m_RealWidth;
			rect.h = m_BorderTop;
			SDL_RenderFillRect(m_Renderer, &rect);
		}
		if (m_BorderBottom > 0) {
			rect.x = 0;
			rect.y = m_RealHeight - m_BorderBottom;
			rect.w = m_RealWidth;
			rect.h = m_BorderBottom;
			SDL_RenderFillRect(m_Renderer, &rect);
		}
	}
#endif


	SDL_RenderPresent(m_Renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fill(byte  r, byte g, byte b, RECT *rect) {
	SDL_SetRenderDrawColor(m_Renderer, r, g, b, 0xFF);
	SDL_RenderClear(m_Renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fade(WORD Alpha) {
	uint32 dwAlpha = 255 - Alpha;
	return FadeToColor(dwAlpha << 24);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::FadeToColor(uint32 Color, RECT *rect) {
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

	SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(m_Renderer, &fillRect);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::DrawLine(int X1, int Y1, int X2, int Y2, uint32 Color) {
	byte r = D3DCOLGetR(Color);
	byte g = D3DCOLGetG(Color);
	byte b = D3DCOLGetB(Color);
	byte a = D3DCOLGetA(Color);

	SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);

	POINT point1, point2;
	point1.x = X1;
	point1.y = Y1;
	PointToScreen(&point1);

	point2.x = X2;
	point2.y = Y2;
	PointToScreen(&point2);


	SDL_RenderDrawLine(m_Renderer, point1.x, point1.y, point2.x, point2.y);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBImage *CBRenderSDL::TakeScreenshot() {
// TODO: Fix this
#if 0
	SDL_Rect viewport;

	SDL_RenderGetViewport(m_Renderer, &viewport);

	SDL_Surface *surface = SDL_CreateRGBSurface(0, viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0x00000000);
	if (!surface) return NULL;

	if (SDL_RenderReadPixels(m_Renderer, NULL, surface->format->format, surface->pixels, surface->pitch) < 0) return NULL;

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
	if (m_Windowed) SDL_SetWindowFullscreen(m_Win, SDL_TRUE);
	else SDL_SetWindowFullscreen(m_Win, SDL_FALSE);

	m_Windowed = !m_Windowed;

	Game->m_Registry->WriteBool("Video", "Windowed", m_Windowed);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
const char *CBRenderSDL::GetName() {
	if (m_Name.empty()) {
		if (m_Renderer) {
			SDL_RendererInfo info;
			SDL_GetRendererInfo(m_Renderer, &info);
			m_Name = AnsiString(info.name);
		}
	}
	return m_Name.c_str();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::SetViewport(int left, int top, int right, int bottom) {
	SDL_Rect rect;
	rect.x = left + m_BorderLeft;
	rect.y = top + m_BorderTop;
	rect.w = (right - left) * m_RatioX;
	rect.h = (bottom - top) * m_RatioY;

	// TODO fix this once viewports work correctly in SDL/landscape
#ifndef __IPHONEOS__
	SDL_RenderSetViewport(GetSdlRenderer(), &rect);
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::ModTargetRect(SDL_Rect *rect) {
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	rect->x = MathUtil::Round(rect->x * m_RatioX + m_BorderLeft - viewportRect.x);
	rect->y = MathUtil::Round(rect->y * m_RatioY + m_BorderTop - viewportRect.y);
	rect->w = MathUtil::RoundUp(rect->w * m_RatioX);
	rect->h = MathUtil::RoundUp(rect->h * m_RatioY);
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::PointFromScreen(POINT *point) {
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	point->x = point->x / m_RatioX - m_BorderLeft / m_RatioX + viewportRect.x;
	point->y = point->y / m_RatioY - m_BorderTop / m_RatioY + viewportRect.y;
}


//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::PointToScreen(POINT *point) {
	SDL_Rect viewportRect;
	SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

	point->x = MathUtil::RoundUp(point->x * m_RatioX) + m_BorderLeft - viewportRect.x;
	point->y = MathUtil::RoundUp(point->y * m_RatioY) + m_BorderTop - viewportRect.y;

}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::DumpData(char *Filename) {
	FILE *f = fopen(Filename, "wt");
	if (!f) return;

	CBSurfaceStorage *Mgr = Game->m_SurfaceStorage;

	int TotalKB = 0;
	int TotalLoss = 0;
	fprintf(f, "Filename;Usage;Size;KBytes\n");
	for (int i = 0; i < Mgr->m_Surfaces.GetSize(); i++) {
		CBSurfaceSDL *Surf = (CBSurfaceSDL *)Mgr->m_Surfaces[i];
		if (!Surf->m_Filename) continue;
		if (!Surf->m_Valid) continue;

		fprintf(f, "%s;%d;", Surf->m_Filename, Surf->m_ReferenceCount);
		fprintf(f, "%dx%d;", Surf->GetWidth(), Surf->GetHeight());

		int kb = Surf->GetWidth() * Surf->GetHeight() * 4 / 1024;

		TotalKB += kb;
		fprintf(f, "%d;", kb);
		fprintf(f, "\n");
	}
	fprintf(f, "Total %d;;;%d\n", Mgr->m_Surfaces.GetSize(), TotalKB);


	fclose(f);
	Game->LOG(0, "Texture Stats Dump completed.");
	Game->QuickMessage("Texture Stats Dump completed.");
}

} // end of namespace WinterMute
