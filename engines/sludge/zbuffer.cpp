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

#include "image/png.h"

#include "sludge/fileset.h"
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/zbuffer.h"

#include "graphics/managed_surface.h"

namespace Sludge {

void GraphicsManager::killZBuffer() {

	if (_zBuffer->tex) {
		delete[] _zBuffer->tex;
		_zBuffer->tex = nullptr;
	}

	_zBuffer->numPanels = 0;
	_zBuffer->originalNum = -1;
}

void GraphicsManager::sortZPal(int *oldpal, int *newpal, int size) {
	int i, tmp;

	for (i = 0; i < size; i++) {
		newpal[i] = i;
	}

	if (size < 2)
		return;

	for (i = 1; i < size; i++) {
		if (oldpal[newpal[i]] < oldpal[newpal[i - 1]]) {
			tmp = newpal[i];
			newpal[i] = newpal[i - 1];
			newpal[i - 1] = tmp;
			i = 0;
		}
	}
}

bool GraphicsManager::setZBuffer(int num) {
	// if the backdrop has not been set yet
	// set zbuffer later
	if (!_backdropSurface.getPixels()) {
		_zBuffer->originalNum = num;
		return true;
	}

	debug (kSludgeDebugGraphics, "Setting zBuffer");
	uint32 stillToGo = 0;
	int yPalette[16], sorted[16], sortback[16];

	killZBuffer();

	setResourceForFatal(num);

	_zBuffer->originalNum = num;
	uint fsize = g_sludge->_resMan->openFileFromNum(num);
	if (!fsize)
		return false;

	Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();

	g_sludge->_resMan->dumpFile(num, "zbuffer%04d.zbu");

	if (readStream->readByte() != 'S')
		return fatal("Not a Z-buffer file");
	if (readStream->readByte() != 'z')
		return fatal("Not a Z-buffer file");
	if (readStream->readByte() != 'b')
		return fatal("Not a Z-buffer file");

	switch (readStream->readByte()) {
		case 0:
			_zBuffer->width = 640;
			_zBuffer->height = 480;
			break;

		case 1:
			_zBuffer->width = readStream->readUint16BE();
			_zBuffer->height = readStream->readUint16BE();
			break;

		default:
			return fatal("Extended Z-buffer format not supported in this version of the SLUDGE engine");
	}
	if (_zBuffer->width != _sceneWidth || _zBuffer->height != _sceneHeight) {
		Common::String tmp = Common::String::format("Z-w: %d Z-h:%d w: %d, h:%d", _zBuffer->width, _zBuffer->height, _sceneWidth, _sceneHeight);
		return fatal("Z-buffer width and height don't match scene width and height", tmp);
	}

	_zBuffer->numPanels = readStream->readByte();
	debugC(2, kSludgeDebugZBuffer, "Loading zBuffer : %i panels", _zBuffer->numPanels);
	for (int y = 0; y < _zBuffer->numPanels; y++) {
		yPalette[y] = readStream->readUint16BE();
	}
	sortZPal(yPalette, sorted, _zBuffer->numPanels);
	for (int y = 0; y < _zBuffer->numPanels; y++) {
		_zBuffer->panel[y] = yPalette[sorted[y]];
		sortback[sorted[y]] = y;
		debugC(2, kSludgeDebugZBuffer, "Y-value : %i", _zBuffer->panel[y]);
	}

	int picWidth = _sceneWidth;
	int picHeight = _sceneHeight;

	_zBuffer->tex = nullptr;
	_zBuffer->tex = new uint8[picHeight * picWidth];

	int n = 0;

	for (uint y = 0; y < _sceneHeight; y++) {
		for (uint x = 0; x < _sceneWidth; x++) {
			if (stillToGo == 0) {
				n = readStream->readByte();
				stillToGo = n >> 4;
				if (stillToGo == 15)
					stillToGo = readStream->readUint16BE() + 16l;
				else
					stillToGo++;
				n &= 15;
			}

			_zBuffer->tex[y*picWidth + x] = sortback[n];

			stillToGo--;
		}
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);

	return true;
}

void GraphicsManager::fillZBuffer(uint8 d) {
	memset(_zBufferSurface, d, _winHeight * _winWidth);
}

void GraphicsManager::drawSpriteToZBuffer(int x, int y, uint8 depth, const Graphics::Surface &surface) {

	for (uint y1 = 0; y1 < (uint)surface.h; y1++) {
		for (uint x1 = 0; x1 < (uint)surface.w; x1++) {
			if (x1 + x >= _sceneWidth || y1 + y >= _sceneHeight) {
				continue;
			}

			byte *target = (byte *)_renderSurface.getBasePtr(x1 + x, y1 + y);
			const byte *source = (const byte *)surface.getBasePtr(x1, y1);

			if (depth > _zBufferSurface[(y1 + y) * _winWidth + (x1 + x)]) {

				if (source[0] == 0xff) {
					// Completely opaque, so copy RGB values over
					target[0] = 0xff;
					target[1] = source[1];
					target[2] = source[2];
					target[3] = source[3];

				}
			}
		}
	}
}

void GraphicsManager::drawZBuffer(int x, int y, bool upsidedown) {
	if (!_zBuffer->numPanels || !_zBuffer->tex)
		return;

	fillZBuffer(0);

	int w = MIN<uint>(_zBuffer->width, _winWidth + x);
	int h = MIN<uint>(_zBuffer->height, _winHeight + y);

	for (int y1 = y; y1 < h; y1++) {
		for (int x1 = x; x1 < w; x1++) {
			uint8 z = 0;

			if (upsidedown) {
				z = (_zBuffer->tex[(_zBuffer->height - y1) * _zBuffer->width + x1] + 1) * 2;
			} else {
				z = (_zBuffer->tex[y1 * _zBuffer->width + x1] + 1) * 2;
			}

			if ( z > _zBufferSurface[(y1 - y) * _winWidth + (x1 - x)])
				_zBufferSurface[(y1 - y) * _winWidth + (x1 - x)] = z;
		}
	}
}

void GraphicsManager::saveZBuffer(Common::WriteStream *stream) {
	if (_zBuffer->numPanels > 0) {
		stream->writeByte(1);
		stream->writeUint16BE(_zBuffer->originalNum);
	} else {
		stream->writeByte(0);
	}
}

bool GraphicsManager::loadZBuffer(Common::SeekableReadStream *stream) {
	if (stream->readByte()) {
		if (!setZBuffer(stream->readUint16BE()))
			return false;
	}
	return true;
}

} // End of namespace Sludge
