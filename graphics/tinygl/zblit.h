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

#ifndef GRAPHICS_TINYGL_ZBLIT_H_
#define GRAPHICS_TINYGL_ZBLIT_H_

#include "common/rect.h"

#include "graphics/surface.h"
#include "graphics/tinygl/zblit_public.h"

namespace TinyGL {

struct BlitImage;

namespace Internal {
	/**
	@brief Performs a cleanup of disposed blit images.
	*/
	void tglCleanupImages(); // This function checks if any blit image is to be cleaned up and deletes it.

	// Documentation for those is the same as the one before, only those function are the one that actually execute the correct code path.
	void tglBlit(BlitImage *blitImage, const BlitTransform &transform);

	// Disables blending, transforms and tinting.
	void tglBlitFast(BlitImage *blitImage, int x, int y);

	void tglBlitZBuffer(BlitImage *blitImage, int x, int y);

	/**
	@brief Sets up a scissor rectangle for blit calls: every blit call is affected by this rectangle.
	*/
	void tglBlitSetScissorRect(const Common::Rect &rect);
	void tglBlitResetScissorRect();
} // end of namespace Internal

} // end of namespace TinyGL

#endif // GRAPHICS_TINYGL_ZBLIT_H_
