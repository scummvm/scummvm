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

#ifndef NEVERHOOD_SUBTITLES_H
#define NEVERHOOD_SUBTITLES_H

#include "common/str.h"
#include "neverhood/neverhood.h"
#include "graphics/surface.h"

namespace Neverhood {

struct Subtitle {
	uint32 fromFrame;
	uint32 toFrame;
	Common::String text;
	bool isTop;
};

class SubtitlePlayer {
public:
	SubtitlePlayer(NeverhoodEngine *vm, uint32 fileHash, int width);

	void renderFrame(uint frameNumber, int centerX);
	const Graphics::Surface *getBottomSubs() const { return _haveBottomSubs ? &_bottomSubs : nullptr; }
	const Graphics::Surface *getTopSubs() const { return _haveTopSubs ? &_topSubs : nullptr; }
	bool isValid() const { return _isValid && !_subtitles.empty(); }

	static const byte kSubtitleAlpha = 0x77;
	static const int kSubtitleCharHeight = 16;
	static const int kSubtitleCharWidth = 8;

private:
	NeverhoodEngine *_vm;
	bool _isValid;
	Common::Array<Subtitle> _subtitles;
	Graphics::Surface _bottomSubs;
	Graphics::Surface _topSubs;
	bool _haveBottomSubs;
	bool _haveTopSubs;
	int64 _currentFrame;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SUBTITLES_H */
