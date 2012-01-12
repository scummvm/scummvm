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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MORTEVIELLE_GRAPHICS_H
#define MORTEVIELLE_GRAPHICS_H

#include "graphics/surface.h"
#include "mortevielle/mortevielle.h"

namespace Mortevielle {

class GfxSurface: public Graphics::Surface {
private:
	int _var1;
	int _entryCount;
	int _var4, _var6;
	int _xp, _yp;
	int _varC, _xSize, _ySize, _var12;
	int _var14, _var15, _var18, _var1A;
	int _var1C, _var1E, _var20, _var22;
	int _var24, _var26, _var28;
	int _width, _height;
	int _xOffset, _yOffset;

	void majTtxTty();
	byte suiv(const byte *&pSrc);
	int desanalyse(const byte *&pSrc);
	void horizontal(const byte *&pSrc, byte *&pDest);
public:
	void decode(const byte *pSrc);
};

} // End of namespace Mortevielle

#endif
