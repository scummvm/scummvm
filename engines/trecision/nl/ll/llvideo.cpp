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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

#include "graphics/pixelformat.h"

namespace Trecision {

// locals
extern bool _linearMode;
extern Graphics::PixelFormat ScreenFormat;

/*-----------------07/11/95 15.18-------------------
					ResetZB
--------------------------------------------------*/
void ResetZB(int x1, int y1, int x2, int y2) {
	if (x1 > x2)
		return;

	if (y1 > y2)
		return;

	if (((x2 - x1) * (y2 - y1) * 2) > ZBUFFERSIZE)
		warning("Warning: ZBuffer size %d!\n", (x2 - x1) * (y2 - y1) * 2);

	wordset(ZBuffer, 0x7FFF, (x2 - x1) * (y2 - y1));
}

/*-----------------10/12/95 15.49-------------------
	VCopy - Esegue copia veloce in Video a 32k
--------------------------------------------------*/
void VCopy(uint32 Sco, uint16 *Src, uint32 Len) {
	LockVideo();
	if ((Video == NULL) || (Len == 0))
		return ;

	if (_linearMode && ((VideoPitch == 0) || (VideoPitch == SCREENLEN * 2))) {
		MCopy(Video + Sco, Src, Len);
		return ;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 EndSco = Sco + Len;

	int32 y2 = EndSco / SCREENLEN;

	uint32 SrcSco = 0;

	uint32 CopyNow;
	if (Len > SCREENLEN - x1)
		CopyNow = SCREENLEN - x1;
	else
		CopyNow = Len;

	MCopy(Video + y1 * (VideoPitch / 2) + x1, Src + SrcSco, CopyNow);
	SrcSco += CopyNow;
	Len -= CopyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		CopyNow = SCREENLEN;
		MCopy(Video + i * (VideoPitch / 2), Src + SrcSco, CopyNow);
		SrcSco += CopyNow;
		Len -= CopyNow;
	}

	if (Len > 0) {
		CopyNow = Len;
		MCopy(Video + y2 * (VideoPitch / 2), Src + SrcSco, CopyNow);
		// Useless assignment, removed
		// SrcSco += CopyNow;
		// Len -= CopyNow;
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
	for (int a = 0; a < dy; a++)
		VCopy(px + (py + a)*VirtualPageLen + VideoScrollPageDx,
			  Video2 + px + (py + a)*CurRoomMaxX + CurScrollPageDx,
			  dx);

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
		p[a] = ScreenFormat.RGBToColor(r, g, b);
	}
}

/* -----------------12/06/97 18.25-------------------
				UnUpdatePixelFormat
 --------------------------------------------------*/
uint16 UnUpdatePixelFormat(uint16 t) {
	uint8 r, g, b;
	ScreenFormat.colorToRGB(t, r, g, b);
	return (RGB2Color(r, g, b));
}

/*-----------------10/12/95 15.53-------------------
				CheckMask - Compreso
--------------------------------------------------*/
LLBOOL CheckMask(uint16 MX, uint16 my) {
	for (int8 a = (MAXOBJINROOM - 1); a >= 0; a--) {

		uint16 CheckedObj = Room[_curRoom]._object[a];

		if ((CheckedObj) && (_obj[CheckedObj]._mode & (OBJMODE_OBJSTATUS))) {
			if ((MX >= _obj[CheckedObj]._lim[0]) &&
					(my >= _obj[CheckedObj]._lim[1] + TOP) &&
					(MX <= _obj[CheckedObj]._lim[2]) &&
					(my <= _obj[CheckedObj]._lim[3] + TOP)) {

				if (_obj[CheckedObj]._mode & OBJMODE_FULL) {
					_curObj = CheckedObj;
					return true;
				}

				if (_obj[CheckedObj]._mode & OBJMODE_LIM) {
					_curObj = CheckedObj;
					return true;
				}

				if (_obj[CheckedObj]._mode & OBJMODE_MASK) {
					uint8 *mask = MaskPointers[a];
					int16 d = _obj[CheckedObj]._px;

					for (int16 b = _obj[CheckedObj]._py; b < (_obj[CheckedObj]._py + _obj[CheckedObj]._dy); b++) {
						int16 c = 0;
						int16 e = 0;
						while (e < _obj[CheckedObj]._dx) {
							if (!c) {		// fuori oggetto
								if ((b + TOP) == my) {
									if ((MX >= (d + e)) && (MX < (d + e + *mask))) {
										_curObj = 0;
// cosi' non cerca piu' 				return false;
									}
								}

								e += *mask;
								mask++;
								c = 1;
							} else {			// dentro oggetto
								if ((b + TOP) == my) {
									if ((MX >= (d + e)) && (MX < (d + e + *mask))) {
										_curObj = CheckedObj;
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
	_curObj = 0;
	return false;
}

} // End of namespace Trecision
