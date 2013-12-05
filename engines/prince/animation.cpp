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

#include "prince/animation.h"
#include "prince/detail/animation.h"


namespace Prince {

Animation::Animation() : _helper(NULL) {
}

Animation::~Animation() {
	delete _helper;
}

bool Animation::loadFromStream(Common::SeekableReadStream &stream) {

	uint32 dataSize = stream.size();

	byte *data = (byte*)malloc(dataSize);

	if(stream.read(data, dataSize) != dataSize) {
		free(data);
		return false;
	}

	delete _helper;

	_helper = new Detail::Animation(data, dataSize);

	return true;
}

const Graphics::Surface * Animation::getSurface(uint16 frameIndex) {
	// bida kaszing
	if (frameIndex >= _frameList.size()) {
		_frameList.resize(frameIndex);
		_frameList.push_back(_helper->getFrame(frameIndex));
		
	}
	return _frameList[frameIndex];	
}

}


/* vim: set tabstop=4 noexpandtab: */
