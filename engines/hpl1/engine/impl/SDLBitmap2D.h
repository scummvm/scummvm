/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_SDL_BITMAP2D_H
#define HPL_SDL_BITMAP2D_H

#include "hpl1/engine/graphics/Bitmap2D.h"
#include "graphics/pixelformat.h"

//#include "SDL/SDL.h"
#include "temp-types.h"

namespace hpl {

class cSDLBitmap2D : public iBitmap2D {
public:
	cSDLBitmap2D(Graphics::PixelFormat *apPxlFmt);
	cSDLBitmap2D(SDL_Surface *apSurface, Graphics::PixelFormat *apPxlFmt, const tString &asType);
	~cSDLBitmap2D();

	bool SaveToFile(const tString &asFile);

	void DrawToBitmap(iBitmap2D *apBmp, const cVector2l &avDestPos);
	bool HasAlpha();
	/**
	 * \todo Support 16 bit format aswell.
	 * \param avSize
	 * \param alBpp
	 * \return
	 */
	bool Create(cVector2l avSize, unsigned int alBpp);

	void FillRect(const cRect2l &aRect, const cColor &aColor);

	void *GetRawData();
	int GetNumChannels();

	// SDL Specific
	SDL_Surface *GetSurface();
	SDL_PixelFormat *GetPixelFmt32() { return mpSDLPixelFmt32; }

	tString msType;

private:
	SDL_Surface *mpSurface;
	SDL_PixelFormat *mpSDLPixelFmt32;
};
};     // namespace hpl
#endif // HPL_SDL_BITMAP2D_H
