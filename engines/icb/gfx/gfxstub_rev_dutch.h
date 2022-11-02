/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_GFXSTUB_REV_DUTCH_HH
#define ICB_GFXSTUB_REV_DUTCH_HH

#include "engines/icb/gfx/gfxstub_dutch.h"

namespace ICB {

class TextureHandle {
public:
	uint8 *pRGBA[9]; // width/1 * height/1 -> width/256 * height/256
	uint32 *palette;
	int32 w;
	int32 h;
	int32 bpp;
};

RevTexture *MakeRevTexture(uint32 w, uint32 h, uint32 *palette, uint8 *img);
void Make24palette(uint32 *outPal, uint16 *inPal);

} // End of namespace ICB

#endif // #ifndef GFXSTUB_REV_DUTCH_HH
