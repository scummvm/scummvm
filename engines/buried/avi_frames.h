/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_AVI_FRAMES_H
#define BURIED_AVI_FRAMES_H

#include "common/list.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace Video {
class VideoDecoder;
}

namespace Buried {

class AVIFrames {
public:
	AVIFrames(const Common::String &fileName = "", uint cachedFrames = 0);
	~AVIFrames();

	bool open(const Common::String &fileName, uint cachedFrames = 0);
	void close();

	const Graphics::Surface *getFrame(int frameIndex);
	Graphics::Surface *getFrameCopy(int frameIndex);

	int getFrameCount();

	void enableFrameCache(bool enable) { _cacheEnabled = enable; }
	bool flushFrameCache();

private:
	const Graphics::Surface *retrieveFrameFromCache(int frameIndex) const;
	void addFrameToCache(int frameIndex, Graphics::Surface *frame);

	struct CachedFrame {
		CachedFrame(int i, Graphics::Surface *f) : index(i), frame(f) {}

		int index;
		Graphics::Surface *frame;
	};

	typedef Common::List<CachedFrame> FrameList;

	Common::String _fileName;
	FrameList _cachedFrames;
	uint _maxCachedFrames;
	Video::VideoDecoder *_video;
	bool _cacheEnabled;

	const Graphics::Surface *_lastFrame;
	Graphics::Surface *_tempFrame;
	int _lastFrameIndex;
};

} // End of namespace Buried

#endif
