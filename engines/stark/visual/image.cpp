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

namespace Stark {

VisualImageXMG::VisualImageXMG() :
		Visual(TYPE),
		_surface(NULL) {
}

VisualImageXMG::~VisualImageXMG() {
	// Free the surface
	if (_surface)
		_surface->free();
	delete _surface;
}

VisualImageXMG *VisualImageXMG::load(Common::ReadStream *stream) {
	// Create the element to return
	VisualImageXMG *element = new VisualImageXMG();

	// Decode the XMG
	element->_surface = XMGDecoder::decode(stream);

	return element;
}

void VisualImageXMG::render(GfxDriver *gfx, const Common::Point &position) {
	// Draw the current element
	gfx->drawSurface(_surface, position);
}

} // End of namespace Stark
