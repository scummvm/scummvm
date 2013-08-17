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

RenderManager::RenderManager(OSystem *system, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat)
		: _system(system),
		  _workingWidth(workingWindow.width()),
		  _workingHeight(workingWindow.height()),
		  _screenCenterX((workingWindow.left + workingWindow.right) /2),
		  _screenCenterY((workingWindow.top + workingWindow.bottom) /2),
		  _workingWindow(workingWindow),
		  _pixelFormat(pixelFormat),
		  _currentBackground(0),
		  _backgroundWidth(0),
		  _backgroundHeight(0),
		  _backgroundInverseVelocity(0),
		  _backgroundOffset(0, 0),
		  _accumulatedVelocityMilliseconds(0),
		  _renderTable(_workingWidth, _workingHeight) {

	_warpedBuffer = new uint16[_workingWidth *_workingHeight];
}

RenderManager::~RenderManager() {
	if (_currentBackground != 0) {
		delete _currentBackground;
	}

	delete[] _warpedBuffer;
}

void RenderManager::update(uint deltaTimeInMillis) {
	// An inverse velocity of 0 would be infinitely fast, so we'll let 0 mean no velocity.
	if (_backgroundInverseVelocity != 0) {
		_accumulatedVelocityMilliseconds += deltaTimeInMillis;

		int absVelocity = abs(_backgroundInverseVelocity);

		int numberOfSteps = 0;
		while (_accumulatedVelocityMilliseconds >= absVelocity) {
			_accumulatedVelocityMilliseconds -= absVelocity;
			numberOfSteps++;
		}

		// Choose the direction of movement using the sign of the velocity
		moveBackground(_backgroundInverseVelocity < 0 ? numberOfSteps : -numberOfSteps);
	}
}

void RenderManager::renderSubRectToScreen(Graphics::Surface &surface, int16 destinationX, int16 destinationY, bool wrap, bool isTransposed) {	
	int16 subRectX = 0;
	int16 subRectY = 0;

	// Take care of negative destinations
	if (destinationX < 0) {
		subRectX = destinationX + surface.w;
		destinationX = 0;
		if (wrap) {
			_backgroundOffset.x += surface.w;
		}
	} else if (destinationX >= surface.w) {
		// Take care of extreme positive destinations
		destinationX -= surface.w;
		if (wrap) {
			_backgroundOffset.x -= surface.w;
		}
	}

	// Take care of negative destinations
	if (destinationY < 0) {
		subRectY = destinationY + surface.h;
		destinationY = 0;
		if (wrap) {
			_backgroundOffset.y += surface.h;
		}
	} else if (destinationY >= surface.h) {
		// Take care of extreme positive destinations
		destinationY -= surface.h;
		if (wrap) {
			_backgroundOffset.y -= surface.h;
		}
	}

	if (wrap) {
		_backgroundWidth = surface.w;
		_backgroundHeight = surface.h;
	
		if (destinationX > 0) { 
			// Move destinationX to 0
			subRectX = surface.w - destinationX;
			destinationX = 0;
		}

		if (destinationY > 0) {
			// Move destinationY to 0
			subRectX = surface.w - destinationX;
			destinationY = 0;
		}
	}

	// Clip subRect to working window bounds
	Common::Rect subRect(subRectX, subRectY, subRectX + _workingWidth, subRectY + _workingHeight);

	if (!wrap) {
		// Clip to image bounds
		subRect.clip(surface.w, surface.h);
	}

	// Check destRect for validity
	if (!subRect.isValidRect() || subRect.isEmpty())
		return;

	if (_renderTable.getRenderState() == RenderTable::FLAT) {
		_system->copyRectToScreen(surface.getBasePtr(subRect.left, subRect.top), surface.pitch, destinationX + _workingWindow.left, destinationY + _workingWindow.top, subRect.width(), subRect.height());
	} else {
		_renderTable.mutateImage((uint16 *)surface.getBasePtr(0, 0), _warpedBuffer, surface.w, surface.h, destinationX, destinationY, subRect, wrap, isTransposed);

		_system->copyRectToScreen(_warpedBuffer, _workingWidth * sizeof(uint16), destinationX + _workingWindow.left, destinationY + _workingWindow.top, subRect.width(), subRect.height());
	}
}

void RenderManager::renderImageToScreen(const Common::String &fileName, uint32 destinationX, uint32 destinationY, bool wrap) {
	Common::File file;

	if (!file.open(fileName)) {
		warning("Could not open file %s", fileName.c_str());
		return;
	}

	renderImageToScreen(file, destinationX, destinationY);
}

void RenderManager::renderImageToScreen(Common::SeekableReadStream &stream, uint32 destinationX, uint32 destinationY, bool wrap) {
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

		uint32 pitch = imageWidth * sizeof(uint16);
		bool isTransposed = _renderTable.getRenderState() == RenderTable::PANORAMA;

		if (isTransposed) {
			uint16 temp = imageHeight;
			imageHeight = imageWidth;
			imageWidth = temp;
		}

		Graphics::Surface surface;
		surface.init(imageWidth, imageHeight, pitch, buffer, _pixelFormat);

		renderSubRectToScreen(surface, destinationX, destinationY, wrap, isTransposed);

		// We have to use delete[] instead of calling surface.free() because we created the memory with new[]
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

		Graphics::Surface tgaSurface = *(tga.getSurface());
		bool isTransposed = _renderTable.getRenderState() == RenderTable::PANORAMA;

		if (isTransposed) {
			uint16 temp = tgaSurface.h;
			tgaSurface.h = tgaSurface.w;
			tgaSurface.w = temp;
		}

		renderSubRectToScreen(tgaSurface, destinationX, destinationY, wrap, isTransposed);
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

	renderImageToScreen(*_currentBackground, _backgroundOffset.x, _backgroundOffset.y, true);
}

void RenderManager::setBackgroundPosition(int offset) {
	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::TILT) {
		_backgroundOffset.x = 0;
		_backgroundOffset.y = _screenCenterY - offset;
	} else if (state == RenderTable::PANORAMA) {
		_backgroundOffset.x = _screenCenterX - offset;
		_backgroundOffset.y = 0;
	} else {
		_backgroundOffset.x = 0;
		_backgroundOffset.y = 0;
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
	_currentBackground->seek(0);

	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::TILT) {
		_backgroundOffset += Common::Point(0, offset);

		renderImageToScreen(*_currentBackground, 0, _backgroundOffset.y, true);
	} else if (state == RenderTable::PANORAMA) {
		_backgroundOffset += Common::Point(offset, 0);

		renderImageToScreen(*_currentBackground, _backgroundOffset.x, 0, true);
	} else {
		renderImageToScreen(*_currentBackground, 0, 0);
	}
}

} // End of namespace ZVision
