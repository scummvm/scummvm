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
#include "common/rect.h"

#include "graphics/decoders/tga.h"

#include "zvision/render_manager.h"
#include "zvision/lzss_read_stream.h"

namespace ZVision {

RenderManager::RenderManager(OSystem *system, const int width, const int height)
	: _system(system),
	  _width(width),
	  _height(height),
	  _renderTable(width, height) {
}

void RenderManager::renderImageToScreen(const Common::String &fileName, uint32 x, uint32 y) {
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

		delete[] buffer;
	} else {
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
	}

	_needsScreenUpdate = true;
}

void RenderManager::generatePanoramaLookupTable() {
	float fieldOfView = _panoramaOptions.fieldOfView;
	float scale = _panoramaOptions.linearScale;
	_renderTable.clear();

	double halfWidth = (double)_width / 2.0;
	double halfHeight = (double)_height / 2.0;

	double fovRadians = (fieldOfView * 3.14159265 / 180.0);
	double halfHeightOverTan = halfHeight / tan(fovRadians);
	double tanOverHalfHeight = tan(fovRadians) / halfHeight;

	for (int32 x = 0; x < _width; x++) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (vertical line on half of screen)
		double xPos = (double)x - halfWidth + 0.01;

		double tempX = atan(xPos*tanOverHalfHeight);
		double scaledX = scale * halfHeightOverTan * tempX;
		double nn = cos(tempX);
		double newHalfWidth = halfHeight * nn * halfHeightOverTan * tanOverHalfHeight*2.0;

		int32 newX = floor(scaledX);// + half_w);

		double yScale = newHalfWidth / (double)_height;
		double et2 = ((double)_height - newHalfWidth) / 2.0;

		for (int32 y = 0; y < _height; y++) {
			double et1 = (double)y*yScale;

			_renderTable(x, y).x = newX; //pixel index

			int32 newY = floor(et2 + et1);
			_renderTable(x, y).y = newY; //pixel index
		}
	}
}
} // End of namespace ZVision
