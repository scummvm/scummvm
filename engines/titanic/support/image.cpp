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

#include "titanic/support/image.h"
#include "titanic/support/files_manager.h"
#include "image/bmp.h"
#include "titanic/titanic.h"

namespace Titanic {

void Image::load(const CString &resName) {
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource(resName);
	loadBitmap(*stream);
	delete stream;
}

void Image::loadBitmap(Common::SeekableReadStream &s) {
	::Image::BitmapDecoder decoder;
	decoder.loadStream(s);
	const Graphics::Surface *src = decoder.getSurface();
	Graphics::PixelFormat scrFormat = g_system->getScreenFormat();

	if (src->format == scrFormat) {
		create(src->w, src->h, scrFormat);
		blitFrom(*src);
	} else {
		// Convert the loaded surface to the screen surface format
		const byte *palette = decoder.getPalette();
		Graphics::Surface *surface = src->convertTo(scrFormat, palette);
		create(surface->w, surface->h, scrFormat);
		blitFrom(*surface);

		surface->free();
		delete surface;
	}
}

} // End of namespace Titanic
