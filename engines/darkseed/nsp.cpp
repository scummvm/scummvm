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

#include "darkseed/darkseed.h"
#include "darkseed/nsp.h"
#include "common/debug.h"

namespace Darkseed {

Sprite::Sprite(uint16 width, uint16 height, uint16 pitch) : _width(width), _height(height), _pitch(pitch) {
	_pixels.resize(pitch * height, 0);
}

bool Sprite::loadData(Common::SeekableReadStream &readStream) {
	g_engine->waitForSpeech();
	if (_width == 1 && _height == 1) {
		byte b = readStream.readByte();
		_pixels[0] = b >> 4;
	} else {
		bool hasReadByte = false;
		int currentDataByte = 0;
		for (int i = 0; i < _pitch * _height; i++) {
			if (!hasReadByte) {
				currentDataByte = readStream.readByte();
				if (readStream.eos()) {
					debug("Argh!");
					return false;
				}
				hasReadByte = true;
				_pixels[i] = currentDataByte >> 4;
			} else {
				hasReadByte = false;
				_pixels[i] =  currentDataByte & 0xf;
			}
		}
	}
	return true;
}

void Sprite::clipToScreen(int x, int y, uint16 frameBottom, uint16 *clippedWidth, uint16 *clippedHeight) const {
	*clippedWidth = _width;
	*clippedHeight = _height;
	if (x + _width > g_engine->_screen->w) {
		*clippedWidth = g_engine->_screen->w - x;
	}
	if (frameBottom != 0 && y + _height > g_engine->_frameBottom) {
		if (y >= frameBottom) {
			return;
		}
		*clippedHeight = frameBottom - y;
	}
}

void Sprite::draw(int x, int y, uint16 frameBottom) const {
	uint16 clippedWidth = _width;
	uint16 clippedHeight = _height;
	clipToScreen(x, y, frameBottom, &clippedWidth, &clippedHeight);
	g_engine->_screen->copyRectToSurfaceWithKey(_pixels.data(), _pitch, x, y, clippedWidth, clippedHeight, 0xf);
}

void Sprite::draw(Graphics::Surface *dst, int x, int y, uint16 frameBottom) const {
	uint16 clippedWidth = _width;
	uint16 clippedHeight = _height;
	clipToScreen(x, y, frameBottom, &clippedWidth, &clippedHeight);
	dst->copyRectToSurfaceWithKey(_pixels.data(), _pitch, x, y, clippedWidth, clippedHeight, 0xf);
}

void Sprite::drawScaled(int destX, int destY, int destWidth, int destHeight, bool flipX) const {
	//TODO image isn't exactly the same when not scaling. It seems larger by about a pixel.
	//TODO this logic is pretty messy. It should probably be re-written. It is trying to scale, clip and flip at once.
	Graphics::ManagedSurface * destSurface = g_engine->_screen;
	// Based on the GNAP engine scaling code
	if (destWidth == 0 || destHeight == 0) {
		return;
	}
	const byte *source = _pixels.data();
	const int xs = ((_width - 1) << 16) / destWidth;
	const int ys = ((_height - 1) << 16) / destHeight;
	int clipX = 0, clipY = 0;
	const int destPitch = destSurface->pitch;
	if (destX < 0) {
		clipX = -destX;
		destX = 0;
		destWidth -= clipX;
	}

	if (destY < 0) {
		clipY = -destY;
		destY = 0;
		destHeight -= clipY;
	}
	if (destY + destHeight >= destSurface->h) {
		destHeight = destSurface->h - destY;
	}
	if (destWidth < 0 || destHeight < 0)
		return;
	byte *dst = (byte *)destSurface->getBasePtr(destX, destY);
	int yi = ys * clipY;
	const byte *hsrc = source + _pitch * ((yi + 0x8000) >> 16);
	int16 currY = destY;
	for (int yc = 0; yc < destHeight && currY < g_engine->_frameBottom; ++yc) {
		byte *wdst = flipX ? dst + (destWidth - 1) : dst;
		int16 currX = flipX ? destX + (destWidth - 1) : destX;
		int xi = flipX ? xs : xs * clipX;
		const byte *wsrc = hsrc + ((xi + 0x8000) >> 16);
		for (int xc = 0; xc < destWidth; ++xc) {
			if (currX >= 69 && currX < destSurface->w) { // clip to game window. TODO pass clip rect into method.
				byte colorIndex = *wsrc;
				//				uint16 c = READ_LE_UINT16(&palette[colorIndex * 2]);
				if (colorIndex != 0xf) {
					*wdst = colorIndex;
					//					if (!(c & 0x8000u) || alpha == NONE) {
					//						// only copy opaque pixels
					//						WRITE_SCREEN(wdst, c & ~0x8000);
					//					} else {
					//						WRITE_SCREEN(wdst, alphaBlendRGB555(c & 0x7fffu, READ_SCREEN(wdst) & 0x7fffu, 128));
					//						// semi-transparent pixels.
					//					}
				}
			}
			currX += (flipX ? -1 : 1);
			wdst += (flipX ? -1 : 1);
			xi += xs;
			wsrc = hsrc + ((xi + 0x8000) >> 16);
		}
		dst += destPitch;
		yi += ys;
		hsrc = source + _pitch * ((yi + 0x8000) >> 16);
		currY++;
	}
}

bool Nsp::load(const Common::Path &filename) {
	Common::File file;
	Common::Path filePath = g_engine->getRoomFilePath(filename);
	if (!file.open(filePath)) {
		return false;
	}
	bool ret = load(file);
	file.close();
	if (ret) {
		Common::String filePathStr = filePath.toString();
		debug("Loaded %s", filePathStr.c_str());
		Common::Path obtFilename = Common::Path(filePathStr.substr(0, filePathStr.size() - 4) + ".obt");
		ret = loadObt(obtFilename);
	}
	return ret;
}

bool Nsp::load(Common::SeekableReadStream &readStream) {
	_frames.clear();
	for (int i = 0; i < 96; i++) {
		int w = readStream.readByte();
		int h = readStream.readByte();
		int p = w + (w & 1);
		_frames.push_back(Sprite(w, h, p));
	}

	for (int i = 0; i < 96; i++) {
		if (!_frames[i].loadData(readStream)) {
			return false;
		}
	}

	return true;
}

const Sprite &Nsp::getSpriteAt(int index) const {
	if (index >= (int)_frames.size()) {
		error("getSpriteAt: Invalid sprite index. %d", index);
	}
	return _frames[index];
}

bool Nsp::loadObt(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename)) {
		return false;
	}

	_animations.clear();
	_animations.resize(20);
	for (int i = 0; i < 20; i++) {
		_animations[i]._numFrames = file.readByte();

		for (int j = 0; j < 20; j++) {
			if (file.readByte()) {
				_animations[i]._deltaX.push_back(-(file.readUint16LE() / 100));
			} else {
				_animations[i]._deltaX.push_back(file.readUint16LE() / 100);
			}
			if (file.readByte()) {
				_animations[i]._deltaY.push_back(-(file.readUint16LE() / 100));
			} else {
				_animations[i]._deltaY.push_back(file.readUint16LE() / 100);
			}
			_animations[i]._frameNo.push_back(file.readByte());
			_animations[i]._frameDuration.push_back(file.readByte());
		}
	}

	file.close();

	debug("Loaded %s", filename.toString().c_str());
	return true;
}

const Obt &Nsp::getAnimAt(int index) {
	return _animations[index];
}

int16 Nsp::getMaxSpriteWidth() {
	int maxWidth = 0;
	for (auto &frame : _frames) {
		if (frame._width > maxWidth) {
			maxWidth = frame._width;
		}
	}
	return maxWidth;
}

Obt::Obt() {
	_numFrames = 0;
	_deltaX.reserve(20);
	_deltaY.reserve(20);
	_frameNo.reserve(20);
	_frameDuration.reserve(20);
}

Obt::~Obt() {
	_deltaX.clear();
	_deltaY.clear();
	_frameNo.clear();
	_frameDuration.clear();
}

} // End of namespace Darkseed
