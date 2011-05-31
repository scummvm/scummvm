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

#ifndef AGI_VIEW_H
#define AGI_VIEW_H

namespace Agi {

struct ViewCel {
	uint8 height;
	uint8 width;
	uint8 transparency;
	uint8 mirrorLoop;
	uint8 mirror;
	uint8 *data;
};

struct ViewLoop {
	int numCels;
	struct ViewCel *cel;
};

/**
 * AGI view resource structure.
 */
struct AgiView {
	int numLoops;
	struct ViewLoop *loop;
	bool agi256_2;
	char *descr;
	uint8 *rdata;
};

/**
 * AGI view table entry
 */
struct VtEntry {
	uint8 stepTime;
	uint8 stepTimeCount;
	uint8 entry;
	int16 xPos;
	int16 yPos;
	uint8 currentView;
	bool viewReplaced;
	struct AgiView *viewData;
	uint8 currentLoop;
	uint8 numLoops;
	struct ViewLoop *loopData;
	uint8 currentCel;
	uint8 numCels;
	struct ViewCel *celData;
	struct ViewCel *celData2;
	int16 xPos2;
	int16 yPos2;
	void *s;
	int16 xSize;
	int16 ySize;
	uint8 stepSize;
	uint8 cycleTime;
	uint8 cycleTimeCount;
	uint8 direction;

#define MOTION_NORMAL		0
#define MOTION_WANDER		1
#define	MOTION_FOLLOW_EGO	2
#define	MOTION_MOVE_OBJ		3
	uint8 motion;

#define	CYCLE_NORMAL		0
#define CYCLE_END_OF_LOOP	1
#define	CYCLE_REV_LOOP		2
#define	CYCLE_REVERSE		3
	uint8 cycle;

	uint8 priority;

#define DRAWN		0x0001
#define IGNORE_BLOCKS	0x0002
#define FIXED_PRIORITY	0x0004
#define IGNORE_HORIZON	0x0008
#define UPDATE		0x0010
#define CYCLING		0x0020
#define ANIMATED	0x0040
#define MOTION		0x0080
#define ON_WATER	0x0100
#define IGNORE_OBJECTS	0x0200
#define UPDATE_POS	0x0400
#define ON_LAND		0x0800
#define DONTUPDATE	0x1000
#define FIX_LOOP	0x2000
#define DIDNT_MOVE	0x4000
#define	ADJ_EGO_XY	0x8000
	uint16 flags;

	uint8 parm1;
	uint8 parm2;
	uint8 parm3;
	uint8 parm4;
}; // struct vt_entry

} // End of namespace Agi

#endif /* AGI_VIEW_H */
