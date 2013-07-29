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

#include "engines/util.h"
#include "graphics/decoders/tga.h"

#include "zvision/render_manager.h"
#include "zvision/lzss_read_stream.h"

namespace ZVision {

RenderManager::RenderManager(OSystem *system, const int width, const int height)
	: _system(system),
	  _width(width),
	  _height(height),
	  _pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),	// RGB555
	  _currentVideo(0),
	  _scaledVideoFrameBuffer(0),
	  _renderTable(width, height) {
}

/**
 * Initialize graphics
 */
void RenderManager::initialize() {
	initGraphics(_width, _height, true, &_pixelFormat);
}

void RenderManager::updateScreen(bool isConsoleActive) {
	if (_currentVideo != 0)
		continueVideo();

	if (_needsScreenUpdate || isConsoleActive) {
		_system->updateScreen();
		_needsScreenUpdate = false;
	}
}

void RenderManager::renderSubRectToScreen(uint16 *buffer, uint32 imageWidth, uint32 imageHeight, uint32 horizontalPitch, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle) {
	// Panoramas are transposed
	// The actual data is transposed in the RenderTable lookup
	if (_renderTable.getRenderState() == RenderTable::PANORAMA || _renderTable.getRenderState() == RenderTable::TILT) {
		uint32 temp = imageHeight;
		imageHeight = imageWidth;
		imageWidth = temp;
	}
	
	// Check if we truly want a subRect of the image
	if (subRectangle.isEmpty())
		subRectangle = Common::Rect(imageWidth, imageHeight);

	// Clip to image bounds
	subRectangle.clip(imageWidth, imageHeight);
	// Clip destRect to screen bounds
	Common::Rect destRect(destinationX, destinationY, destinationX + subRectangle.width(), destinationY + subRectangle.height());
	destRect.clip(_width, _height);
	// Clip subRect to screen bounds
	subRectangle.translate(destRect.left - destinationX, destRect.top - destinationY);
	subRectangle.setWidth(destRect.width());
	subRectangle.setHeight(destRect.height());

	// Check for validity
	if (!subRectangle.isValidRect() || subRectangle.isEmpty() || !destRect.isValidRect() || destRect.isEmpty())
		return;

	if (_renderTable.getRenderState() == RenderTable::FLAT) {
		_system->copyRectToScreen(buffer + subRectangle.top * horizontalPitch + subRectangle.left, horizontalPitch, destRect.left, destRect.top, destRect.width(), destRect.height());
	} else {
		uint16 *destBuffer = new uint16[destRect.width() * destRect.height()];
		_renderTable.mutateImage((uint16 *)buffer, destBuffer, imageWidth, imageHeight, subRectangle, destRect);

		_system->copyRectToScreen(destBuffer, subRectangle.width() * sizeof(uint16), destRect.left, destRect.top, destRect.width(), destRect.height());
		delete[] destBuffer;
	}
}

void RenderManager::renderImageToScreen(const Common::String &fileName, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle) {
	Common::File file;

	if (!file.open(fileName)) {
		warning("Could not open file %s", fileName.c_str());
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
		uint32 imageWidth = file.readSint32LE();
		uint32 imageHeight = file.readSint32LE();

		LzssReadStream stream(&file);
		byte *buffer = new byte[decompressedSize];
		stream.read(buffer, decompressedSize);

		uint32 horizontalPitch = imageWidth * sizeof(uint16);

		renderSubRectToScreen((uint16 *)buffer, imageWidth, imageHeight, horizontalPitch, destinationX, destinationY, subRectangle);
		delete[] buffer;
	} else {
		// Reset the cursor
		file.seek(0);

		// Decode
		Graphics::TGADecoder tga;
		if (!tga.loadStream(file)) {
			warning("Error while reading TGA image");
			return;
		}

		const Graphics::Surface *tgaSurface = tga.getSurface();
		renderSubRectToScreen((uint16 *)tgaSurface->pixels, tgaSurface->w, tgaSurface->h, tgaSurface->pitch, destinationX, destinationY, subRectangle);		

		tga.destroy();
	}

	_needsScreenUpdate = true;
}

void RenderManager::setRenderState(RenderTable::RenderState state) {
	_renderTable.setRenderState(state);
}

} // End of namespace ZVision
