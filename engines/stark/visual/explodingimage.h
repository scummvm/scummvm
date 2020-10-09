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

#ifndef STARK_VISUAL_EXPLODING_IMAGE_H
#define STARK_VISUAL_EXPLODING_IMAGE_H

#include "engines/stark/visual/visual.h"

#include "common/array.h"
#include "common/rect.h"

#include "graphics/pixelformat.h"

#include "math/vector2d.h"

namespace Graphics {
struct Surface;
}

namespace Stark {

namespace Gfx {
class Driver;
class SurfaceRenderer;
class Texture;
}

/**
 * An image with an animated explosion effect
 *
 * Used by the top bar when picking up an inventory item
 */
class VisualExplodingImage : public Visual {
public:
	static const VisualType TYPE = Visual::kExplodingImage;

	explicit VisualExplodingImage(Gfx::Driver *gfx);
	~VisualExplodingImage() override;

	/** Prepare exploding the specified image */
	void initFromSurface(const Graphics::Surface *surface, uint originalWidth, uint originalHeight);

	/** Render the image at the specified position */
	void render(const Common::Point &position);

private:
	struct ExplosionUnit {
		ExplosionUnit();

		void setPosition(int x, int y);
		void setExplosionSettings(const Common::Point &center, const Common::Point &amplitude, float scale);
		void setColor(uint32 color, const Graphics::PixelFormat &format);
		void update();
		void draw(Graphics::Surface *surface);

		Math::Vector2d _position;
		Math::Vector2d _speed;
		Math::Vector2d _center;
		float _scale;

		int _stillImageTimeRemaining;
		int _explosionFastAccelerationTimeRemaining;
		uint32 _mainColor;
		uint32 _darkColor;
	};

	Gfx::Driver *_gfx;
	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Texture *_texture;
	Graphics::Surface *_surface;

	uint _originalWidth;
	uint _originalHeight;

	Common::Array<ExplosionUnit> _units;
};

} // End of namespace Stark

#endif // STARK_VISUAL_EXPLODING_IMAGE_H
