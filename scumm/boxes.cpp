	/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "boxes.h"
#include "common/util.h"

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct Box {				/* Internal walkbox file format */
	union {
		struct {
			byte uy;
			byte ly;
			byte ulx;
			byte urx;
			byte llx;
			byte lrx;
			byte mask;
			byte flags;
		} GCC_PACK v2;

		struct {
			int16 ulx, uly;
			int16 urx, ury;
			int16 lrx, lry;
			int16 llx, lly;
			byte mask;
			byte flags;
			uint16 scale;
		} GCC_PACK old;

		struct {
			int32 ulx, uly;
			int32 urx, ury;
			int32 lrx, lry;
			int32 llx, lly;
			uint32 mask;	// FIXME - is 'mask' really here?
			uint32 flags;	// FIXME - is 'flags' really here?
			uint32 scaleSlot;
			uint32 scale;
			uint32 unk2;
			uint32 unk3;
		} GCC_PACK v8;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

#define BOX_MATRIX_SIZE 2000
#define BOX_DEBUG 0


static bool compareSlope(int X1, int Y1, int X2, int Y2, int X3, int Y3);
static ScummVM::Point closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y);


byte Scumm::getMaskFromBox(int box) {
	// Fix for bug #740244 and #755863. This appears to have been a 
	// long standing bug in the original engine?
	if (_version <= 3 && box == 255)
		return 1;

	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 0;

	if (_version == 8)
		return (byte) FROM_LE_32(ptr->v8.mask);
	else if (_version <= 2)
		return ptr->v2.mask;
	else
		return ptr->old.mask;
}

void Scumm::setBoxFlags(int box, int val) {
	debug(2, "setBoxFlags(%d, 0x%02x)", box, val);

	/* FULL_THROTTLE stuff */
	if (val & 0xC000) {
		assert(box >= 0 && box < 65);
		_extraBoxFlags[box] = val;
	} else {
		Box *ptr = getBoxBaseAddr(box);
		assert(ptr);
		if (_version == 8)
			ptr->v8.flags = TO_LE_32(val);
		else if (_version <= 2)
			ptr->v2.flags = val;
		else
			ptr->old.flags = val;
	}
}

byte Scumm::getBoxFlags(int box) {
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 0;
	if (_version == 8)
		return (byte) FROM_LE_32(ptr->v8.flags);
	else if (_version <= 2)
		return ptr->v2.flags;
	else
		return ptr->old.flags;
}

void Scumm::setBoxScale(int box, int scale) {
	Box *ptr = getBoxBaseAddr(box);
	assert(ptr);
	if (_version == 8)
		ptr->v8.scale = TO_LE_32(scale);
	else if (_version <= 2)
		error("This should not ever be called!");
	else
		ptr->old.scale = TO_LE_16(scale);
}

void Scumm::setBoxScaleSlot(int box, int slot) {
	Box *ptr = getBoxBaseAddr(box);
	assert(ptr);
	ptr->v8.scaleSlot = TO_LE_32(slot);
}

int Scumm::getScale(int box, int x, int y) {
	if (_features & GF_NO_SCALING)
		return 255;

	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 255;
		
	int slot , scale;

	if (_version == 8) {
		// COMI has a separate field for the scale slot...
		slot = FROM_LE_32(ptr->v8.scaleSlot);
		scale = FROM_LE_32(ptr->v8.scale);
	} else {
		scale = READ_LE_UINT16(&ptr->old.scale);
		if (scale & 0x8000)
			slot = (scale & 0x7FFF) + 1;
		else
			slot = 0;
	}

	// Was a scale slot specified? If so, we compute the effective scale
	// from it, ignoring the box scale.
	if (slot) {
		assert(1 <= slot && slot <= ARRAYSIZE(_scaleSlots));
		int scaleX = 0, scaleY = 0;
		ScaleSlot &s = _scaleSlots[slot-1];

		if (s.y1 == s.y2 && s.x1 == s.x2)
			error("Invalid scale slot %d", slot);

		if (s.y1 != s.y2) {
			if (y < 0)
				y = 0;

			scaleY = (s.scale2 - s.scale1) * (y - s.y1) / (s.y2 - s.y1) + s.scale1;
		}
		if (s.x1 == s.x2) {
			scale = scaleY;
		} else {
			scaleX = (s.scale2 - s.scale1) * (x - s.x1) / (s.x2 - s.x1) + s.scale1;

			if (s.y1 == s.y2) {
				scale = scaleX;
			} else {
				scale = (scaleX + scaleY - s.x1) / 2;
			}
		}

		// Clip the scale to range 1-255
		if (scale < 1)
			scale = 1;
		else if (scale > 255)
			scale = 255;
	}
	
	// Finally return the scale
	return scale;
}

int Scumm::getBoxScale(int box) {
	if (_features & GF_NO_SCALING)
		return 255;
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 255;
	if (_version == 8)
		return FROM_LE_32(ptr->v8.scale);
	else
		return READ_LE_UINT16(&ptr->old.scale);
}

/**
 * Convert a rtScaleTable resource to a corresponding scale slot entry.
 *
 * At some point, we discovered that the old scale items (stored in rtScaleTable
 * resources) are in fact the same as (or rather, a predecessor of) the
 * scale slots used in COMI. While not being precomputed (and thus slightly
 * slower), they are more flexible, and most importantly, can cope with
 * rooms higher than 200 pixels. That's an essential feature for DIG and FT
 * and in fact the lack of it caused various bugs in the past.
 *
 * Hence, we decided to switch all games to use the more powerful scale slots.
 * To accomodate old savegames, we attempt here to convert rtScaleTable
 * resources to scale slots.
 */
void Scumm::convertScaleTableToScaleSlot(int slot) {
	assert(1 <= slot && slot <= ARRAYSIZE(_scaleSlots));

	byte *resptr = getResourceAddress(rtScaleTable, slot);
	ScaleSlot &s = _scaleSlots[slot-1];
	int lowerIdx, upperIdx;
	float m, oldM;
	
	// Do nothing if the given scale table doesn't exist
	if (resptr == 0)
		return;
	
	if (resptr[0] == resptr[199]) {
		// The scale is constant This usually means we encountered one of the
		// "broken" cases. We set pseudo scale item values which lead to a 
		// constant scale of 255.
		s.y1 = 0;
		s.y2 = 199;
		s.scale1 = s.scale2 = 255;
		return;
	}
	
	/*
	 * Essentially, what we are doing here is some kind of "line fitting"
	 * algorithm. The data in the scale table represents a linear graph. What
	 * we want to find is the slope and (vertical) offset of this line. Things
	 * are complicated by the fact that the line is cut of vertically at 1 and
	 * 255. We have to be careful in handling this and some border cases.
	 * 
	 * Some typical graphs look like these:
	 *       ---         ---     ---
	 *       /         ---           \ 
	 *  ___/       ---               \___
	 * 
	 * The method used here is to compute the slope of secants fixed at the
	 * left and right end. For most cases this detects the cut-over points
	 * quite accurately. 
	 */
	
	// Search for the bend on the left side
	m = (resptr[199] - resptr[0]) / 199.0;
	for (lowerIdx = 0; lowerIdx < 199 && (resptr[lowerIdx] == 1 || resptr[lowerIdx] == 255); lowerIdx++) {
		oldM = m;
		m = (resptr[199] - resptr[lowerIdx+1]) / (float)(199 - (lowerIdx+1));
		if (m > 0) {
			if (m <= oldM)
				break;
		} else {
			if (m >= oldM)
				break;
		}
	}
	
	// Search for the bend on the right side
	m = (resptr[199] - resptr[0]) / 199.0;
	for (upperIdx = 199; upperIdx > 1 && (resptr[upperIdx] == 1 || resptr[upperIdx] == 255); upperIdx--) {
		oldM = m;
		m = (resptr[upperIdx-1] - resptr[0]) / (float)(upperIdx-1);
		if (m > 0) {
			if (m <= oldM)
				break;
		} else {
			if (m >= oldM)
				break;
		}
	}

	// If lowerIdx and upperIdx are equal, we assume that there
	// was no bend at all, and go for the maximum range.
	if (lowerIdx == upperIdx) {
		lowerIdx = 0;
		upperIdx = 199;
	}

	// The values of y1 and y2, as well as the scale, can now easily be computed
	s.y1 = lowerIdx;
	s.y2 = upperIdx;
	s.scale1 = resptr[lowerIdx];
	s.scale2 = resptr[upperIdx];

	
	// Compute the variance, for debugging. It shouldn't exceed 1
	int y;
	int sum = 0;
	int scale;
	float variance;
	for (y = 0; y < 200; y++) {
		scale = (s.scale2 - s.scale1) * (y - s.y1) / (s.y2 - s.y1) + s.scale1;
		if (scale < 1)
			scale = 1;
		else if (scale > 255)
			scale = 255;

		sum += (resptr[y] - scale) * (resptr[y] - scale);
	}
	variance = sum / (200.0 - 1.0);
	if (variance > 1)
		warning("scale item %d, variance %f exceeds 1 (room %d)\n", slot, variance, _currentRoom);
}

void Scumm::setScaleSlot(int slot, int x1, int y1, int scale1, int x2, int y2, int scale2) {
	assert(1 <= slot && slot <= ARRAYSIZE(_scaleSlots));
	ScaleSlot &s = _scaleSlots[slot-1];
	s.x2 = x2;
	s.y2 = y2;
	s.scale2 = scale2;
	s.x1 = x1;
	s.y1 = y1;
	s.scale1 = scale1;
}

byte Scumm::getNumBoxes() {
	byte *ptr = getResourceAddress(rtMatrix, 2);
	if (!ptr)
		return 0;
	if (_version == 8)
		return (byte) READ_LE_UINT32(ptr);
	else
		return ptr[0];
}

Box *Scumm::getBoxBaseAddr(int box) {
	byte *ptr = getResourceAddress(rtMatrix, 2);
	if (!ptr || box == 255)
		return NULL;

	// FIXME: In "pass to adventure", the loom demo, when bobbin enters
	// the tent to the elders, box = 2, but ptr[0] = 2 -> errors out.
	// Hence we disable the check for now. Maybe in PASS (and other old games)
	// we shouldn't subtract 1 from ptr[0] when performing the check?
	// this also seems to be incorrect for atari st demo of zak
	// and assumingly other v2 games
	// The same happens in Indy3EGA (see bug #770351)
	// Also happend in ZakEGA (see bug #771803).
	//
	// This *might* mean that we have a bug in our box implementation
	// OTOH, the original engine, unlike ScummVM, performed no bound
	// checking at all. All the problems so far have been cases where
	// the value was exactly one more than what we consider the maximum.
	// So it's very well possible that all of these are script errors.
	if ((_gameId == GID_PASS) || ((_features & GF_OLD_BUNDLE)
	    && (_gameId == GID_INDY3 || _gameId == GID_ZAK))) {
		checkRange(ptr[0], 0, box, "Illegal box %d");
	} else
		checkRange(ptr[0] - 1, 0, box, "Illegal box %d");

	if (_version <= 2)
		return (Box *)(ptr + box * SIZEOF_BOX_V2 + 1);
	else if (_version == 3)
		return (Box *)(ptr + box * SIZEOF_BOX_V3 + 1);
	else if (_features & GF_SMALL_HEADER)
		return (Box *)(ptr + box * SIZEOF_BOX + 1);
	else if (_version == 8)
		return (Box *)(ptr + box * SIZEOF_BOX_V8 + 4);
	else
		return (Box *)(ptr + box * SIZEOF_BOX + 2);
}

int Scumm::getSpecialBox(int x, int y) {
	int i;
	int numOfBoxes;
	byte flag;

	numOfBoxes = getNumBoxes() - 1;

	for (i = numOfBoxes; i >= 0; i--) {
		flag = getBoxFlags(i);

		if (!(flag & kBoxInvisible) && (flag & kBoxPlayerOnly))
			return (-1);

		if (checkXYInBoxBounds(i, x, y))
			return (i);
	}

	return (-1);
}

bool Scumm::checkXYInBoxBounds(int b, int x, int y) {
	BoxCoords box;

	if (b < 0 || b == Actor::kInvalidBox)
		return false;

	getBoxCoordinates(b, &box);

	if (x < box.ul.x && x < box.ur.x && x < box.lr.x && x < box.ll.x)
		return false;

	if (x > box.ul.x && x > box.ur.x && x > box.lr.x && x > box.ll.x)
		return false;

	if (y < box.ul.y && y < box.ur.y && y < box.lr.y && y < box.ll.y)
		return false;

	if (y > box.ul.y && y > box.ur.y && y > box.lr.y && y > box.ll.y)
		return false;

	if (box.ul.x == box.ur.x && box.ul.y == box.ur.y && box.lr.x == box.ll.x && box.lr.y == box.ll.y ||
		box.ul.x == box.ll.x && box.ul.y == box.ll.y && box.ur.x == box.lr.x && box.ur.y == box.lr.y) {

		ScummVM::Point pt;
		pt = closestPtOnLine(box.ul.x, box.ul.y, box.lr.x, box.lr.y, x, y);
		if (distanceFromPt(x, y, pt.x, pt.y) <= 4)
			return true;
	}

	if (!compareSlope(box.ul.x, box.ul.y, box.ur.x, box.ur.y, x, y))
		return false;

	if (!compareSlope(box.ur.x, box.ur.y, box.lr.x, box.lr.y, x, y))
		return false;

	if (!compareSlope(box.ll.x, box.ll.y, x, y, box.lr.x, box.lr.y))
		return false;

	if (!compareSlope(box.ul.x, box.ul.y, x, y, box.ll.x, box.ll.y))
		return false;

	return true;
}

void Scumm::getBoxCoordinates(int boxnum, BoxCoords *box) {
	Box *bp = getBoxBaseAddr(boxnum);
	assert(bp);

	if (_version == 8) {
		box->ul.x = (short)FROM_LE_32(bp->v8.ulx);
		box->ul.y = (short)FROM_LE_32(bp->v8.uly);
		box->ur.x = (short)FROM_LE_32(bp->v8.urx);
		box->ur.y = (short)FROM_LE_32(bp->v8.ury);
	
		box->ll.x = (short)FROM_LE_32(bp->v8.llx);
		box->ll.y = (short)FROM_LE_32(bp->v8.lly);
		box->lr.x = (short)FROM_LE_32(bp->v8.lrx);
		box->lr.y = (short)FROM_LE_32(bp->v8.lry);

		// FIXME: Some walkboxes in CMI appear to have been flipped,
		// in the sense that for instance the lower boundary is above
		// the upper one. Can that really be the case, or is there
		// some more sinister problem afoot?
		//
		// Is this fix sufficient, or will we need something more
		// elaborate?

		if (box->ul.y > box->ll.y && box->ur.y > box->lr.y) {
			SWAP(box->ul.x, box->ll.x);
			SWAP(box->ul.y, box->ll.y);
			SWAP(box->ur.x, box->lr.x);
			SWAP(box->ur.y, box->lr.y);
		}

		if (box->ul.x > box->ur.x && box->ll.x > box->lr.x) {
			SWAP(box->ul.x, box->ur.x);
			SWAP(box->ul.y, box->ur.y);
			SWAP(box->ll.x, box->lr.x);
			SWAP(box->ll.y, box->lr.y);
		}
	} else if (_version <= 2) {
		box->ul.x = bp->v2.ulx * 8;
		box->ul.y = bp->v2.uy * 2;
		box->ur.x = bp->v2.urx * 8;
		box->ur.y = bp->v2.uy * 2;
	
		box->ll.x = bp->v2.llx * 8;
		box->ll.y = bp->v2.ly * 2;
		box->lr.x = bp->v2.lrx * 8;
		box->lr.y = bp->v2.ly * 2;
	} else {
		box->ul.x = (int16)READ_LE_UINT16(&bp->old.ulx);
		box->ul.y = (int16)READ_LE_UINT16(&bp->old.uly);
		box->ur.x = (int16)READ_LE_UINT16(&bp->old.urx);
		box->ur.y = (int16)READ_LE_UINT16(&bp->old.ury);
	
		box->ll.x = (int16)READ_LE_UINT16(&bp->old.llx);
		box->ll.y = (int16)READ_LE_UINT16(&bp->old.lly);
		box->lr.x = (int16)READ_LE_UINT16(&bp->old.lrx);
		box->lr.y = (int16)READ_LE_UINT16(&bp->old.lry);
	}
}

uint Scumm::distanceFromPt(int x, int y, int ptx, int pty) {
	int diffx, diffy;

	diffx = abs(ptx - x);

	if (diffx >= 0x1000)
		return 0xFFFFFF;

	diffy = abs(pty - y);

	if (diffy >= 0x1000)
		return 0xFFFFFF;
	diffx *= diffx;
	diffy *= diffy;
	return diffx + diffy;
}


bool compareSlope(int X1, int Y1, int X2, int Y2, int X3, int Y3) {
	return (Y2 - Y1) * (X3 - X1) <= (Y3 - Y1) * (X2 - X1);
}

ScummVM::Point closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y) {
	int lydiff, lxdiff;
	int32 dist, a, b, c;
	int x2, y2;
	ScummVM::Point pt;

	if (llx == ulx) {	// Vertical line?
		x2 = ulx;
		y2 = y;
	} else if (lly == uly) {	// Horizontal line?
		x2 = x;
		y2 = uly;
	} else {
		lydiff = lly - uly;

		lxdiff = llx - ulx;

		if (abs(lxdiff) > abs(lydiff)) {
			dist = lxdiff * lxdiff + lydiff * lydiff;

			a = ulx * lydiff / lxdiff;
			b = x * lxdiff / lydiff;

			c = (a + b - uly + y) * lydiff * lxdiff / dist;

			x2 = c;
			y2 = c * lydiff / lxdiff - a + uly;
		} else {
			dist = lydiff * lydiff + lxdiff * lxdiff;

			a = uly * lxdiff / lydiff;
			b = y * lydiff / lxdiff;

			c = (a + b - ulx + x) * lydiff * lxdiff / dist;

			y2 = c;
			x2 = c * lxdiff / lydiff - a + ulx;
		}
	}

	lxdiff = llx - ulx;
	lydiff = lly - uly;

	if (abs(lydiff) < abs(lxdiff)) {
		if (lxdiff > 0) {
			if (x2 < ulx) {
			type1:;
				x2 = ulx;
				y2 = uly;
			} else if (x2 > llx) {
			type2:;
				x2 = llx;
				y2 = lly;
			}
		} else {
			if (x2 > ulx)
				goto type1;
			if (x2 < llx)
				goto type2;
		}
	} else {
		if (lydiff > 0) {
			if (y2 < uly)
				goto type1;
			if (y2 > lly)
				goto type2;
		} else {
			if (y2 > uly)
				goto type1;
			if (y2 < lly)
				goto type2;
		}
	}

	pt.x = x2;
	pt.y = y2;
	return pt;
}

