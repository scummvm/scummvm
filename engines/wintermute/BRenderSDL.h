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
#include "common/rect.h"
#include "graphics/surface.h"

class SDL_Window;
class SDL_Renderer;

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
	
	void drawFromSurface(Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRest);


	HRESULT SetViewport(int left, int top, int right, int bottom);

	void ModTargetRect(Common::Rect *rect);
	void PointFromScreen(POINT *point);
	void PointToScreen(POINT *point);

	void DumpData(char *Filename);

	float GetScaleRatioX() const {
		return _ratioX;
	}
	float GetScaleRatioY() const {
		return _ratioY;
	}

private:
/*	SDL_Renderer *_renderer;
	SDL_Window *_win;*/
	Graphics::Surface *_renderSurface;
	AnsiString _name;

	int _borderLeft;
	int _borderTop;
	int _borderRight;
	int _borderBottom;

	float _ratioX;
	float _ratioY;
};

} // end of namespace WinterMute

#endif // __WmeBRenderSDL_H__
