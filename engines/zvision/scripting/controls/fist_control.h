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

#ifndef ZVISION_FIST_CONTROL_H
#define ZVISION_FIST_CONTROL_H

#include "zvision/scripting/control.h"

#include "common/array.h"
#include "common/rect.h"

namespace Video {
	class VideoDecoder;
}

namespace ZVision {

// Only used in Zork Nemesis, handles the door lock puzzle with the skeletal fingers (td9e)
class FistControl : public Control {
public:
	FistControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~FistControl() override;

private:
	uint32 _fiststatus;
	int   _fistnum;
	int16  _cursor;
	int   _order;

	Common::Array< Common::Array<Common::Rect> > _fistsUp;
	Common::Array< Common::Array<Common::Rect> > _fistsDwn;

	int32 _numEntries;

	struct entries {
		uint32 _bitsStrt;
		uint32 _bitsEnd;
		int32 _anmStrt;
		int32 _anmEnd;
		int32 _sound;
	};

	Common::Array<entries> _entries;

	Video::VideoDecoder *_animation;
	Common::Rect _anmRect;
	int32   _soundKey;
	int32   _animationId;

public:
	bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool process(uint32 deltaTimeInMillis) override;

private:
	void readDescFile(const Common::String &fileName);
	void clearFistArray(Common::Array< Common::Array<Common::Rect> > &arr);
	uint32 readBits(const char *str);
	int mouseIn(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	void getFistParams(const Common::String &inputStr, Common::String &parameter, Common::String &values);
};

} // End of namespace ZVision

#endif
