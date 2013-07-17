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
#include "zvision/lzss_read_stream.h"

namespace ZVision {

void ZVision::renderImageToScreen(const Common::String &fileName, uint32 x, uint32 y) {
	Common::File file;

	if (!file.open(fileName)) {
		error("Could not open file %s", fileName.c_str());
		return;
	}

	// Read the magic number
	// Some files are true TGA, while others are TGZ
	uint32 fileType;
	fileType = file.readUint32BE();

	// Check for TGZ files
	if (fileType == MKTAG('T', 'G', 'Z', '\0')) {
		// TGZ files have a header and then Bitmap data that is compressed with LZSS
		uint32 decompressedSize = file.readSint32LE();
		uint32 width = file.readSint32LE();
		uint32 height = file.readSint32LE();

		LzssReadStream stream(&file);
		byte *buffer = new byte[decompressedSize];
		stream.read(buffer, decompressedSize);

		_system->copyRectToScreen(buffer, width * 2, x, y, width, height);
	} else {
		// Reset the cursor
		file.seek(0);

		// Decode
		Graphics::TGADecoder tga;
		if (!tga.loadStream(file)) {
			error("Error while reading TGA image");
			return;
		}

		const Graphics::Surface *tgaSurface = tga.getSurface();
		_system->copyRectToScreen(tgaSurface->pixels, tgaSurface->pitch, x, y, tgaSurface->w, tgaSurface->h);

		tga.destroy();
	}

	_needsScreenUpdate = true;
}

} // End of namespace ZVision
