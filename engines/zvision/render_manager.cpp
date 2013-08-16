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
		  _workingWindow(workingWindow),
		  _pixelFormat(pixelFormat),
		  _currentBackground(0),
		  _backgroundWidth(0),
		  _backgroundHeight(0),
		  _backgroundInverseVelocity(0),
		  _accumulatedVelocityMilliseconds(0),
		  _renderTable(workingWindow.width(), workingWindow.height()) {
	_backbuffer.create(_workingWidth, _workingHeight, pixelFormat);
	_warpedBackbuffer = new uint16[_workingWidth *_workingHeight];
}

RenderManager::~RenderManager() {
	if (_currentBackground != 0) {
		delete _currentBackground;
	}

	_backbuffer.free();
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
		moveBackground(_backgroundInverseVelocity < 0 ? -numberOfSteps : numberOfSteps);
	}

	// Warp the entire backbuffer
	_renderTable.mutateImage((uint16 *)_backbuffer.getBasePtr(0, 0), _warpedBackbuffer, _workingWidth, _workingHeight);

	// Blit the backbuffer to the screen
	_system->copyRectToScreen(_backbuffer.getBasePtr(0, 0), _backbuffer.pitch, _workingWindow.left, _workingWindow.top, _backbuffer.w, _backbuffer.h);
}

void RenderManager::renderSubRectToBackbuffer(Graphics::Surface &surface, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool wrap, bool isTransposed) {	
	if (wrap) {
		_backgroundWidth = surface.w;
		_backgroundHeight = surface.h;
	}
	
	// If subRect is empty, use the entire image
	if (subRectangle.isEmpty())
		subRectangle = Common::Rect(subRectangle.left, subRectangle.top, subRectangle.left + surface.w, subRectangle.top + surface.h);

	// Clip destRect to working window bounds
	Common::Rect destRect(destinationX, destinationY, destinationX + subRectangle.width(), destinationY + subRectangle.height());
	destRect.clip(_workingWidth, _workingHeight);
	// Clip subRect to working window bounds
	subRectangle.translate(destRect.left - destinationX, destRect.top - destinationY);
	subRectangle.setWidth(destRect.width());
	subRectangle.setHeight(destRect.height());
	// Clip to image bounds
	Common::Point subRectOrigOrigin(subRectangle.left, subRectangle.top);
	subRectangle.clip(surface.w, surface.h);

	// If the image is to be wrapped, check if it's smaller than destRect
	// If it is, then call renderSubRectToScreen with a subRect representing wrapping
	if (wrap && subRectangle.width() < destRect.width()) {
		uint32 wrapDestX;
		uint32 wrapDestY; 
		Common::Rect wrapSubRect;

		if (_backgroundWidth - subRectangle.left < destRect.width()) {
			wrapDestX = destRect.left + subRectangle.width();
			wrapDestY = destRect.top;
			wrapSubRect = Common::Rect(0, 0, destRect.width() - subRectangle.width(), subRectangle.bottom);
		} else {
			wrapDestX = destRect.left;
			wrapDestY = destRect.top;
			wrapSubRect = Common::Rect(_backgroundWidth - subRectangle.width(), 0, _backgroundWidth - 1, subRectangle.bottom);
		}

		renderSubRectToBackbuffer(surface, wrapDestX, wrapDestY, wrapSubRect, false, isTransposed);
	} else if (wrap && subRectangle.height() < destRect.height()) {
		uint32 wrapDestX;
		uint32 wrapDestY; 
		Common::Rect wrapSubRect;

		if (_backgroundHeight - subRectangle.top < destRect.height()) {
			wrapDestX = destRect.left;
			wrapDestY = destRect.height() - subRectangle.height();
			wrapSubRect = Common::Rect(0, 0, subRectangle.right, destRect.height() - subRectangle.height());
		} else {
			wrapDestX = destRect.left;
			wrapDestY = destRect.top;
			wrapSubRect = Common::Rect(0, _backgroundHeight - subRectangle.height(), subRectangle.right, _backgroundHeight - 1);
		}

		renderSubRectToBackbuffer(surface, wrapDestX, wrapDestY, wrapSubRect, false, isTransposed);
	} else {
		// Clip destRect to image bounds
		destRect.translate(subRectangle.left - subRectOrigOrigin.x, subRectangle.top - subRectOrigOrigin.y);
		destRect.setWidth(subRectangle.width());
		destRect.setHeight(subRectangle.height());
	}

	// Check all Rects for validity
	if (!subRectangle.isValidRect() || subRectangle.isEmpty() || !destRect.isValidRect() || destRect.isEmpty())
		return;

	if (isTransposed) {
		copyTransposedRectToBackbuffer((uint16 *)surface.getBasePtr(subRectangle.left, subRectangle.right), surface.w, destRect.left, destRect.top, destRect.width(), destRect.height());
	} else {
		_backbuffer.copyRectToSurface(surface.getBasePtr(subRectangle.left, subRectangle.top), surface.pitch, destRect.left, destRect.top, destRect.width(), destRect.height());
	}
}

void RenderManager::copyTransposedRectToBackbuffer(const uint16 *buffer, int imageHeight, int x, int y, int width, int height) {
	uint16 *dest = (uint16 *)_backbuffer.getBasePtr(x, y);

	for (int x = 0; x < width; x++) {
		int columnOffset = x * imageHeight;
		for (int y = 0; y < height; y++) {
			*dest = buffer[columnOffset + y];
			dest++;
		}
	}
}

void RenderManager::renderImageToBackbuffer(const Common::String &fileName, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool wrap) {
	Common::File file;

	if (!file.open(fileName)) {
		warning("Could not open file %s", fileName.c_str());
		return;
	}

	renderImageToBackbuffer(file, destinationX, destinationY, subRectangle);
}

void RenderManager::renderImageToBackbuffer(Common::SeekableReadStream &stream, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool wrap) {
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

		renderSubRectToBackbuffer(surface, destinationX, destinationY, subRectangle, wrap, isTransposed);

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

		renderSubRectToBackbuffer(tgaSurface, destinationX, destinationY, subRectangle, wrap, isTransposed);
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

	// Purposely make the subRectangle empty. renderImageToScreen will then set the width and height automatically.
	renderImageToBackbuffer(*_currentBackground, 0, 0, Common::Rect(_backgroundOffset.x, _backgroundOffset.y, _backgroundOffset.x, _backgroundOffset.y), true);
}

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
	renderImageToBackbuffer(*_currentBackground, 0, 0, Common::Rect(_backgroundOffset.x, _backgroundOffset.y, _backgroundOffset.x, _backgroundOffset.y), true);
}

} // End of namespace ZVision