bool Scumm::inBoxQuickReject(int b, int x, int y, int threshold) {
	int t;
	BoxCoords box;

	getBoxCoordinates(b, &box);

	t = x - threshold;
	if (t > box.ul.x && t > box.ur.x && t > box.lr.x && t > box.ll.x)
		return true;

	t = x + threshold;
	if (t < box.ul.x && t < box.ur.x && t < box.lr.x && t < box.ll.x)
		return true;

	t = y - threshold;
	if (t > box.ul.y && t > box.ur.y && t > box.lr.y && t > box.ll.y)
		return true;

	t = y + threshold;
	if (t < box.ul.y && t < box.ur.y && t < box.lr.y && t < box.ll.y)
		return true;

	return false;
}

int Scumm::getClosestPtOnBox(int b, int x, int y, int16& outX, int16& outY) {
	ScummVM::Point pt;
	uint dist;
	uint bestdist = 0xFFFFFF;
	BoxCoords box;

	getBoxCoordinates(b, &box);

	pt = closestPtOnLine(box.ul.x, box.ul.y, box.ur.x, box.ur.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		outX = pt.x;
		outY = pt.y;
	}

	pt = closestPtOnLine(box.ur.x, box.ur.y, box.lr.x, box.lr.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		outX = pt.x;
		outY = pt.y;
	}

	pt = closestPtOnLine(box.lr.x, box.lr.y, box.ll.x, box.ll.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		outX = pt.x;
		outY = pt.y;
	}

	pt = closestPtOnLine(box.ll.x, box.ll.y, box.ul.x, box.ul.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		outX = pt.x;
		outY = pt.y;
	}

	return bestdist;
}

