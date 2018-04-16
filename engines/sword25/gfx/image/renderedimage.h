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

#ifndef SWORD25_RENDERED_IMAGE_H
#define SWORD25_RENDERED_IMAGE_H

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/graphicengine.h"
#include "graphics/transparent_surface.h"

namespace Sword25 {

class RenderedImage : public Image {
private:
	RenderedImage(const RenderedImage &) : Image(), _doCleanup(false) {}
	RenderedImage &operator=(const RenderedImage &) { return *this; }
public:
	RenderedImage(const Common::String &filename, bool &result);

	/**
	    @brief Creates an empty BS_RenderedImage

	    @param Width The width of the image to be created.
	    @param Height The height of the image to be created
	    @param Result Informs the caller, whether the constructor is executed successfully. If it contains false
	                  after the call, do not call methods on the object and destroy the object immediately.
	*/
	RenderedImage(uint width, uint height, bool &result);
	RenderedImage();

	virtual ~RenderedImage();

	virtual int getWidth() const {
		return _surface.w;
	}
	virtual int getHeight() const {
		return _surface.h;
	}
	virtual GraphicEngine::COLOR_FORMATS getColorFormat() const {
		return GraphicEngine::CF_ARGB32;
	}

	void copyDirectly(int posX, int posY);

	virtual bool blit(int posX = 0, int posY = 0,
	                  int flipping = Graphics::FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1,
					  RectangleList *updateRects = 0);
	virtual bool fill(const Common::Rect *pFillRect, uint color);
	virtual bool setContent(const byte *pixeldata, uint size, uint offset = 0, uint stride = 0);
	void replaceContent(byte *pixeldata, int width, int height);
	virtual uint getPixel(int x, int y);

	virtual bool isBlitSource() const {
		return true;
	}
	virtual bool isBlitTarget() const {
		return false;
	}
	virtual bool isScalingAllowed() const {
		return true;
	}
	virtual bool isFillingAllowed() const {
		return false;
	}
	virtual bool isAlphaAllowed() const {
		return true;
	}
	virtual bool isColorModulationAllowed() const {
		return true;
	}
	virtual bool isSetContentAllowed() const {
		return true;
	}

	void setIsTransparent(bool isTransparent) { _isTransparent = isTransparent; }
	virtual bool isSolid() const { return !_isTransparent; }

private:
	Graphics::TransparentSurface _surface;
	bool _doCleanup;
	bool _isTransparent;

	Graphics::Surface *_backSurface;

	void checkForTransparency();
};

} // End of namespace Sword25

#endif
