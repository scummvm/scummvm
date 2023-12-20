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

#include "common/debug.h"
#include "nsp.h"

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

bool Darkseed::Nsp::load(const Common::String &filename) {
	Common::File file;
	if(!file.open(filename)) {
		return false;
	}
	bool ret = load(file);
	file.close();
	if (ret) {
		debug("Loaded %s", filename.c_str());
	}
	return ret;
}

bool Darkseed::Nsp::load(Common::SeekableReadStream &readStream) {
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