byte *Scumm::getBoxMatrixBaseAddr() {
	byte *ptr = getResourceAddress(rtMatrix, 1);
	assert(ptr);
	if (*ptr == 0xFF)
		ptr++;
	return ptr;
}

/*
 * Compute if there is a way that connects box 'from' with box 'to'.
 * Returns the number of a box adjactant to 'from' that is the next on the
 * way to 'to' (this can be 'to' itself or a third box).
 * If there is no connection -1 is return.
 */
int Scumm::getPathToDestBox(byte from, byte to) {
	const byte *boxm;
	byte i;
	const int numOfBoxes = getNumBoxes();
	int dest = -1;
	
	if (from == to)
		return to;

	if (to == Actor::kInvalidBox)
		return -1;

	if (from == Actor::kInvalidBox)
		return to;
	
	assert(from < numOfBoxes);
	assert(to < numOfBoxes);

	boxm = getBoxMatrixBaseAddr();

	if (_version <= 2) {
		// The v2 box matrix is a real matrix with numOfBoxes rows and columns.
		// The first numOfBoxes bytes contain indices to the start of the corresponding
		// row (although that seems unnecessary to me - the value is easily computable.
		boxm += numOfBoxes + boxm[from];
		return (int8)boxm[to];
	}

	// WORKAROUND #1: It seems that in some cases, the box matrix is corrupt
	// (more precisely, is too short) in the datafiles already. In
	// particular this seems to be the case in room 46 of Indy3 EGA (see
	// also bug #770690). This didn't cause problems in the original
	// engine, because there, the memory layout is different. After the
	// walkbox would follow the rest of the room file, thus the program
	// always behaved the same (and by chance, correct). Not so for us,
	// since random data may follow after the resource in ScummVM.
	//
	// As a workaround, we add a check for the end of the box matrix
	// resource, and abort the search once we reach the end.
	const byte *end = boxm + getResourceSize(rtMatrix, 1);

	// WORKAROUND #2: In addition to the above, we have to add this special
	// case to fix the scene in Indy3 where Indy meets Hitler in Berlin.
	// It's one of the places (or maybe even the only one?). See bug #770690
	// and also bug #774783.
	if ((_gameId == GID_INDY3) && _roomResource == 46 && from == 1 && to == 0)
		return 1;

	// Skip up to the matrix data for box 'from'
	for (i = 0; i < from && boxm < end; i++) {
		while (boxm < end && *boxm != 0xFF)
			boxm += 3;
		boxm++;
	}

	// Now search for the entry for box 'to'
	while (boxm < end && boxm[0] != 0xFF) {
		if (boxm[0] <= to && to <= boxm[1])
			dest = (int8)boxm[2];
		boxm += 3;
	}
	
	if (boxm >= end)
		warning("The box matrix apparently is truncated (room %d)", _roomResource);

	return dest;
}

