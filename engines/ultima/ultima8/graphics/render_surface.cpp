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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/soft_render_surface.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/screen.h"

namespace Ultima {
namespace Ultima8 {

RenderSurface::U8PixelFormat RenderSurface::_format;

uint8 RenderSurface::_gamma10toGamma22[256];
uint8 RenderSurface::_gamma22toGamma10[256];

//
// RenderSurface::SetVideoMode()
//
// Desc: Create a standard RenderSurface
// Returns: Created RenderSurface or 0
//

RenderSurface *RenderSurface::SetVideoMode(uint32 width, uint32 height, int bpp) {
	// Set up the pixel format to use
	Graphics::PixelFormat pixelFormat;

	if (bpp == 16) {
		pixelFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	} else if (bpp == 32) {
		pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	} else {
		error("Only 16 bit and 32 bit video modes supported");
	}

	// Set up screen mode
	initGraphics(width, height, &pixelFormat);

	// Set up blitting surface
	Graphics::ManagedSurface *surface = new Graphics::Screen(width, height, pixelFormat);
	assert(surface);

	// Now create the SoftRenderSurface
	RenderSurface *surf;
	if (pixelFormat.bytesPerPixel == 4) surf = new SoftRenderSurface<uint32>(surface);
	else surf = new SoftRenderSurface<uint16>(surface);

	// Initialize gamma correction tables
	for (int i = 0; i < 256; i++) {
		_gamma22toGamma10[i] = static_cast<uint8>(0.5 + (Std::pow(i / 255.0, 2.2 / 1.0) * 255.0));
		_gamma10toGamma22[i] = static_cast<uint8>(0.5 + (Std::pow(i / 255.0, 1.0 / 2.2) * 255.0));
	}

	return surf;
}

// Create a SecondaryRenderSurface with an associated Texture object
RenderSurface *RenderSurface::CreateSecondaryRenderSurface(uint32 width, uint32 height) {
	// Now create the SoftRenderSurface
	RenderSurface *surf;

	// TODO: Change this
	Graphics::ManagedSurface *managedSurface = new Graphics::ManagedSurface(width, height, _format);
	if (_format.bytesPerPixel == 4) surf = new SoftRenderSurface<uint32>(managedSurface);
	else surf = new SoftRenderSurface<uint16>(managedSurface);
	return surf;
}

RenderSurface::~RenderSurface() {
}

Graphics::PixelFormat RenderSurface::getPixelFormat() {
	return g_system->getScreenFormat();
}

} // End of namespace Ultima8
} // End of namespace Ultima
