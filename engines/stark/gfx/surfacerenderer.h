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

#ifndef STARK_GFX_SURFACE_RENDERER_H
#define STARK_GFX_SURFACE_RENDERER_H

#include "common/rect.h"

namespace Stark {
namespace Gfx {

class Texture;

/**
 * A renderer to draw textures as two dimensional surfaces to the current viewport
 */
class SurfaceRenderer {
public:
	SurfaceRenderer();
	virtual ~SurfaceRenderer();

	/**
	 * Draw a 2D surface from the specified texture
	 */
	virtual void render(const Texture *texture, const Common::Point &dest) = 0;

	/**
	 * Draw a 2D surface from the specified texture with given width and height
	 */
	virtual void render(const Texture *texture, const Common::Point &dest, uint width, uint height) = 0;

	/**
	 * When this is set to true, the texture size is expected to be in current
	 * coordinates, and is to be drawn without scaling.
	 *
	 * This setting does not affect the destination point coordinates
	 */
	void setNoScalingOverride(bool noScalingOverride);

	/**
	 * The fade level is added to the color value of each pixel
	 *
	 * It is a value between -1 and 1
	 */
	void setFadeLevel(float fadeLevel);

	/**
	 * Align vertex coordinates to the native pixel grid
	 */
	void setSnapToGrid(bool snapToGrid);

protected:
	bool _noScalingOverride;
	float _fadeLevel;
	bool _snapToGrid;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_SURFACE_RENDERER_H
