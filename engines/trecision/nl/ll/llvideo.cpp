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

#include <common/util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

#include "graphics/pixelformat.h"

namespace Trecision {

// locals
extern bool _linearMode;
extern Graphics::PixelFormat _screenFormat;

/*-----------------07/11/95 15.18-------------------
					ResetZB
--------------------------------------------------*/
void ResetZB(int x1, int y1, int x2, int y2) {
	if (x1 > x2 || y1 > y2)
		return;

	int size = (x2 - x1) * (y2 - y1);
	if (size * 2 > ZBUFFERSIZE)
		warning("Warning: ZBuffer size %d!\n", size * 2);

	wordset(ZBuffer, 0x7FFF, size);
}

/*-----------------10/12/95 15.49-------------------
	VCopy - Execute fast copy to video 
--------------------------------------------------*/
void VCopy(uint32 Sco, uint16 *Src, uint32 Len) {
	lockVideo();
	if ((_video == nullptr) || (Len == 0))
		return ;

	if (_linearMode && ((VideoPitch == 0) || (VideoPitch == SCREENLEN * 2))) {
		MCopy(_video + Sco, Src, Len);
		return ;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 endSco = Sco + Len;
	int32 y2 = endSco / SCREENLEN;

	uint32 srcSco = 0;
	uint32 copyNow = MIN<uint32>(Len, SCREENLEN - x1);

	MCopy(_video + y1 * (VideoPitch / 2) + x1, Src + srcSco, copyNow);
	srcSco += copyNow;
	Len -= copyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		copyNow = SCREENLEN;
		MCopy(_video + i * (VideoPitch / 2), Src + srcSco, copyNow);
		srcSco += copyNow;
		Len -= copyNow;
	}

	if (Len > 0) {
		copyNow = Len;
		MCopy(_video + y2 * (VideoPitch / 2), Src + srcSco, copyNow);
	}
}

/*-----------------10/12/95 15.51-------------------
		MCopy - Esegue copia veloce in memoria
--------------------------------------------------*/
void MCopy(uint16 *Dest, uint16 *Src, uint32 Len) {
	if (Len & 1) {
		*Dest++ = *Src++;
		Len --;
	}

	longcopy(Dest, Src, (Len >> 1));
}

/*-----------------10/12/95 15.52-------------------
					ShowScreen
--------------------------------------------------*/
void ShowScreen(int px, int py, int dx, int dy) {
	for (int a = 0; a < dy; a++) {
		VCopy(px + (py + a)*VirtualPageLen + VideoScrollPageDx,
			  Video2 + px + (py + a)*CurRoomMaxX + CurScrollPageDx,
			  dx);
	}

	UnlockVideo();
}

/* -----------------12/06/97 18.25-------------------
				UpdatePixelFormat
 --------------------------------------------------*/
void UpdatePixelFormat(uint16 *p, uint32 len) {
	uint8 r, g, b;

	for (int a = 0; a < len; a++) {
		uint16 t = p[a];
		RGBColor(t, &r, &g, &b);
		p[a] = _screenFormat.RGBToColor(r, g, b);
	}
}

/* -----------------12/06/97 18.25-------------------
				UnUpdatePixelFormat
 --------------------------------------------------*/
uint16 UnUpdatePixelFormat(uint16 t) {
	uint8 r, g, b;
	_screenFormat.colorToRGB(t, r, g, b);
	return (RGB2Color(r, g, b));
}

/*-----------------10/12/95 15.53-------------------
				CheckMask - Compreso
--------------------------------------------------*/
bool CheckMask(uint16 MX, uint16 MY) {
	for (int8 a = (MAXOBJINROOM - 1); a >= 0; a--) {
		uint16 checkedObj = g_vm->_room[g_vm->_curRoom]._object[a];

		if ((checkedObj) && (g_vm->_obj[checkedObj]._mode & (OBJMODE_OBJSTATUS))) {
			if ((MX >= g_vm->_obj[checkedObj]._lim[0]) &&
			    (MY >= g_vm->_obj[checkedObj]._lim[1] + TOP) &&
			    (MX <= g_vm->_obj[checkedObj]._lim[2]) &&
			    (MY <= g_vm->_obj[checkedObj]._lim[3] + TOP)) {

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
