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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HPL_SDL_PIXELFORMAT_H
#define HPL_SDL_PIXELFORMAT_H

#include "hpl1/engine/graphics/PixelFormat.h"
//#include "SDL/SDL.h"
#include "temp-types.h"

namespace hpl {

	class cSDLPixelFormat : public iPixelFormat
	{
	public:
		cSDLPixelFormat(): iPixelFormat("SDL")
		{
#if 0
  	////// BEGIN - PIXEL FORMAT 32 bit RGBA ////////
			mSDLPixelFmt32.palette = 0;
			mSDLPixelFmt32.BitsPerPixel = 32;
			mSDLPixelFmt32.BytesPerPixel = 4;

			if(SDL_BYTEORDER == SDL_BIG_ENDIAN){
				mSDLPixelFmt32.Rmask = 0xff000000;
				mSDLPixelFmt32.Gmask = 0x00ff0000;
				mSDLPixelFmt32.Bmask = 0x0000ff00;
				mSDLPixelFmt32.Amask = 0x000000ff;

				mSDLPixelFmt32.Rshift = 24;
				mSDLPixelFmt32.Gshift = 16;
				mSDLPixelFmt32.Bshift = 8;
				mSDLPixelFmt32.Ashift = 0;

			}
			else
			{
				mSDLPixelFmt32.Rmask = 0x000000ff;
				mSDLPixelFmt32.Gmask = 0x0000ff00;
				mSDLPixelFmt32.Bmask = 0x00ff0000;
				mSDLPixelFmt32.Amask = 0xff000000;

				mSDLPixelFmt32.Rshift = 0;
				mSDLPixelFmt32.Gshift = 8;
				mSDLPixelFmt32.Bshift = 16;
				mSDLPixelFmt32.Ashift = 24;
			}

			mSDLPixelFmt32.Rloss=0;
			mSDLPixelFmt32.Gloss=0;
			mSDLPixelFmt32.Bloss=0;
			mSDLPixelFmt32.Aloss=0;

			mSDLPixelFmt32.colorkey =0;
			mSDLPixelFmt32.alpha = 255;
			////// END - PIXEL FORMAT 32 bit RGBA ////////

#endif

		}

		SDL_PixelFormat* GetSDLPixelFormat32()
		{
			return &mSDLPixelFmt32;
		}

	private:
		SDL_PixelFormat mSDLPixelFmt32;


	};

};
#endif // HPL_SDL_PIXELFORMAT_H
