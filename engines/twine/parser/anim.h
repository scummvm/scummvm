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

#ifndef TWINE_PARSER_ANIM_H
#define TWINE_PARSER_ANIM_H

#include "common/array.h"
#include "common/stream.h"
#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

struct BoneFrame {
	/**
	 * 0 = allow global rotate
	 * 1 = disallow global rotate
	 * 2 = disallow global rotate and hide
	 */
	uint16 type = 0;
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;
};

struct KeyFrame {
	uint16 length = 0;
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;
	Common::Array<BoneFrame> boneframes;
};

class AnimData : public Parser {
private:
	Common::Array<KeyFrame> _keyframes;

	bool loadBoneFrame(KeyFrame &keyframe, Common::SeekableReadStream &stream);
	void loadKeyFrames(Common::SeekableReadStream &stream);

	uint16 _numKeyframes;
	uint16 _numBoneframes;
	uint16 _loopFrame;

protected:
	void reset() override;

public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	const KeyFrame* getKeyframe(uint index) const;
	const Common::Array<KeyFrame>& getKeyframes() const;
	uint getNumKeyframes() const;
	uint16 getLoopFrame() const;
	uint16 getNumBoneframes() const;
};

inline uint AnimData::getNumKeyframes() const {
	return getKeyframes().size();
}

} // End of namespace TwinE

#endif
