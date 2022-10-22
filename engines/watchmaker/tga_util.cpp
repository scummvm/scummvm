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

#include "watchmaker/tga_util.h"
#include "image/tga.h"
#include "watchmaker/types.h"
#include "watchmaker/render.h"
#include "watchmaker/utils.h"

namespace Watchmaker {

Graphics::Surface *ReadTgaImage(const char *Name, Common::SeekableReadStream &stream, Graphics::PixelFormat format, unsigned int flag) {
	Image::TGADecoder tgaDecoder;
	if (!tgaDecoder.loadStream(stream)) {
		error("Failed to load TGA: %s", Name);
	}
	auto surface = tgaDecoder.getSurface();
	return surface->convertTo(format);
#if 0
	if (flag & 1)                                                                                // read alpa byte?
		bAlpha = 1;
	if (flag & rSURFACEFLIP)                                                                    // flip tga orientation?
		bFlip = 1;
#endif
}

} // End of namespace Watchmaker
