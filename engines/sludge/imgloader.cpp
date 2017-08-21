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

#include "sludge/allfiles.h"
#include "sludge/hsi.h"
#include "sludge/imgloader.h"
#include "sludge/sludge.h"

namespace Sludge {

bool ImgLoader::loadImage(Common::SeekableReadStream *stream, Graphics::Surface *dest, int reserve) {
	debugC(3, kSludgeDebugGraphics, "Loading image at position: %i", stream->pos());
	int32 start_ptr = stream->pos();
	if (!loadPNGImage(stream, dest)) {
		stream->seek(start_ptr);
		if (!loadHSIImage(stream, dest, reserve)) {
			return false;
		}
	}
	return true;
}

bool ImgLoader::loadPNGImage(Common::SeekableReadStream *stream, Graphics::Surface *dest, bool checkSig) {
	::Image::PNGDecoder png;

	// set skip signature
	if (!checkSig) {
		png.setSkipSignature(true);
	}

	if (!png.loadStream(*stream))
		return false;

	// set value back
	if (!checkSig) {
		png.setSkipSignature(false);
	}

	const Graphics::Surface *sourceSurface = png.getSurface();
	Graphics::Surface *pngSurface = sourceSurface->convertTo(*g_sludge->getScreenPixelFormat(), png.getPalette());
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
