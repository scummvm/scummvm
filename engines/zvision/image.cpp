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

	if (file.open(fileName)) {
		// Read the magic number
		// Some files are true TGA, while others are TGZ
		char fileType[4];
		file.read(fileType, 4);

		// Check for true TGA files
		if (fileType[0] == 'T' && fileType[1] == 'G' && fileType[2] == 'A' && fileType[3] == '\0') {
			// Reset the cursor
			file.seek(0);

			// Decode
			Graphics::TGADecoder tga;
			if (!tga.loadStream(file))
				error("Error while reading TGA image");
			file.close();

			const Graphics::Surface *tgaSurface = tga.getSurface();

			_system->copyRectToScreen(tgaSurface->pixels, tgaSurface->pitch, x, y, tgaSurface->w, tgaSurface->h);

			tga.destroy();
		} else {
			// TGZ files have a header and then Bitmap data that is compressed with LZSS
			uint32 decompressedSize = file.readSint32LE();
			uint32 width = file.readSint32LE();
			uint32 height = file.readSint32LE();

			LzssReadStream stream(&file, false, decompressedSize);
			byte *buffer = new byte[stream.currentSize()];
			stream.read(buffer, stream.currentSize());

			//Graphics::PixelFormat format(16, 5, 6, 5, 0, 11, 5, 0, 0);
			// Graphics::PixelFormat format(16, 5, 5, 5, 1, 11, 6, 1, 0);

			_system->copyRectToScreen(buffer, width * 2, x, y, width, height);
		}
		

		_needsScreenUpdate = true;
	} else {
		error("Could not open file %s", fileName.c_str());
	}
}

} // End of namespace ZVision
