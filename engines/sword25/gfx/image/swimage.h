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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_SWIMAGE_H
#define SWORD25_SWIMAGE_H

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/graphicengine.h"


namespace Sword25 {

class SWImage : public Image {
public:
	SWImage(const Common::String &filename, bool &result);
	virtual ~SWImage();

	virtual int getWidth() const override {
		return _width;
	}
	virtual int getHeight() const override {
		return _height;
	}
	virtual GraphicEngine::COLOR_FORMATS getColorFormat() const override {
		return GraphicEngine::CF_ARGB32;
	}

	virtual bool blit(int posX = 0, int posY = 0,
	                  int flipping = Graphics::FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1,
					  RectangleList *updateRects = 0) override;
	virtual bool fill(const Common::Rect *fillRectPtr, uint color) override;
	virtual bool setContent(const byte *pixeldata, uint size, uint offset, uint stride) override;
	virtual uint getPixel(int x, int y) override;

	virtual bool isBlitSource() const override               {
		return false;
	}
	virtual bool isBlitTarget() const override               {
		return false;
	}
	virtual bool isScalingAllowed() const override           {
		return false;
	}
	virtual bool isFillingAllowed() const override           {
		return false;
	}
	virtual bool isAlphaAllowed() const override             {
		return false;
	}
	virtual bool isColorModulationAllowed() const override   {
		return false;
	}
	virtual bool isSetContentAllowed() const override        {
		return false;
	}
private:
	uint *_imageDataPtr;

	int _width;
	int _height;
};

} // End of namespace Sword25

#endif
