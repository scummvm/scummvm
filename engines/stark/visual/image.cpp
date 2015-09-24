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

#include "engines/stark/visual/image.h"

#include "common/stream.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "engines/stark/debug.h"
#include "engines/stark/formats/xmg.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

namespace Stark {

VisualImageXMG::VisualImageXMG(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_texture(nullptr),
		_surface(nullptr),
		_width(0),
		_height(0) {
}

VisualImageXMG::~VisualImageXMG() {
	if (_surface) {
		_surface->free();
	}
	delete _surface;
	delete _texture;
}

void VisualImageXMG::setHotSpot(const Common::Point &hotspot) {
	_hotspot = hotspot;
}

void VisualImageXMG::load(Common::ReadStream *stream) {
	delete _texture;

	// Decode the XMG
	_surface = Formats::XMGDecoder::decode(stream);
	_width = _surface->w;
	_height = _surface->h;
	_texture = _gfx->createTexture(_surface);
}

void VisualImageXMG::render(const Common::Point &position, bool useOffset) {
	Common::Point drawPos = useOffset ? position - _hotspot : position;
	_gfx->drawSurface(_texture, drawPos);
}

bool VisualImageXMG::isPointSolid(const Common::Point &point) const {
	if (_width < 32 && _height < 32) {
		return true; // Small images are always solid
	}

	// Maybe implement this method in some other way to avoid having to keep the surface in memory
	const uint32 *ptr = (const uint32 *) _surface->getBasePtr(point.x, point.y);
	return ((*ptr) & 0xFF000000) == 0xFF000000;
}

} // End of namespace Stark
