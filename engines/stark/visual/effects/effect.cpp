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

#include "engines/stark/visual/effects/effect.h"

#include "graphics/surface.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"

namespace Stark {

VisualEffect::VisualEffect(VisualType type, const Common::Point &size, Gfx::Driver *gfx) :
		Visual(type),
		_size(size),
		_gfx(gfx),
		_timeBetweenTwoUpdates(3 * 33), // ms (frames @ 30 fps)
		_timeRemainingUntilNextUpdate(0) {
	_surface = new Graphics::Surface();
	_surface->create(size.x, size.y, Gfx::Driver::getRGBAPixelFormat());

	_texture = _gfx->createBitmap(_surface);
	_texture->setSamplingFilter(StarkSettings->getImageSamplingFilter());

	_surfaceRenderer = _gfx->createSurfaceRenderer();
}

VisualEffect::~VisualEffect() {
	if (_surface) {
		_surface->free();
	}
	delete _surface;
	delete _texture;
	delete _surfaceRenderer;
}

} // End of namespace Stark