/*
 * Computes the next point actor a has to walk towards in a straight
 * line in order to get from box1 to box3 via box2.
 */
bool Actor::findPathTowards(byte box1nr, byte box2nr, byte box3nr, int16 &foundPathX, int16 &foundPathY) {
	BoxCoords box1;
	BoxCoords box2;
	ScummVM::Point tmp;
	int i, j;
	int flag;
	int q, pos;

	_vm->getBoxCoordinates(box1nr, &box1);
	_vm->getBoxCoordinates(box2nr, &box2);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (box1.ul.x == box1.ur.x && box1.ul.x == box2.ul.x && box1.ul.x == box2.ur.x) {
				flag = 0;
				if (box1.ul.y > box1.ur.y) {
					SWAP(box1.ul.y, box1.ur.y);
					flag |= 1;
				}

				if (box2.ul.y > box2.ur.y) {
					SWAP(box2.ul.y, box2.ur.y);
					flag |= 2;
				}

				if (box1.ul.y > box2.ur.y || box2.ul.y > box1.ur.y ||
						(box1.ur.y == box2.ul.y || box2.ur.y == box1.ul.y) &&
						box1.ul.y != box1.ur.y && box2.ul.y != box2.ur.y) {
					if (flag & 1)
						SWAP(box1.ul.y, box1.ur.y);
					if (flag & 2)
						SWAP(box2.ul.y, box2.ur.y);
				} else {
					pos = y;
					if (box2nr == box3nr) {
						int diffX = walkdata.destx - x;
						int diffY = walkdata.desty - y;
						int boxDiffX = box1.ul.x - x;

						if (diffX != 0) {
							int t;

							diffY *= boxDiffX;
							t = diffY / diffX;
							if (t == 0 && (diffY <= 0 || diffX <= 0)
									&& (diffY >= 0 || diffX >= 0))
								t = -1;
							pos = y + t;
						}
					}

					q = pos;
					if (q < box2.ul.y)
						q = box2.ul.y;
					if (q > box2.ur.y)
						q = box2.ur.y;
					if (q < box1.ul.y)
						q = box1.ul.y;
					if (q > box1.ur.y)
						q = box1.ur.y;
					if (q == pos && box2nr == box3nr)
						return true;
					foundPathY = q;
					foundPathX = box1.ul.x;
					return false;
				}
			}

			if (box1.ul.y == box1.ur.y && box1.ul.y == box2.ul.y && box1.ul.y == box2.ur.y) {
				flag = 0;
				if (box1.ul.x > box1.ur.x) {
					SWAP(box1.ul.x, box1.ur.x);
					flag |= 1;
				}

				if (box2.ul.x > box2.ur.x) {
					SWAP(box2.ul.x, box2.ur.x);
					flag |= 2;
				}

				if (box1.ul.x > box2.ur.x || box2.ul.x > box1.ur.x ||
						(box1.ur.x == box2.ul.x || box2.ur.x == box1.ul.x) &&
						box1.ul.x != box1.ur.x && box2.ul.x != box2.ur.x) {
					if (flag & 1)
						SWAP(box1.ul.x, box1.ur.x);
					if (flag & 2)
						SWAP(box2.ul.x, box2.ur.x);
				} else {

					if (box2nr == box3nr) {
						int diffX = walkdata.destx - x;
						int diffY = walkdata.desty - y;
						int boxDiffY = box1.ul.y - y;

						pos = x;
						if (diffY != 0) {
							pos += diffX * boxDiffY / diffY;
						}
					} else {
						pos = x;
					}

					q = pos;
					if (q < box2.ul.x)
						q = box2.ul.x;
					if (q > box2.ur.x)
						q = box2.ur.x;
					if (q < box1.ul.x)
						q = box1.ul.x;
					if (q > box1.ur.x)
						q = box1.ur.x;
					if (q == pos && box2nr == box3nr)
						return true;
					foundPathX = q;
					foundPathY = box1.ul.y;
					return false;
				}
			}
			tmp = box1.ul;
			box1.ul = box1.ur;
			box1.ur = box1.lr;
			box1.lr = box1.ll;
			box1.ll = tmp;
		}
		tmp = box2.ul;
		box2.ul = box2.ur;
		box2.ur = box2.lr;
		box2.lr = box2.ll;
		box2.ll = tmp;
	}
	return false;
}

