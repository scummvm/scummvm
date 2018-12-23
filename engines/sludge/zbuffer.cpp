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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "graphics/pixelformat.h"
#include "graphics/transparent_surface.h"

#include "sludge/allfiles.h"
#include "sludge/fileset.h"
#include "sludge/graphics.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sprites.h"
#include "sludge/zbuffer.h"

namespace Sludge {

void GraphicsManager::killZBuffer() {
	if (_zBuffer->sprites) {
		for (int i = 0; i < _zBuffer->numPanels; ++i) {
			_zBuffer->sprites[i].free();
		}
		delete []_zBuffer->sprites;
		_zBuffer->sprites = nullptr;
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
	int yPalette[16], sorted[16];

	killZBuffer();

	setResourceForFatal(num);

	_zBuffer->originalNum = num;
	if (!g_sludge->_resMan->openFileFromNum(num))
		return false;
	Common::ReadStream *readStream = g_sludge->_resMan->getData();
	if (readStream->readByte() != 'S')
		return fatal("Not a Z-buffer file");
	if (readStream->readByte() != 'z')
		return fatal("Not a Z-buffer file");
	if (readStream->readByte() != 'b')
		return fatal("Not a Z-buffer file");

	uint width, height;
	switch (readStream->readByte()) {
		case 0:
			width = 640;
			height = 480;
			break;

		case 1:
			width = readStream->readUint16BE();
			height = readStream->readUint16BE();
			break;

		default:
			return fatal("Extended Z-buffer format not supported in this version of the SLUDGE engine");
	}
	if (width != _sceneWidth || height != _sceneHeight) {
		Common::String tmp = Common::String::format("Z-w: %d Z-h:%d w: %d, h:%d", width, height, _sceneWidth, _sceneHeight);
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
		debugC(2, kSludgeDebugZBuffer, "Y-value : %i", _zBuffer->panel[y]);
	}

	int picWidth = _sceneWidth;
	int picHeight = _sceneHeight;

	_zBuffer->sprites = nullptr;
	_zBuffer->sprites = new Graphics::Surface[_zBuffer->numPanels];

	for (int i = 0; i < _zBuffer->numPanels; ++i) {
		_zBuffer->sprites[i].create(picWidth, picHeight, *g_sludge->getScreenPixelFormat());
	}

	for (uint y = 0; y < _sceneHeight; y++) {
		for (uint x = 0; x < _sceneWidth; x++) {
			int n = 0;
			if (stillToGo == 0) {
				n = readStream->readByte();
				stillToGo = n >> 4;
				if (stillToGo == 15)
					stillToGo = readStream->readUint16BE() + 16l;
				else
					stillToGo++;
				n &= 15;
			}
			for (int i = 0; i < _zBuffer->numPanels; ++i) {
				byte *target = (byte *)_zBuffer->sprites[i].getBasePtr(x, y);
				if (n && (sorted[i] == n || i == 0)) {
					byte *source = (byte *)_backdropSurface.getBasePtr(x, y);
					target[0] = source[0];
					target[1] = source[1];
					target[2] = source[2];
					target[3] = source[3];
				} else {
					target[0] = 0;
					target[1] = 0;
					target[2] = 0;
					target[3] = 0;
				}
			}
			stillToGo--;
		}
	}
	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);
	return true;
}

void GraphicsManager::drawZBuffer(int x, int y, bool upsidedown) {
	if (!_zBuffer->numPanels || !_zBuffer->sprites)
		return;

	g_sludge->_gfxMan->resetSpriteLayers(_zBuffer, x, y, upsidedown);
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
