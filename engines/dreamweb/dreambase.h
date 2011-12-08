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

/**
 * This class is one of the parent classes of DreamGenContext. Its sole purpose
 * is to allow us to incrementally move things out of DreamGenContext into this
 * base class, as soon as they don't modify any context registers (ax, bx, cx, ...)
 * anymore.
 * Ultimately, DreamGenContext should be empty, at which point it can be removed
 * together with class Context. When that happens, we can probably merge
 * DreamBase into DreamWebEngine.
 */
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
	// from pathfind.cpp
	Common::Point _lineData[200];		// Output of Bresenham
	void checkDest(const RoomPaths *roomsPaths);

	// from print.cpp
	uint8 getNextWord(const Frame *charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount);
	uint8 getNumber(const Frame *charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16 *offset);
	uint8 kernChars(uint8 firstChar, uint8 secondChar, uint8 width);

	// from sound.cpp
	void volumeAdjust();
	void cancelCh0();
	void cancelCh1();
	void loadRoomsSample();

	// from stubs.cpp
	void crosshair();
	void showBlink();
	void dumpBlink();
	void dumpPointer();
	void showRyanPage();

	// from vgagrafx.cpp
	uint8 _workspace[(0x1000 + 2) * 16];
	inline uint8 *workspace() { return _workspace; }
	void clearWork();

	void multiGet(uint8 *dst, uint16 x, uint16 y, uint8 width, uint8 height);
	void multiPut(const uint8 *src, uint16 x, uint16 y, uint8 width, uint8 height);
	void multiDump(uint16 x, uint16 y, uint8 width, uint8 height);
	void workToScreenCPP();
	void printUnderMon();
	void cls();
	void frameOutV(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, int16 x, int16 y);
	void frameOutNm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutBh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutFx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void doShake();
	void showPCX(const Common::String &name);
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height);
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag);
	void createPanel();
	void createPanel2();
	void showPanel();
};


} // End of namespace DreamGen

#endif
