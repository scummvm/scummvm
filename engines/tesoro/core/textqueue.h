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

#ifndef TESORO_CORE_TEXTQUEUE_H
#define TESORO_CORE_TEXTQUEUE_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Tesoro {

class AnimBank;

class TextQueue {
public:
	struct Line {
		Common::Array<byte> chars;
		int32 x;
		int32 y;
		uint16 colour;
	};

	enum { kWidth = 640, kHeight = 480 };

	void setFont(const AnimBank *bank) { _font = bank; }

	void clear() { _lines.clear(); }
	uint32 lineCount() const { return _lines.size(); }
	const Line &line(uint32 i) const { return _lines[i]; }

	uint16 add(const byte *text, uint32 colour, int32 centreX, int32 y, int32 maxWidth, uint16 layerDepth, bool centred, int32 lineHeight);

	/** Moves every line from the one numbered first on, down by dy. */
	void shiftLines(uint32 first, int32 dy);

	void draw(uint16 *frame) const;

private:
	const AnimBank *_font = nullptr;
	Common::Array<Line> _lines;
};

} // End of namespace Tesoro

#endif
