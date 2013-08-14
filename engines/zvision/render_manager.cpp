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
#include "common/stream.h"

#include "engines/util.h"
#include "graphics/decoders/tga.h"

#include "zvision/render_manager.h"
#include "zvision/lzss_read_stream.h"

namespace ZVision {

RenderManager::RenderManager(OSystem *system, const Common::Rect workingWindow)
	: _system(system),
	  _workingWidth(workingWindow.width()),
	  _workingHeight(workingWindow.height()),
	  _workingWindow(workingWindow),
	  _currentBackground(0),
	  _backgroundWidth(0),
	  _backgroundHeight(0),
	  _backgroundInverseVelocity(0),
	  _accumulatedVelocityMilliseconds(0),
	  _renderTable(workingWindow.width(), workingWindow.height()) {
}

RenderManager::~RenderManager() {
	if (_currentBackground != 0) {
		delete _currentBackground;
	}
}

void RenderManager::renderSubRectToScreen(uint16 *buffer, uint32 imageWidth, uint32 imageHeight, uint32 horizontalPitch, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool autoCenter) {
	// Panoramas are transposed
	// The actual data is transposed in mutateImage
	if (_renderTable.getRenderState() == RenderTable::PANORAMA || _renderTable.getRenderState() == RenderTable::TILT) {
		uint32 temp = imageHeight;
		imageHeight = imageWidth;
		imageWidth = temp;
void RenderManager::update(uint deltaTimeInMillis) {
	// An inverse velocity of 0 would be infinitely fast, so we'll let 0 mean no velocity.
	if (_backgroundInverseVelocity == 0)
		return;

	_accumulatedVelocityMilliseconds += deltaTimeInMillis;

	int absVelocity = abs(_backgroundInverseVelocity);

	uint numberOfSteps = 0;
	while (_accumulatedVelocityMilliseconds >= absVelocity) {
		_accumulatedVelocityMilliseconds -= absVelocity;
		numberOfSteps++;
	}

	// Choose the direction of movement using the sign of the velocity
	moveBackground(_backgroundInverseVelocity < 0 ? -numberOfSteps : numberOfSteps);
}
	}
	
	// If subRect is empty, use the entire image
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

	}

	_backgroundOffset = Common::Point(destRect.left, destRect.top);

	if (_renderTable.getRenderState() == RenderTable::FLAT) {
		// Convert destRect to screen space by adding _workingWindowOffset
		_system->copyRectToScreen(buffer + subRectangle.top * horizontalPitch + subRectangle.left, horizontalPitch, destRect.left + _workingWindow.left, destRect.top + _workingWindow.top, destRect.width(), destRect.height());
	} else {
		uint16 *destBuffer = new uint16[destRect.width() * destRect.height()];
		_renderTable.mutateImage((uint16 *)buffer, destBuffer, imageWidth, imageHeight, subRectangle, destRect);

		// Convert destRect to screen space by adding _workingWindow offest
		_system->copyRectToScreen(destBuffer, subRectangle.width() * sizeof(uint16), destRect.left + _workingWindow.left, destRect.top + _workingWindow.top, destRect.width(), destRect.height());
		delete[] destBuffer;
	}
}

void RenderManager::renderImageToScreen(const Common::String &fileName, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool autoCenter) {
	Common::File file;

	if (!file.open(fileName)) {
		warning("Could not open file %s", fileName.c_str());
		return;
	}

	renderImageToScreen(file, destinationX, destinationY, subRectangle, autoCenter);
}

