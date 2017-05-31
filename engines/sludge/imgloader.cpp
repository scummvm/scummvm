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

#include "common/debug.h"
#include "common/stream.h"
#include "image/png.h"
#include "graphics/surface.h"

#include "allfiles.h"
#include "hsi.h"
#include "imgloader.h"
#include "colours.h"
#include "sludge.h"

namespace Sludge {

bool ImgLoader::loadImage(Common::SeekableReadStream *stream, Graphics::Surface *dest, int reserve) {
	debug(kSludgeDebugDataLoad, "Loading image at position: %i", stream->pos());
	int32 start_ptr = stream->pos();
	if (!loadPNGImage(stream, dest)) {
		stream->seek(start_ptr);
		if (!loadHSIImage(stream, dest, reserve)) {
			return false;
		}
	}
	return true;
}

bool ImgLoader::loadPNGImage(Common::SeekableReadStream *stream, Graphics::Surface *dest) {
	::Image::PNGDecoder png;
	if (!png.loadStream(*stream))
		return false;
	const Graphics::Surface *sourceSurface = png.getSurface();
	Graphics::Surface *pngSurface = sourceSurface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), png.getPalette());
	dest->copyFrom(*pngSurface);
	pngSurface->free();
	delete pngSurface;
	return true;
}

bool ImgLoader::loadHSIImage(Common::SeekableReadStream *stream, Graphics::Surface *dest, int reserve) {
	HSIDecoder hsiDecoder;
	hsiDecoder.setReserve(reserve);
	if (!hsiDecoder.loadStream(*stream)) {
		return false;
	}
	dest->copyFrom(*(hsiDecoder.getSurface()));
	return true;
}

} // End of namespace Sludge
