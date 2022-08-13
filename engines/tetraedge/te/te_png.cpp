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

#include "common/file.h"
#include "image/png.h"
#include "graphics/surface.h"

#include "tetraedge/te/te_png.h"

namespace Tetraedge {

TePng::TePng() {
}

TePng::~TePng() {
}

/*static*/
bool TePng::matchExtension(const Common::String &extn) {
	return extn == "png";
}

bool TePng::load(Common::SeekableReadStream &stream) {
	if (_loadedSurface)
		delete _loadedSurface;
	_loadedSurface = nullptr;

	Image::PNGDecoder png;
	if (!png.loadStream(stream))
		return false;

	//if (png.getTransparentColor() == -1) {
	//	_loadedSurface = png.getSurface()->convertTo(Graphics::createPixelFormat<888>());
	//} else {
		_loadedSurface = png.getSurface()->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	//}
	return true;
}

TeImage::Format TePng::imageFormat() {
	if (_loadedSurface) {
		if (_loadedSurface->format == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24))
			return TeImage::RGBA8;
		else if (_loadedSurface->format == Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0))
			return TeImage::RGB8;
	}
	return TeImage::INVALID;
}

} // end namespace Tetraedge
