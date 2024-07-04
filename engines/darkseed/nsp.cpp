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

#include "nsp.h"
#include "common/debug.h"
#include "darkseed.h"

Darkseed::Sprite::Sprite(uint16 width, uint16 height, uint16 pitch) : width(width), height(height), pitch(pitch) {
	pixels.resize(pitch * height, 0);
}

bool Darkseed::Sprite::loadData(Common::SeekableReadStream &readStream) {
	if (width == 1 && height == 1) {
		byte b = readStream.readByte();
		pixels[0] = b >> 4;
	} else {
		bool hasReadByte = false;
		int currentDataByte = 0;
		for (int i = 0; i < pitch * height; i++) {
			if (!hasReadByte) {
				currentDataByte = readStream.readByte();
				if (readStream.eos()) {
					debug("Argh!");
					return false;
				}
				hasReadByte = true;
				 pixels[i] = currentDataByte >> 4;
			} else {
				hasReadByte = false;
				pixels[i] =  currentDataByte & 0xf;
			}
		}
	}
	return true;
}

void Darkseed::Sprite::draw(int x, int y) const {
	g_engine->_screen->copyRectToSurfaceWithKey(pixels.data(), pitch, x, y, width, height, 0xf);
}

void Darkseed::Sprite::drawScaled(int destX, int destY, int destWidth, int destHeight, bool flipX) const {
	//TODO this logic is pretty messy. It should probably be re-written. It is trying to scale, clip and flip at once.
	Graphics::ManagedSurface * destSurface = g_engine->_screen;
	// Based on the GNAP engine scaling code
	if (destWidth == 0 || destHeight == 0) {
		return;
	}
	const byte *source = pixels.data();
	const int xs = ((width - 1) << 16) / destWidth;
	const int ys = ((height -1) << 16) / destHeight;
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
	const byte *hsrc = source + pitch * ((yi + 0x8000) >> 16);
	for (int yc = 0; yc < destHeight; ++yc) {
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
		hsrc = source + pitch * ((yi + 0x8000) >> 16);
	}
}

bool Darkseed::Nsp::load(const Common::String &filename) {
	Common::File file;
	Common::String filePath = g_engine->getRoomFilePath(filename);
	if(!file.open(filePath)) {
		return false;
	}
	bool ret = load(file);
	file.close();
	if (ret) {
		debug("Loaded %s", filePath.c_str());
		Common::String obtFilename = filePath.substr(0, filePath.size() - 4) + ".obt";
		ret = loadObt(obtFilename);
	}
	return ret;
}

bool Darkseed::Nsp::load(Common::SeekableReadStream &readStream) {
	frames.clear();
	for (int i = 0; i < 96; i++) {
		int w = readStream.readByte();
		int h = readStream.readByte();
		int p = w + (w & 1);
		frames.push_back(Sprite(w, h, p));
	}

	for (int i = 0; i < 96; i++) {
		if(!frames[i].loadData(readStream)) {
			return false;
		}
	}

	return true;
}

const Darkseed::Sprite &Darkseed::Nsp::getSpriteAt(int index) {
	return frames[index];
}

bool Darkseed::Nsp::loadObt(const Common::String &filename) {
	Common::File file;
	if(!file.open(filename)) {
		return false;
	}

	animations.clear();
	animations.resize(20);
	for (int i = 0; i < 20; i++) {
		animations[i].numFrames = file.readByte();

		for (int j = 0; j < 20; j++) {
			if (file.readByte()) {
				animations[i].deltaX.push_back(-(file.readUint16LE()/100));
			} else {
				animations[i].deltaX.push_back(file.readUint16LE()/100);
			}
			if (file.readByte()) {
				animations[i].deltaY.push_back(-(file.readUint16LE()/100));
			} else {
				animations[i].deltaY.push_back(file.readUint16LE()/100);
			}
			animations[i].frameNo.push_back(file.readByte());
			animations[i].frameDuration.push_back(file.readByte());
		}
	}

	file.close();

	debug("Loaded %s", filename.c_str());
	return true;
}

const Darkseed::Obt &Darkseed::Nsp::getAnimAt(int index) {
	return animations[index];
}

Darkseed::Obt::Obt() {
	numFrames = 0;
	deltaX.reserve(20);
	deltaY.reserve(20);
	frameNo.reserve(20);
	frameDuration.reserve(20);
}

Darkseed::Obt::~Obt() {
	deltaX.clear();
	deltaY.clear();
	frameNo.clear();
	frameDuration.clear();
}