void RenderManager::renderImageToScreen(Common::SeekableReadStream &stream, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool autoCenter) {
	// Read the magic number
	// Some files are true TGA, while others are TGZ
	uint32 fileType;
	fileType = stream.readUint32BE();

	// Check for TGZ files
	if (fileType == MKTAG('T', 'G', 'Z', '\0')) {
		// TGZ files have a header and then Bitmap data that is compressed with LZSS
		uint32 decompressedSize = stream.readSint32LE();
		uint32 imageWidth = stream.readSint32LE();
		uint32 imageHeight = stream.readSint32LE();

		LzssReadStream lzssStream(&stream);
		byte *buffer = new byte[decompressedSize];
		lzssStream.read(buffer, decompressedSize);

		uint32 horizontalPitch = imageWidth * sizeof(uint16);

		renderSubRectToScreen((uint16 *)buffer, imageWidth, imageHeight, horizontalPitch, destinationX, destinationY, subRectangle, autoCenter);
		delete[] buffer;
	} else {
		// Reset the cursor
		stream.seek(0);

		// Decode
		Graphics::TGADecoder tga;
		if (!tga.loadStream(stream)) {
			warning("Error while reading TGA image");
			return;
		}

		const Graphics::Surface *tgaSurface = tga.getSurface();
		renderSubRectToScreen((uint16 *)tgaSurface->pixels, tgaSurface->w, tgaSurface->h, tgaSurface->pitch, destinationX, destinationY, subRectangle, autoCenter);

		tga.destroy();
	}
}

const Common::Point RenderManager::screenSpaceToImageSpace(const Common::Point &point) {
	// Convert from screen space to working window space
	Common::Point newPoint(point - Common::Point(_workingWindow.left, _workingWindow.top));

	if (_renderTable.getRenderState() == RenderTable::PANORAMA || _renderTable.getRenderState() == RenderTable::TILT) {
		newPoint = _renderTable.convertWarpedCoordToFlatCoord(newPoint);
	}

	newPoint -= _backgroundOffset;
	if (newPoint.x < 0)
		newPoint.x += _backgroundWidth;
	if (newPoint.y < 0)
		newPoint.y += _backgroundHeight;

	return newPoint;
}

RenderTable *RenderManager::getRenderTable() {
	return &_renderTable;
}

void RenderManager::setBackgroundImage(const Common::String &fileName) {
	Common::File *file = new Common::File;

	if (!file->open(fileName)) {
		warning("Could not open file %s", fileName.c_str());
		return;
	}

	if (_currentBackground != 0) {
		delete _currentBackground;
	}
	_currentBackground = file;

	renderImageToScreen(*_currentBackground, 0, 0, Common::Rect(), true);
void RenderManager::setBackgroundPosition(int offset) {
	if (_renderTable.getRenderState() == RenderTable::TILT) {
		_backgroundOffset = Common::Point(0, offset);
	} else {
		_backgroundOffset = Common::Point(offset, 0);
	}
}

void RenderManager::setBackgroundVelocity(int velocity) {
	// setBackgroundVelocity(0) will be called quite often, so make sure
	// _backgroundInverseVelocity isn't already 0 to prevent an extraneous assignment
	if (velocity == 0) {
		if (_backgroundInverseVelocity != 0) {
			_backgroundInverseVelocity = 0;
		}
	} else {
		_backgroundInverseVelocity = 1000 / velocity;
	}
}

void RenderManager::moveBackground(int offset) {
	if (_renderTable.getRenderState() == RenderTable::TILT) {
		_backgroundOffset += Common::Point(0, offset);
	} else {
		_backgroundOffset += Common::Point(offset, 0);
	}

	// Make sure the offset is within image bounds
	if (_backgroundOffset.x < 0)
		_backgroundOffset.x += _backgroundWidth;
	if (_backgroundOffset.x > _backgroundWidth)
		_backgroundOffset.x -= _backgroundWidth;
	if (_backgroundOffset.y < 0)
		_backgroundOffset.y += _backgroundHeight;
	if (_backgroundOffset.y > _backgroundHeight)
		_backgroundOffset.y -= _backgroundHeight;

	_currentBackground->seek(0);
	// Purposely make the subRectangle empty. renderImageToScreen will then set the width and height automatically.
	renderImageToScreen(*_currentBackground, 0, 0, Common::Rect(_backgroundOffset.x, _backgroundOffset.y, _backgroundOffset.x, _backgroundOffset.y), true);
}

} // End of namespace ZVision
