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

Darkseed::Sprite::Sprite(uint16 width, uint16 height) : width(width), height(height) {
	pixels.resize(width * height, 0);
}

bool Darkseed::Sprite::loadData(Common::SeekableReadStream &readStream) {
	if (width == 1 && height == 1) {
		byte b = readStream.readByte();
		pixels[0] = b >> 4;
	} else {
		bool hasReadByte = false;
		int currentDataByte = 0;
		for (int i = 0; i < width * height; i++) {
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
	g_engine->_screen->copyRectToSurfaceWithKey(pixels.data(), width, x, y, width, height, 0xf);
}

bool Darkseed::Nsp::load(const Common::String &filename) {
	Common::File file;
	if(!file.open(filename)) {
		return false;
	}
	bool ret = load(file);
	file.close();
	if (ret) {
		debug("Loaded %s", filename.c_str());
		Common::String obtFilename = filename.substr(0, filename.size() - 4) + ".obt";
		ret = loadObt(obtFilename);
	}
	return ret;
}

bool Darkseed::Nsp::load(Common::SeekableReadStream &readStream) {
	frames.clear();
	for (int i = 0; i < 96; i++) {
		int w = readStream.readByte();
		int h = readStream.readByte();
		w = w + (w & 1);
		frames.push_back(Sprite(w, h));
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

	animations.resize(20);
	for (int i = 0; i < 20; i++) {
		animations[i].numFrames = file.readByte();

		for (int j = 0; j < 20; j++) {
			file.readByte();
			int msb = file.readUint16BE();
			animations[i].deltaX.push_back(msb);
			file.readByte();
			msb = file.readUint16BE();
			animations[i].deltaY.push_back(msb);
			animations[i].frameNo.push_back(file.readByte());
			animations[i].frameDuration.push_back(file.readByte());
		}
	}

	file.close();

	debug("Loaded %s", filename.c_str());
	return true;
}

Darkseed::Obt::Obt() {
	numFrames = 0;
	deltaX.reserve(20);
	deltaY.reserve(20);
	frameNo.reserve(20);
	frameDuration.reserve(20);
}
