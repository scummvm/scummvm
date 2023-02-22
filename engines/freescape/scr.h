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

#ifndef FREESCAPE_SCR_H
#define FREESCAPE_SCR_H

#include "image/image_decoder.h"

/*
ZX-Spectrum SCREEN$ decoder based on:
https://gist.github.com/alexanderk23/f459c76847d9412548f7
*/

namespace Common {
class SeekableReadStream;
}

namespace Freescape {

class ScrDecoder : public Image::ImageDecoder {
public:
	ScrDecoder();
	virtual ~ScrDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
private:
	Graphics::Surface *_surface;
	uint32 getPixelAddress(int x, int y);
	uint32 getAttributeAddress(int x, int y);
};
} // End of namespace Freescape

#endif // FREESCAPE_SCR_H