#if BOX_DEBUG
static void printMatrix(byte *boxm, int num) {
	int i;
	for (i = 0; i < num; i++) {
		printf("%d: ", i);
		while (*boxm != 0xFF) {
			printf("%d, ", *boxm);
			boxm++;
		}
		boxm++;
		printf("\n");
	}
}

static void printMatrix2(byte *matrix, int num) {
	int i, j;
	printf("    ");
	for (i = 0; i < num; i++)
		printf("%2d ", i);
	printf("\n");
	for (i = 0; i < num; i++) {
		printf("%2d: ", i);
		for (j = 0; j < num; j++) {
			int val = matrix[i * 64 + j];
			if (val == Actor::kInvalidBox)
				printf(" ? ");
			else
				printf("%2d ", val);
		}
		printf("\n");
	}
}
#endif

void Scumm::createBoxMatrix() {
	int num, i, j, k;
	byte *adjacentMatrix, *itineraryMatrix;

	// The total number of boxes
	num = getNumBoxes();
	assert(num <= 64);

	// Allocate the adjacent & itinerary matrices
	adjacentMatrix = (byte *)malloc(64 * 64);
	itineraryMatrix = (byte *)malloc(64 * 64);

	// Initialise the adjacent matrix: each box has distance 0 to itself,
	// and distance 1 to its direct neighbors. Initially, it has distance
	// 255 (= infinity) to all other boxes.
	for (i = 0; i < num; i++) {
		for (j = 0; j < num; j++) {
			if (i == j) {
				adjacentMatrix[i * 64 + j] = 0;
				itineraryMatrix[i * 64 + j] = j;
			} else if (areBoxesNeighbours(i, j)) {
				adjacentMatrix[i * 64 + j] = 1;
				itineraryMatrix[i * 64 + j] = j;
			} else {
				adjacentMatrix[i * 64 + j] = 255;
				itineraryMatrix[i * 64 + j] = Actor::kInvalidBox;
			}
		}
	}

	// Compute the shortest routes between boxes via Kleene's algorithm.
	// The original code used some kind of mangled Dijkstra's algorithm;
	// while that might in theory be slightly faster, it was
	// a) extremly obfuscated
	// b) incorrect: it didn't always find the shortest paths
	// c) not any faster in reality for our sparse & small adjacent matrices
	for (k = 0; k < num; k++) {
		for (i = 0; i < num; i++) {
			for (j = 0; j < num; j++) {
				if (i == j)
					continue;
				byte distIK = adjacentMatrix[64 * i + k];
				byte distKJ = adjacentMatrix[64 * k + j];
				if (adjacentMatrix[64 * i + j] > distIK + distKJ) {
					adjacentMatrix[64 * i + j] = distIK + distKJ;
					itineraryMatrix[64 * i + j] = itineraryMatrix[64 * i + k];
				}
			}
		}
	
	}

	// "Compress" the distance matrix into the box matrix format used
	// by the engine. The format is like this:
	// For each box (from 0 to num) there is first a byte with value 0xFF,
	// followed by an arbitrary number of byte triples; the end is marked
	// again by the lead 0xFF for the next "row". The meaning of the
	// byte triples is as follows: the first two bytes define a range
	// of box numbers (e.g. 7-11), while the third byte defines an 
	// itineray box. Assuming we are in the 5th "row" and encounter 
	// the triplet 7,11,15: this means to get from box 5 to any of
	// the boxes 7,8,9,10,11 the shortest way is to go via box 15.
	// See also getPathToDestBox.
	
	byte *matrixStart = createResource(rtMatrix, 1, BOX_MATRIX_SIZE);
	const byte *matrixEnd = matrixStart + BOX_MATRIX_SIZE;

	#define addToMatrix(b)	do { *matrixStart++ = (b); assert(matrixStart < matrixEnd); } while (0)

	for (i = 0; i < num; i++) {
		addToMatrix(0xFF);
		for (j = 0; j < num; j++) {
			byte itinerary = itineraryMatrix[64 * i + j];
			if (itinerary != Actor::kInvalidBox) {
				addToMatrix(j);
				while (j < num - 1 && itinerary == itineraryMatrix[64 * i + (j + 1)])
					j++;
				addToMatrix(j);
				addToMatrix(itinerary);
			}
		}
	}
	addToMatrix(0xFF);
	

#if BOX_DEBUG
	printf("Itinerary matrix:\n");
	printMatrix2(itineraryMatrix, num);
	printf("compressed matrix:\n");
	printMatrix(getBoxMatrixBaseAddr(), num);
#endif

	free(adjacentMatrix);
	free(itineraryMatrix);
}

