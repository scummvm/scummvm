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

#ifndef ZVISION_SAFE_CONTROL_H
#define ZVISION_SAFE_CONTROL_H

#include "zvision/scripting/control.h"

#include "common/list.h"
#include "common/rect.h"

namespace Video {
	class VideoDecoder;
}

namespace ZVision {

// Only used in Zork Nemesis, handles the safe in the Asylum (ac4g)
class SafeControl : public Control {
public:
	SafeControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~SafeControl() override;

private:
	int16  _statesCount;
	int16  _curState;
	Video::VideoDecoder *_animation;
	Common::Point _center;
	Common::Rect _rectangle;
	int16  _innerRaduis;
	int32  _innerRadiusSqr;
	int16  _outerRadius;
	int32  _outerRadiusSqr;
	int16  _zeroPointer;
	int16  _startPointer;
	int16  _targetFrame;

public:
	bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool process(uint32 deltaTimeInMillis) override;
};

} // End of namespace ZVision

#endif
