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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "common/scummsys.h"

#include "common/file.h"
#include "common/system.h"

#include "graphics/decoders/tga.h"

#include "zvision/zvision.h"

namespace ZVision {

void ZVision::renderImageToScreen(const Common::String &fileName, uint32 x, uint32 y, uint32 width, uint32 height) {
	Common::File file;

	if (file.open(fileName)) {
		Graphics::TGADecoder tga;
		if (!tga.loadStream(file))
			error("Error while reading TGA image");
		file.close();

		const Graphics::Surface *tgaSurface = tga.getSurface();

		_system->copyRectToScreen(tgaSurface->pixels, tgaSurface->pitch, x, y, width, height);

		tga.destroy();

		_needsScreenUpdate = true;
	}
}

} // End of namespace ZVision
