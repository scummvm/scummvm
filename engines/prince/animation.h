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

#ifndef PRINCE_ANIMATION_H
#define PRINCE_ANIMATION_H

#include "common/array.h"
#include "common/stream.h"

#include "graphics/surface.h"

namespace Prince {

class Animation {
public:
	Animation();
	~Animation();
	bool loadStream(Common::SeekableReadStream &stream);

	int16 getLoopCount() const;
	int32 getPhaseCount() const;
	int32 getFrameCount() const;
	int16 getBaseX() const;
	int16 getBaseY() const;
	int16 getPhaseOffsetX(int phaseIndex) const;
	int16 getPhaseOffsetY(int phaseIndex) const;
	int16 getPhaseFrameIndex(int phaseIndex) const;
	Graphics::Surface *getFrame(int frameIndex);
	void clear();

private:
	struct Phase {
		int16 _phaseOffsetX;
		int16 _phaseOffsetY;
		uint16 _phaseToFrameIndex;
	};
	struct Frame {
		bool _isCompressed;
		uint32 _dataSize;
		byte *_compressedData;
		Graphics::Surface *_surface;
	};
	Common::Array<Frame> _frameList;
	Common::Array<Phase> _phaseList;
	int16 _loopCount;
	int16 _phaseCount;
	int32 _frameCount;
	int16 _baseX;
	int16 _baseY;
};

} // End of namespace Prince

#endif
