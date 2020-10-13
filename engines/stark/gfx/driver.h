/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_GFX_DRIVER_H
#define STARK_GFX_DRIVER_H

#include "common/rect.h"
#include "graphics/pixelformat.h"

namespace Graphics {
struct Surface;
}

namespace Stark {

class VisualActor;
class VisualProp;

namespace Gfx {

class SurfaceRenderer;
class FadeRenderer;
class Texture;

class Driver {
public:
	static Driver *create();

	virtual ~Driver() {}

	virtual void init() = 0;

	bool computeScreenViewport();
	virtual void setScreenViewport(bool noScaling) = 0; // deprecated

	virtual void setViewport(const Common::Rect &rect) = 0;

	/** Get the screen viewport in actual resolution */
	Common::Rect getScreenViewport() { return _screenViewport; }

	Common::Rect gameViewport() const;

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	/**
	 * Create a new texture
	 *
	 * The caller is responsible for freeing it.
	 *
	 */
	virtual Texture *createTexture(const Graphics::Surface *surface = nullptr, const byte *palette = nullptr) = 0;

	/**
	 * Create a new actor renderer
	 *
	 * The caller is responsible for freeing it.
	 */
	virtual VisualActor *createActorRenderer() = 0;

	/**
	 * Create a new prop renderer
	 *
	 * The caller is responsible for freeing it.
	 */
	virtual VisualProp *createPropRenderer() = 0;

	/**
	 * Create a new surface renderer
	 *
	 * The caller is responsible for freeing it.
	 */
	virtual SurfaceRenderer *createSurfaceRenderer() = 0;

	/**
	 * Create a new fade renderer
	 *
	 * The caller is responsible for freeing it.
	 */
	virtual FadeRenderer *createFadeRenderer() = 0;

	/** Checks if a screenpoint coord is within window bounds */
	bool isPosInScreenBounds(const Common::Point &point) const;

	/** Convert a coordinate from current to original resolution */
	Common::Point convertCoordinateCurrentToOriginal(const Common::Point &point) const;

	/** Scale a width value from original resolution to current resolution */
	uint scaleWidthOriginalToCurrent(uint width) const;

	/** Scale a height value from original resolution to current resolution */
	uint scaleHeightOriginalToCurrent(uint height) const;

	/** Scale a width value from current resolution to original resolution */
	uint scaleWidthCurrentToOriginal(uint width) const;

	/** Scale a height value from current resolution to original resolution */
	uint scaleHeightCurrentToOriginal(uint width) const;

	/**
	 * Textures are expected to be in the RGBA byte order
	 *
	 * That is to say bitmaps sent to OpenGL need to have the following layout:
	 * R G B A R G B A, ...
	 *
	 * This method can be used to retrieve what that means in terms
	 * of pixel format according to the current platform's endianness.
	 */
	static const Graphics::PixelFormat getRGBAPixelFormat();

	/** Grab a screenshot of the currently active viewport as defined by setViewport */
	virtual Graphics::Surface *getViewportScreenshot() const = 0;

	virtual void set3DMode() = 0;

	static const int32 kOriginalWidth = 640;
	static const int32 kOriginalHeight = 480;

	static const int32 kTopBorderHeight = 36;
	static const int32 kGameViewportHeight = 365;
	static const int32 kBottomBorderHeight = 79;

	static const int32 kGameViewportWidth = 640;

protected:
	static void flipVertical(Graphics::Surface *s);

	Common::Rect _screenViewport;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_DRIVER_H