/** Check if two boxes are neighbours. */
bool Scumm::areBoxesNeighbours(int box1nr, int box2nr) {
	int j, k, m, n;
	int tmp_x, tmp_y;
	bool result;
	BoxCoords box;
	BoxCoords box2;

	if (getBoxFlags(box1nr) & kBoxInvisible || getBoxFlags(box2nr) & kBoxInvisible)
		return false;

	getBoxCoordinates(box1nr, &box2);
	getBoxCoordinates(box2nr, &box);

	result = false;
	j = 4;

	do {
		k = 4;
		do {
			if (box2.ur.x == box2.ul.x && box.ul.x == box2.ul.x && box.ur.x == box2.ur.x) {
				n = m = 0;
				if (box2.ur.y < box2.ul.y) {
					n = 1;
					SWAP(box2.ur.y, box2.ul.y);
				}
				if (box.ur.y < box.ul.y) {
					m = 1;
					SWAP(box.ur.y, box.ul.y);
				}
				if (box.ur.y < box2.ul.y ||
						box.ul.y > box2.ur.y ||
						(box.ul.y == box2.ur.y ||
						 box.ur.y == box2.ul.y) && box2.ur.y != box2.ul.y && box.ul.y != box.ur.y) {
					if (n) {
						SWAP(box2.ur.y, box2.ul.y);
					}
					if (m) {
						SWAP(box.ur.y, box.ul.y);
					}
				} else {
					if (n) {
						SWAP(box2.ur.y, box2.ul.y);
					}
					if (m) {
						SWAP(box.ur.y, box.ul.y);
					}
					result = true;
				}
			}

			if (box2.ur.y == box2.ul.y && box.ul.y == box2.ul.y && box.ur.y == box2.ur.y) {
				n = m = 0;
				if (box2.ur.x < box2.ul.x) {
					n = 1;
					SWAP(box2.ur.x, box2.ul.x);
				}
				if (box.ur.x < box.ul.x) {
					m = 1;
					SWAP(box.ur.x, box.ul.x);
				}
				if (box.ur.x < box2.ul.x ||
						box.ul.x > box2.ur.x ||
						(box.ul.x == box2.ur.x ||
						 box.ur.x == box2.ul.x) && box2.ur.x != box2.ul.x && box.ul.x != box.ur.x) {

					if (n) {
						SWAP(box2.ur.x, box2.ul.x);
					}
					if (m) {
						SWAP(box.ur.x, box.ul.x);
					}
				} else {
					if (n) {
						SWAP(box2.ur.x, box2.ul.x);
					}
					if (m) {
						SWAP(box.ur.x, box.ul.x);
					}
					result = true;
				}
			}

			tmp_x = box2.ul.x;
			tmp_y = box2.ul.y;
			box2.ul.x = box2.ur.x;
			box2.ul.y = box2.ur.y;
			box2.ur.x = box2.lr.x;
			box2.ur.y = box2.lr.y;
			box2.lr.x = box2.ll.x;
			box2.lr.y = box2.ll.y;
			box2.ll.x = tmp_x;
			box2.ll.y = tmp_y;
		} while (--k);

		tmp_x = box.ul.x;
		tmp_y = box.ul.y;
		box.ul.x = box.ur.x;
		box.ul.y = box.ur.y;
		box.ur.x = box.lr.x;
		box.ur.y = box.lr.y;
		box.lr.x = box.ll.x;
		box.lr.y = box.ll.y;
		box.ll.x = tmp_x;
		box.ll.y = tmp_y;
	} while (--j);

	return result;
}

