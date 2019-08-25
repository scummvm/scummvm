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

#ifndef AGI_VIEW_H
#define AGI_VIEW_H

namespace Agi {

struct AgiViewCel {
	uint8 height;
	uint8 width;
	uint8 clearKey;
	bool  mirrored;
	byte *rawBitmap;
};

struct AgiViewLoop {
	int16 celCount;
	AgiViewCel *cel;
};

/**
 * AGI view resource structure.
 */
struct AgiView {
	byte  headerStepSize;
	byte  headerCycleTime;
	byte *description;
	int16 loopCount;
	AgiViewLoop *loop;

	void reset() {
		headerStepSize = 0;
		headerCycleTime = 0;
		description = nullptr;
		loopCount = 0;
		loop = nullptr;
	}

	AgiView() { reset(); }
};

enum MotionType {
	kMotionNormal = 0,
	kMotionWander = 1,
	kMotionFollowEgo = 2,
	kMotionMoveObj = 3,
	kMotionEgo = 4 // used by us for mouse movement only?
};

enum CycleType {
	kCycleNormal = 0,
	kCycleEndOfLoop = 1,
	kCycleRevLoop = 2,
	kCycleReverse = 3
};

enum ViewFlags {
	fDrawn          = (1 << 0),     // 0x0001
	fIgnoreBlocks   = (1 << 1),     // 0x0002
	fFixedPriority  = (1 << 2),     // 0x0004
	fIgnoreHorizon  = (1 << 3),     // 0x0008
	fUpdate         = (1 << 4),     // 0x0010
	fCycling        = (1 << 5),     // 0x0020
	fAnimated       = (1 << 6),     // 0x0040
	fMotion         = (1 << 7),     // 0x0080
	fOnWater        = (1 << 8),     // 0x0100
	fIgnoreObjects  = (1 << 9),     // 0x0200
	fUpdatePos      = (1 << 10),    // 0x0400
	fOnLand         = (1 << 11),    // 0x0800
	fDontupdate     = (1 << 12),    // 0x1000
	fFixLoop        = (1 << 13),    // 0x2000
	fDidntMove      = (1 << 14),    // 0x4000
	fAdjEgoXY       = (1 << 15)     // 0x8000
};

/**
 * AGI screen object table entry
 */
struct ScreenObjEntry {
	int16 objectNr; // 0-255 -> regular screenObjTable, -1 -> addToPic-view
	uint8 stepTime;
	uint8 stepTimeCount;
	int16 xPos;
	int16 yPos;
	uint8 currentViewNr;
	bool viewReplaced;
	struct AgiView *viewResource;
	uint8 currentLoopNr;
	uint8 loopCount;
	struct AgiViewLoop *loopData;
	uint8 currentCelNr;
	uint8 celCount;
	struct AgiViewCel *celData;
	//int16 xPos2;
	//int16 yPos2;
	int16 xSize;
	int16 ySize;

	int16 xPos_prev;
	int16 yPos_prev;
	int16 xSize_prev;
	int16 ySize_prev;

	uint8 stepSize;
	uint8 cycleTime;
	uint8 cycleTimeCount;
	uint8 direction;
	MotionType motionType;
	CycleType cycle;
	uint8 priority;
	uint16 flags;
	// kMotionMoveObj
	int16 move_x;
	int16 move_y;
	uint8 move_stepSize;
	uint8 move_flag;
	// kMotionFollowEgo
	uint8 follow_stepSize;
	uint8 follow_flag;
	uint8 follow_count;
	// kMotionWander
	uint8 wander_count;
	// end of motion related variables
	uint8 loop_flag;

	void reset() { memset(this, 0, sizeof(ScreenObjEntry)); }
	ScreenObjEntry() { reset(); }
}; // struct vt_entry

} // End of namespace Agi

#endif /* AGI_VIEW_H */
