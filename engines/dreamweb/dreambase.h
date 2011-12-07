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

#ifndef DREAMBASE_H
#define DREAMBASE_H

#include "common/scummsys.h"

#include "dreamweb/runtime.h"

namespace DreamWeb {
	class DreamWebEngine;
}


namespace DreamGen {

class DreamBase {
protected:
	DreamWeb::DreamWebEngine *engine;

public:
	enum { kDefaultDataSegment = 0x1000 };

	SegmentPtr _realData;	///< the primary data segment, points to a huge blob of binary data
	SegmentRef data;	///< fake segment register always pointing to data segment

	DreamBase(DreamWeb::DreamWebEngine *en) :
		engine(en),
		_realData(new Segment()),
		data(kDefaultDataSegment, _realData) {
	}


public:
	// from print.cpp
	uint8 getNextWord(const Frame *charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount);
	uint8 getNumber(const Frame *charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16 *offset);
	uint8 kernChars(uint8 firstChar, uint8 secondChar, uint8 width);

	// from vgagrafx.cpp
	void printUnderMon();
	void cls();
	void frameOutV(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, int16 x, int16 y);
	void frameOutNm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutBh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutFx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void doShake();
	void showPCX(const Common::String &name);

// TODO: Move more methods from stubs.h to here.
};


} // End of namespace DreamGen

#endif
