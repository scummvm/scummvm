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

#ifndef ZVISION_ANIMATION_CONTROL_H
#define ZVISION_ANIMATION_CONTROL_H

#include "zvision/scripting/control.h"


namespace Common {
class String;
}

namespace Video {
class VideoDecoder;
}

namespace Graphics {
struct Surface;
}

namespace ZVision {

class ZVision;
class RlfAnimation;

class AnimationControl : public Control {
public:
	AnimationControl(ZVision *engine, uint32 controlKey, const Common::String &fileName);
	~AnimationControl();

private:
	enum FileType {
		RLF = 1,
		AVI = 2
	};

private:
	uint32 _animationKey;
	
	union {
		RlfAnimation *rlf;
		Video::VideoDecoder *avi;
	} _animation;

	FileType _fileType;
	uint _loopCount;
	int32 _x;
	int32 _y;

	uint _accumulatedTime;
	uint _currentLoop;

	Graphics::Surface *_cachedFrame;
	bool _cachedFrameNeedsDeletion;

public:
	bool process(uint32 deltaTimeInMillis);

	void setAnimationKey(uint32 animationKey) { _animationKey = animationKey; }
	void setLoopCount(uint loopCount) { _loopCount = loopCount; }
	void setXPos(int32 x) { _x = x; }
	void setYPost(int32 y) { _y = y; }
};

} // End of namespace ZVision

#endif
