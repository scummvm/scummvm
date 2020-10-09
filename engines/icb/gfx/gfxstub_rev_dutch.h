/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_GFXSTUB_REV_DUTCH_HH
#define ICB_GFXSTUB_REV_DUTCH_HH

#include "engines/icb/gfx/gfxstub_dutch.h"

#if defined (SDL_BACKEND) && defined (ENABLE_OPENGL)
#include <SDL_opengl.h>
#endif

namespace ICB {

class TextureHandle {
public:
	u_char *pRGBA[9]; // width/1 * height/1 -> width/256 * height/256
	u_int *palette;
	int w;
	int h;
	int bpp;
#if defined (SDL_BACKEND) && defined (ENABLE_OPENGL)
	GLuint textureId;
#endif
};

RevTexture *MakeRevTexture(u_int w, u_int h, u_int *palette, u_char *img);
void Make24palette(u_int *outPal, u_short *inPal);

} // End of namespace ICB

#endif // #ifndef GFXSTUB_REV_DUTCH_HH
