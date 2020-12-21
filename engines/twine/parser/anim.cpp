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

#include "twine/parser/anim.h"
#include "common/memstream.h"

namespace TwinE {

bool AnimData::loadBoneFrame(KeyFrame &keyframe, Common::SeekableReadStream &stream) {
	BoneFrame boneframe;
	boneframe.type = stream.readSint16LE();
	boneframe.x = stream.readSint16LE();
	boneframe.y = stream.readSint16LE();
	boneframe.z = stream.readSint16LE();
	keyframe.boneframes.push_back(boneframe);
	return boneframe.type != 0;
}

void AnimData::loadKeyFrames(Common::SeekableReadStream &stream) {
	for (uint16 i = 0U; i < _numKeyframes; ++i) {
		KeyFrame keyframe;
		keyframe.length = stream.readUint16LE();
		keyframe.x = stream.readSint16LE();
		keyframe.y = stream.readSint16LE();
		keyframe.z = stream.readSint16LE();

		for (uint16 j = 0U; j < _numBoneframes; ++j) {
			loadBoneFrame(keyframe, stream);
		}

		_keyframes.push_back(keyframe);
		assert(keyframe.boneframes.size() == (uint)_numBoneframes);
	}
}

bool AnimData::loadFromStream(Common::SeekableReadStream &stream) {
	_numKeyframes = stream.readUint16LE();
	_numBoneframes = stream.readUint16LE();
	_loopFrame = stream.readUint16LE();
	stream.readUint16LE();

	loadKeyFrames(stream);

	return !stream.err();
}

const Common::Array<KeyFrame>& AnimData::getKeyframes() const {
	return _keyframes;
}

const KeyFrame* AnimData::getKeyframe(uint index) const {
	if (index >= _numKeyframes) {
		return nullptr;
	}
	return &_keyframes[index];
}

uint16 AnimData::getLoopFrame() const {
	return _loopFrame;
}

uint16 AnimData::getNumBoneframes() const {
	return _numBoneframes;
}

} // namespace TwinE
