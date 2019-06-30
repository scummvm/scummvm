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

#include "common/system.h"
#include "titanic/support/image_decoders.h"

namespace Titanic {

void CJPEGDecode::decode(OSVideoSurface &surface, const CString &name) {
	// Open up the resource
	StdCWadFile file;
	file.open(name);

	// Use the ScummVM decoder to decode it
	setOutputPixelFormat(g_system->getScreenFormat());
	loadStream(*file.readStream());
	const Graphics::Surface *srcSurf = getSurface();

	// Resize the surface if necessary
	if (!surface.hasSurface() || surface.getWidth() != srcSurf->w
			|| surface.getHeight() != srcSurf->h)
		surface.recreate(srcSurf->w, srcSurf->h, 16);

	// Copy the decoded surface
	surface.lock();

	assert(srcSurf->format == surface._rawSurface->format);

	Common::copy((const byte *)srcSurf->getPixels(), (const byte *)srcSurf->getPixels() +
		surface.getPitch() * surface.getHeight(), (byte *)surface._rawSurface->getPixels());

	surface.unlock();
}

/*------------------------------------------------------------------------*/

void CTargaDecode::decode(OSVideoSurface &surface, const CString &name) {
	// Open up the resource
	StdCWadFile file;
	file.open(name);

	// Use the ScucmmVM deoder to decode it
	loadStream(*file.readStream());
	const Graphics::Surface *srcSurf = getSurface();

	// Resize the surface if necessary
	if (!surface.hasSurface() || surface.getWidth() != srcSurf->w
			|| surface.getHeight() != srcSurf->h)
		surface.recreate(srcSurf->w, srcSurf->h, 16);

	// Convert the decoded surface to the correct pixel format, and then copy it over
	surface.lock();
	Graphics::Surface *convertedSurface = srcSurf->convertTo(surface._rawSurface->format);

	Common::copy((byte *)convertedSurface->getPixels(), (byte *)convertedSurface->getPixels() +
		surface.getPitch() * surface.getHeight(), (byte *)surface._rawSurface->getPixels());

	convertedSurface->free();
	delete convertedSurface;
	surface.unlock();
}

} // End of namespace Titanic
