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

#include "common/scummsys.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

namespace Trecision {

/*------------------------------------------------
					resetZBuffer
--------------------------------------------------*/
void resetZBuffer(int x1, int y1, int x2, int y2) {
	if (x1 > x2 || y1 > y2)
		return;

	int size = (x2 - x1) * (y2 - y1);
	if (size * 2 > ZBUFFERSIZE)
		warning("Warning: ZBuffer size %d!\n", size * 2);

	uint16 *d = (uint16 *)g_vm->ZBuffer;
	for (uint32 i = 0; i < size; i++)
		*d++ = 0x7FFF;
}

/*-----------------10/12/95 15.53-------------------
				CheckMask - Compreso
--------------------------------------------------*/
bool CheckMask(uint16 MX, uint16 MY) {
	for (int8 a = (MAXOBJINROOM - 1); a >= 0; a--) {
		uint16 checkedObj = g_vm->_room[g_vm->_curRoom]._object[a];

		if ((checkedObj) && (g_vm->_obj[checkedObj]._mode & (OBJMODE_OBJSTATUS))) {
			if ((MX >= g_vm->_obj[checkedObj]._lim.left) &&
			    (MY >= g_vm->_obj[checkedObj]._lim.top + TOP) &&
			    (MX <= g_vm->_obj[checkedObj]._lim.right) &&
			    (MY <= g_vm->_obj[checkedObj]._lim.bottom + TOP)) {

				if (g_vm->_obj[checkedObj]._mode & OBJMODE_FULL) {
					g_vm->_curObj = checkedObj;
					return true;
				}

				if (g_vm->_obj[checkedObj]._mode & OBJMODE_LIM) {
					g_vm->_curObj = checkedObj;
					return true;
				}

				if (g_vm->_obj[checkedObj]._mode & OBJMODE_MASK) {
					uint8 *mask = MaskPointers[a];
					int16 d = g_vm->_obj[checkedObj]._px;

					for (int16 b = g_vm->_obj[checkedObj]._py; b < (g_vm->_obj[checkedObj]._py + g_vm->_obj[checkedObj]._dy); b++) {
						int16 c = 0;
						int16 e = 0;
						while (e < g_vm->_obj[checkedObj]._dx) {
							if (!c) {		// not inside an object
								if (b + TOP == MY) {
									if ((MX >= (d + e)) && (MX < (d + e + *mask))) {
										g_vm->_curObj = 0;
									}
								}

								e += *mask;
								mask++;
								c = 1;
							} else {		// inside an object
								if (b + TOP == MY) {
									if ((MX >= (d + e)) && (MX < (d + e + *mask))) {
										g_vm->_curObj = checkedObj;
										return true;
									}
								}

								e += *mask;
								mask++;
								c = 0;
							}
						}
					}
				}
			}
		}
	}
	g_vm->_curObj = 0;
	return false;
}

} // End of namespace Trecision