void Actor::findPathTowardsOld(byte trap1, byte trap2, byte final_trap, ScummVM::Point gateLoc[5]) {
	ScummVM::Point pt;
	ScummVM::Point gateA[2];
	ScummVM::Point gateB[2];

	_vm->getGates(trap1, trap2, gateA, gateB);

	gateLoc[1].x = x;
	gateLoc[1].y = y;
	gateLoc[2].x = 32000;
	gateLoc[3].x = 32000;
	gateLoc[4].x = 32000;

	if (trap2 == final_trap) {		/* next = final box? */
		gateLoc[4].x = walkdata.destx;
		gateLoc[4].y = walkdata.desty;

		if (_vm->getMaskFromBox(trap1) == _vm->getMaskFromBox(trap2) || 1) {
			if (compareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gateA[0].x, gateA[0].y) !=
					compareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gateB[0].x, gateB[0].y) &&
					compareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gateA[1].x, gateA[1].y) !=
					compareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gateB[1].x, gateB[1].y)) {
				return;								/* same zplane and between both gates? */
			}
		}
	}

	pt = closestPtOnLine(gateA[1].x, gateA[1].y, gateB[1].x, gateB[1].y, gateLoc[1].x, gateLoc[1].y);
	gateLoc[3].x = pt.x;
	gateLoc[3].y = pt.y;

	if (compareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[3].x, gateLoc[3].y, gateA[0].x, gateA[0].y) ==
			compareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[3].x, gateLoc[3].y, gateB[0].x, gateB[0].y)) {
		closestPtOnLine(gateA[0].x, gateA[0].y, gateB[0].x, gateB[0].y, gateLoc[1].x, gateLoc[1].y);
		gateLoc[2].x = pt.x;				/* if point 2 between gates, ignore! */
		gateLoc[2].y = pt.y;
	}

	return;
}

