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

#ifndef ZVISION_ANIMATION_NODE_H
#define ZVISION_ANIMATION_NODE_H

#include "zvision/sidefx.h"
#include "common/rect.h"
#include "common/list.h"


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

class AnimationNode : public SideFX {
public:
	AnimationNode(ZVision *engine, uint32 controlKey, const Common::String &fileName, int32 mask, int32 frate, bool DisposeAfterUse = true);
	~AnimationNode();

	struct playnode {
		Common::Rect pos;
		int32 slot;
		int32 start;
		int32 stop;
		int32 loop;
		int32 _cur_frm;
		int32 _delay;
	};

private:
	enum FileType {
	    RLF = 1,
	    AVI = 2
	};

private:
	typedef Common::List<playnode> PlayNodes;

	PlayNodes _playList;

	union {
		RlfAnimation *rlf;
		Video::VideoDecoder *avi;
	} _animation;

	FileType _fileType;
	int32 _frmDelay;
	int32 _mask;
	bool _DisposeAfterUse;

public:
	bool process(uint32 deltaTimeInMillis);

	void addPlayNode(int32 slot, int x, int y, int w, int h, int start_frame, int end_frame, int loops = 1);

	bool stop();
};

} // End of namespace ZVision

#endif
