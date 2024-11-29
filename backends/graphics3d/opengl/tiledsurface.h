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

#ifndef BACKENDS_GRAPHICS3D_OPENGL_TILED_SURFACE_H
#define BACKENDS_GRAPHICS3D_OPENGL_TILED_SURFACE_H

#include "graphics/opengl/system_headers.h"

#include "common/array.h"
#include "common/rect.h"

#include "graphics/surface.h"

namespace OpenGL {

class Texture;
class SurfaceRenderer;

/**
 * Surface implementation using OpenGL texture tiles
 */
class TiledSurface {
public:
	TiledSurface(uint width, uint height, const Graphics::PixelFormat &pixelFormat);
	~TiledSurface();

	/**
	 * Copy image data to the surface.
	 *
	 * The format of the input data needs to match the format returned by
	 * getFormat.
	 * This does not immediately updates the textures.
	 *
	 * @param x        X coordinate of upper left corner to copy data to.
	 * @param y        Y coordinate of upper left corner to copy data to.
	 * @param w        Width of the image data to copy.
	 * @param h        Height of the image data to copy.
	 * @param src      Pointer to image data.
	 * @param srcPitch The number of bytes in a row of the image data.
	 */
	void copyRectToSurface(const void *src, int srcPitch, int x, int y, int w, int h);

	/**
	 * Update the OpenGL textures from the backing surface
	 */
	void update();

	/**
	 * Draw as a 2D surface
	 *
	 * The destination rect is as follow :
	 * x: left [0.0, 1.0] right
	 * y: top [0.0, 1.0] bottom
	 *
	 * @param surfaceRenderer
	 */
	void draw(SurfaceRenderer *surfaceRenderer) const;

	/**
	 * Fill the surface with a fixed color.
	 *
	 * @param color Color value in format returned by getFormat.
	 */
	void fill(uint32 color);

	/**
	 * Get the backing surface
	 *
	 * This can be used to update the image data, but causes a full update.
	 * Using copyRectToSurface is preferred
	 */
	Graphics::Surface *getBackingSurface();

	/**
	 * Get the backing surface
	 *
	 * This const version can be used to read image data without causing
	 * a full texture invalidation.
	 */
	const Graphics::Surface *getBackingSurface() const;

	/**
	 * Get the dimensions in pixels of the surface
	 *
	 * This does not include the unused area in the textures at the edges
	 * of the grid.
	 */
	uint16 getWidth() const { return _backingSurface.w; }
	uint16 getHeight() const { return _backingSurface.h; }

private:
	static const uint maxTextureSize = 256;

	struct Tile {
		Texture *texture;
		Common::Rect rect;
		bool dirty;
	};

	Graphics::Surface _backingSurface;
	Common::Array<Tile> _tiles;

	void invalidateAllTiles();
};

} // End of namespace OpenGL

#endif
