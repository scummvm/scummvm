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

#ifndef ZVISION_METAANIM_NODE_H
#define ZVISION_METAANIM_NODE_H

#include "zvision/scripting/sidefx.h"
#include "zvision/zvision.h"
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
class RLFDecoder;

class MetaAnimation {
public:
	MetaAnimation(const Common::String &fileName, ZVision *engine);
	~MetaAnimation();

	struct playnode {
		Common::Rect pos;
		int32 slot;
		int32 start;
		int32 stop;
		int32 loop;
		int32 _curFrame;
		int32 _delay;
		Graphics::Surface *_scaled;
	};

private:
	enum FileType {
		RLF = 1,
		AVI = 2
	};

private:
	union {
		RLFDecoder *rlf;
		Video::VideoDecoder *avi;
	} _animation;

	FileType _fileType;
	int32 _frmDelay;

	const Graphics::Surface *_curFrame;

public:

	uint frameCount();
	uint width();
	uint height();
	uint32 frameTime();

	void seekToFrame(int frameNumber);

	const Graphics::Surface *decodeNextFrame();
	const Graphics::Surface *getFrameData(uint frameNumber);

	bool endOfAnimation();
};

} // End of namespace ZVision

#endif
