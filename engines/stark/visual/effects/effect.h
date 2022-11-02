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

#ifndef STARK_VISUAL_EFFECTS_EFFECT_H
#define STARK_VISUAL_EFFECTS_EFFECT_H

#include "engines/stark/visual/visual.h"

#include "common/rect.h"

#include "graphics/pixelformat.h"

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
 * A 2D visual effect overlay
 *
 * The backing surface is alpha blended on top of the scene
 */
class VisualEffect : public Visual {
public:
	explicit VisualEffect(VisualType type, const Common::Point &size, Gfx::Driver *gfx);
	~VisualEffect() override;

protected:
	Gfx::Driver *_gfx;
	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Texture *_texture;
	Graphics::Surface *_surface;

	uint _timeBetweenTwoUpdates;
	int _timeRemainingUntilNextUpdate;
	Common::Point _size;
};

} // End of namespace Stark

#endif // STARK_VISUAL_EFFECTS_EFFECT_H
