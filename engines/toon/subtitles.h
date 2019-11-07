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

#ifndef TOON_SUBTITLES_H
#define TOON_SUBTITLES_H

#include "graphics/surface.h"
#include "toon/toon.h"

namespace Toon {

class TimeWindow {
public:
	uint16 _startFrame;
	uint16 _endFrame;
	Common::String _text;
	TimeWindow(int startFrame, int endFrame, const Common::String &text) {
		_startFrame = startFrame;
		_endFrame = endFrame;
		_text = text;
	}
};

class SubtitleRenderer {
public:
	SubtitleRenderer(ToonEngine *vm);
	~SubtitleRenderer();

	bool load(const Common::String &video);
	void render(const Graphics::Surface &frame, uint32 frameNumber, byte color);
protected:
	ToonEngine *_vm;
	Graphics::Surface *_subSurface;
	bool _hasSubtitles;
	Common::List<TimeWindow> _tw;
};

} // End of namespace Toon

#endif
