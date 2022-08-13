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
#include "common/str.h"
#include "common/stream.h"
#include "common/path.h"
#include "image/tga.h"

#include "tetraedge/te/te_tga.h"

namespace Tetraedge {

TeTga::TeTga() {
}

/*static*/
bool TeTga::matchExtension(const Common::String &extn) {
	return extn == "tga";
}

bool TeTga::load(Common::SeekableReadStream &stream) {
	if (_loadedSurface)
		delete _loadedSurface;
	_loadedSurface = nullptr;

	Image::TGADecoder tga;
	if (!tga.loadStream(stream))
		return false;

	_loadedSurface = tga.getSurface()->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	return true;
}

} // end namespace Tetraedge