void Scumm::getGates(int trap1, int trap2, ScummVM::Point gateA[2], ScummVM::Point gateB[2]) {
	int i, j;
	int dist[8];
	int minDist[3];
	int closest[3];
	int box[3];
	BoxCoords coords;
	ScummVM::Point Clo[8];
	ScummVM::Point poly[8];
	int line1, line2;

	// For all corner coordinates of the first box, compute the point closest 
	// to them on the second box (and also compute the distance of these points).
	getBoxCoordinates(trap1, &coords);
	poly[0] = coords.ul;
	poly[1] = coords.ur;
	poly[2] = coords.lr;
	poly[3] = coords.ll;
	for (i = 0; i < 4; i++) {
		dist[i] = getClosestPtOnBox(trap2, poly[i].x, poly[i].y, Clo[i].x, Clo[i].y);
	}

	// Now do the same but with the roles of the first and second box swapped.
	getBoxCoordinates(trap2, &coords);
	poly[4] = coords.ul;
	poly[5] = coords.ur;
	poly[6] = coords.lr;
	poly[7] = coords.ll;
	for (i = 4; i < 8; i++) {
		dist[i] = getClosestPtOnBox(trap1, poly[i].x, poly[i].y, Clo[i].x, Clo[i].y);
	}

	// Find the three closest "close" points between the two boxes.
	for (j = 0; j < 3; j++) {
		minDist[j] = 0xFFFF;
		for (i = 0; i < 8; i++) {
			if (dist[i] < minDist[j]) {
				minDist[j] = dist[i];
				closest[j] = i;
			}
		}
		dist[closest[j]] = 0xFFFF;
		minDist[j] = (int)sqrt((double)minDist[j]);
		box[j] = (closest[j] > 3);	// Is the point on the first or on the second box?
	}


	// Finally, compute the "gate". That's a pair of two points that are
	// in the same box (actually, on the border of that box), which both have
	// "minimal" distance to the other box in a certain sense.

	if (box[0] == box[1] && abs(minDist[0] - minDist[1]) < 4) {
		line1 = closest[0];
		line2 = closest[1];

	} else if (box[0] == box[1] && minDist[0] == minDist[1]) {	/* parallel */
		line1 = closest[0];
		line2 = closest[1];
	} else if (box[0] == box[2] && minDist[0] == minDist[2]) {	/* parallel */
		line1 = closest[0];
		line2 = closest[2];
	} else if (box[1] == box[2] && minDist[1] == minDist[2]) {	/* parallel */
		line1 = closest[1];
		line2 = closest[2];

	} else if (box[0] == box[2] && abs(minDist[0] - minDist[2]) < 4) {
		line1 = closest[0];
		line2 = closest[2];
	} else if (abs(minDist[0] - minDist[2]) < 4) {	/* if 1 close to 3 then use 2-3 */
		line1 = closest[1];
		line2 = closest[2];
	} else if (abs(minDist[0] - minDist[1]) < 4) {
		line1 = closest[0];
		line2 = closest[1];
	} else {
		line1 = closest[0];
		line2 = closest[0];
	}

	// Set the gate
	if (line1 < 4) {							/* from box 1 to box 2 */
		gateA[0] = poly[line1];
		gateA[1] = Clo[line1];
	} else {
		gateA[1] = poly[line1];
		gateA[0] = Clo[line1];
	}

	if (line2 < 4) {							/* from box */
		gateB[0] = poly[line2];
		gateB[1] = Clo[line2];
	} else {
		gateB[1] = poly[line2];
		gateB[0] = Clo[line2];
	}
}

