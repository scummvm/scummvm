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

#ifndef ZVISION_HOTMOV_CONTROL_H
#define ZVISION_HOTMOV_CONTROL_H

#include "zvision/scripting/control.h"

#include "common/array.h"
#include "common/rect.h"

namespace Video {
	class VideoDecoder;
}

namespace ZVision {

// Only used in Zork Nemesis, handles movies where the player needs to click on something (mj7g, vw3g)
class HotMovControl : public Control {
public:
	HotMovControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~HotMovControl() override;

private:
	int32  _framesCount;
	int32  _cycle;
	int32  _cyclesCount;
	Video::VideoDecoder *_animation;
	Common::Rect _rectangle;
	Common::Array<Common::Rect> _frames;
public:
	bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool process(uint32 deltaTimeInMillis) override;

private:
	void readHsFile(const Common::String &fileName);
};

} // End of namespace ZVision

#endif
