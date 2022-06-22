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

#ifndef HPL_BITMAP2D_H
#define HPL_BITMAP2D_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/LowLevelPicture.h"
#include "hpl1/engine/math/MathTypes.h"
#include "graphics/pixelformat.h"
#include "common/ptr.h"
#include "graphics/surface.h"

namespace hpl {

class Bitmap2D : public iLowLevelPicture {
public:
	Bitmap2D(const tString &type) : iLowLevelPicture(type) {}
	Bitmap2D(const Graphics::Surface &surface, const tString &type) : 
		iLowLevelPicture(type), _surface(surface) {
	} 
	
	~Bitmap2D(); 


	void drawToBitmap(Bitmap2D &dest, const cVector2l &pos);

	bool create(const cVector2l &size, const Graphics::PixelFormat &format);

	void fillRect(const cRect2l &rect, const cColor &color);

	void *getRawData();

	int getNumChannels();

	bool HasAlpha() override;

private: 
	Graphics::Surface _surface; 
};

typedef std::vector<Bitmap2D *> tBitmap2DVec;
typedef std::vector<Bitmap2D *>::iterator tBitmap2DVecIt;

};     // namespace hpl
#endif // HPL_BITMAP2D_H
