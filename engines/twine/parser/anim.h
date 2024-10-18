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

enum BoneType : uint16 {
	TYPE_ROTATE = 0,
	TYPE_TRANSLATE = 1,
	TYPE_ZOOM = 2,
};

struct BoneFrame { // T_GROUP_INFO
	BoneType type = BoneType::TYPE_ROTATE;
	int16 x = 0; // alpha
	int16 y = 0; // beta
	int16 z = 0; // gamma
};
using T_GROUP_INFO = BoneFrame; // (lba2)

struct KeyFrame {
	uint16 length = 0;
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;
	int16 animMasterRot = 0;
	int16 animStepAlpha = 0;
	int16 animStepBeta = 0;
	int16 animStepGamma = 0;
	Common::Array<BoneFrame> boneframes;
};

class AnimData : public Parser {
private:
	Common::Array<KeyFrame> _keyframes;

	void loadBoneFrame(KeyFrame &keyframe, Common::SeekableReadStream &stream);
	void loadKeyFrames(Common::SeekableReadStream &stream);

	uint16 _numKeyframes;
	uint16 _numBoneframes;
	uint16 _loopFrame;

protected:
	void reset() override;

public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	const KeyFrame *getKeyframe(uint index) const;
	const Common::Array<KeyFrame> &getKeyframes() const;
	uint getNbFramesAnim() const;
	uint16 getLoopFrame() const;
	uint16 getNumBoneframes() const;
};

inline uint AnimData::getNbFramesAnim() const {
	return getKeyframes().size();
}

} // End of namespace TwinE

#endif
