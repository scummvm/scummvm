/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/gfx/rab_api.h"

namespace ICB {

// Decompress the FrameData into the CurrentFrame data storage
// e.g. undo the delta compression or replace the zero ignored
// values with zero
Bone_Frame *rab_API::GetFrame(const int f) {
	Bone_Frame *curFrm = GetCurrentFrame();
	Bone_Frame *prevFrm = NULL;

	if (f != currentFrame) {
		// Decode and decompress the data from the animation frame into
		// the current frame
		FrameData *frm = GetFrameData(f);

		uint32 b, i;
		uint32 nt = frm->nThings;
		int8 *data = frm->data;

		// For deltas we need the previous frame
		if ((frm->typeSize & DataTypeDeltas) != 0) {
			prevFrm = GetFrame(f - 1);
			int dcvx;
			int dcvy;
			int dcvz;
			int prevVx;
			int prevVy;
			int prevVz;
			int curVx;
			int curVy;
			int curVz;
			int temp;
			int prevCrot;
			int nBits = 0;
			int min;
			int mask;
			int storeZero;
			int dataSize;

			// So just need to add on the deltas to the previous frame !
			{
				dataSize = (frm->typeSize & DataSizeBitMask);
				switch (dataSize) {
				case 3: {
					nBits = DELTA_24_NBITS;
					break;
				}
				case 2: {
					nBits = DELTA_16_NBITS;
					break;
				}
				case 1: {
					nBits = DELTA_8_NBITS;
					break;
				}
				}
				min = (1 << (nBits - 1));
				mask = (1 << nBits) - 1;
				storeZero = frm->typeSize & DataStoreZero;
				for (i = 0; i < nt; i++) {
					// The data is not aligned so need to do memcpy
					temp = 0;

					if (storeZero == 0) {
						b = (int)(*data);
						data++;
					} else {
						b = i;
					}
					memcpy(&temp, (data), dataSize);

					// The previous compressed angles
					prevCrot = prevFrm->bones[b].crot;
					prevVx = (prevCrot >> COMP_VX_SHIFT) & COMP_VX_MASK;
					prevVy = (prevCrot >> COMP_VY_SHIFT) & COMP_VY_MASK;
					prevVz = (prevCrot >> COMP_VZ_SHIFT) & COMP_VZ_MASK;

					// The signed deltas
					dcvz = (temp & mask) << COMP_EXTRA_SHIFT;
					dcvz = dcvz - min;

					temp = temp >> nBits;
					dcvy = (temp & mask) << COMP_EXTRA_SHIFT;
					dcvy = dcvy - min;

					temp = temp >> nBits;
					dcvx = (temp & mask) << COMP_EXTRA_SHIFT;
					dcvx = dcvx - min;

					// Add on the deltas
					curVx = prevVx + dcvx;
					curVy = prevVy + dcvy;
					curVz = prevVz + dcvz;

					// Correct for being -ve !
					if (curVx < 0)
						curVx += (COMP_DELTA_RANGE);
					if (curVy < 0)
						curVy += (COMP_DELTA_RANGE);
					if (curVz < 0)
						curVz += (COMP_DELTA_RANGE);
					if (curVx >= (COMP_DELTA_RANGE))
						curVx -= (COMP_DELTA_RANGE);
					if (curVy >= (COMP_DELTA_RANGE))
						curVy -= (COMP_DELTA_RANGE);
					if (curVz >= (COMP_DELTA_RANGE))
						curVz -= (COMP_DELTA_RANGE);

					// Pack up the deltas
					temp = 0;
					temp = (curVx << COMP_VX_SHIFT);
					// y = 10-bits into middle rot
					temp |= (curVy << COMP_VY_SHIFT);
					// z = 10-bits into lower rot
					temp |= (curVz << COMP_VZ_SHIFT);

					// Store the deltas in the currentFrame structure
					curFrm->bones[b].crot = temp;
					data += dataSize;
				}
			}
		} else {
			// switch type stuff on the type of the compressed data
			switch (frm->typeSize) {
			case ALL_ANGLES_32_TYPESIZE: {
				for (b = 0; b < nt; b++) {
					// The data is not aligned so need to do memcpy
					memcpy(&(curFrm->bones[b].crot), (data), sizeof(CompTriplet));
					data += ALL_ANGLES_32_BYTE_SIZE;
				}
				break;
			}
			case NONZERO_ANGLES_32_TYPESIZE: {
				// Zero out the angles first
				for (b = 0; b < nBones; b++) {
					curFrm->bones[b].crot = ZERO_ANGLE;
				}
				// Then update the angles which are non-zero
				for (i = 0; i < nt; i++) {
					b = (int)(*data);
					memcpy(&(curFrm->bones[b].crot), (data + 1), sizeof(CompTriplet));
					data += NONZERO_ANGLES_32_BYTE_SIZE;
				}
				break;
			}
			default: { return NULL; }
			}
		}
		curFrm->poseBone = frm->poseBone;
		currentFrame = (u_char)f;
	}

	return curFrm;
}

// Compress an SVECTOR ( uint16 vx,vy,vz, pad; ) -> uint32
// by dividing the angles (12-bits 0-4095) by four to make them 10-bits
int CompressSVECTOR(SVECTOR rotin, CompTriplet *rotout) {
	int16 vx = rotin.vx;
	int16 vy = rotin.vy;
	int16 vz = rotin.vz;

	// Make the angles +ve
	if (vx < 0)
		vx += 4096;
	if (vy < 0)
		vy += 4096;
	if (vz < 0)
		vz += 4096;

	// Reduce to 10-bits
	vx = (short)((vx >> COMP_ANGLE_SHIFT) & COMP_VX_MASK);
	vy = (short)((vy >> COMP_ANGLE_SHIFT) & COMP_VY_MASK);
	vz = (short)((vz >> COMP_ANGLE_SHIFT) & COMP_VZ_MASK);

	// Pack into a single int32
	// x = 10-bits into upper rot
	int temp = (vx << COMP_VX_SHIFT);
	// y = 10-bits into middle rot
	temp |= (vy << COMP_VY_SHIFT);
	// z = 10-bits into lower rot
	temp |= (vz << COMP_VZ_SHIFT);

	*rotout = temp;
	return 1;
}

} // End of namespace ICB
