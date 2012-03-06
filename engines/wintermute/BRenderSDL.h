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

#ifndef WINTERMUTE_BRENDERER_SDL_H
#define WINTERMUTE_BRENDERER_SDL_H

#include "BRenderer.h"
#include "SDL.h"

namespace WinterMute {

class CBRenderSDL : public CBRenderer {
public:
	CBRenderSDL(CBGame *inGame);
	~CBRenderSDL();

	const char *GetName();

	HRESULT InitRenderer(int width, int height, bool windowed);
	HRESULT Flip();
	HRESULT Fill(byte  r, byte g, byte b, RECT *rect);

	HRESULT Fade(WORD Alpha);
	HRESULT FadeToColor(uint32 Color, RECT *rect = NULL);

	HRESULT SwitchFullscreen();

	HRESULT DrawLine(int X1, int Y1, int X2, int Y2, uint32 Color);

	CBImage *TakeScreenshot();

	SDL_Renderer *GetSdlRenderer() const {
		return m_Renderer;
	}
	SDL_Window *GetSdlWindow() const {
		return m_Win;
	}

	HRESULT SetViewport(int left, int top, int right, int bottom);

	void ModTargetRect(SDL_Rect *rect);
	void PointFromScreen(POINT *point);
	void PointToScreen(POINT *point);

	void DumpData(char *Filename);

	float GetScaleRatioX() const {
		return m_RatioX;
	}
	float GetScaleRatioY() const {
		return m_RatioY;
	}

private:
	SDL_Renderer *m_Renderer;
	SDL_Window *m_Win;
	AnsiString m_Name;

	int m_BorderLeft;
	int m_BorderTop;
	int m_BorderRight;
	int m_BorderBottom;

	float m_RatioX;
	float m_RatioY;
};

} // end of namespace WinterMute

#endif // __WmeBRenderSDL_H__
