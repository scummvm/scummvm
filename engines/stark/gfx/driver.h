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
#include "graphics/surface.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Stark {

class VisualActor;

namespace Gfx {

class Texture;

class Driver {
public:
	static Driver *create();

	virtual ~Driver() {}

	virtual void init() = 0;

	virtual void setGameViewport() = 0;
	virtual void setScreenViewport(bool noScaling) = 0;

	/** Get the screen viewport in actual resolution */
	Common::Rect getScreenViewport() { return _screenViewport; }

	virtual void setupCamera(const Math::Matrix4 &projection, const Math::Matrix4 &view) = 0;

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
	 *
	 */
	virtual VisualActor *createActorRenderer() = 0;

	/** Scale a coordinate from original to current coordinates */
	Common::Point scalePoint(Common::Point point);
	/**
	 * Draw a 2D surface from the specified texture
	 */
	virtual void drawSurface(const Texture *texture, const Common::Point &dest) = 0;

	virtual Texture *createTextureFromString(const Common::String &str, uint32 color);

	virtual void set3DMode() = 0;

	static const int32 kOriginalWidth = 640;
	static const int32 kOriginalHeight = 480;

	static const int32 kTopBorderHeight = 36;
	static const int32 kGameViewportHeight = 365;
	static const int32 kBottomBorderHeight = 79;

	static const int32 kGameViewportWidth = 640;

protected:
	void computeScreenViewport();
	Common::Rect gameViewport() const;

	Common::Rect _screenViewport;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_DRIVER_H
