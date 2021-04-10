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
#include "trecision/trecision.h"

namespace Trecision {

void byte2word(void *dest, void *src, void *data, uint32 len) {
	uint16 *d = (uint16 *)dest, *p = (uint16 *)data;
	uint8 *s = (uint8 *)src;
	for (uint32 i = 0; i < len; i++)
		*d++ = p[*s++];
}

#if (!USE_NEW_VIDEO_CODE)
void byte2wordm(void *dest, void *src, void *data, uint32 len) {
	uint16 *d = (uint16 *)dest, *p = (uint16 *)data;
	uint8 *s = (uint8 *)src;
	for (uint32 i = 0; i < len; i++) {
		uint8 v = *s++;
		if (v == 0)
			d++;
		else
			*d++ = p[v];
	}
}
#endif

void byte2wordn(void *dest, void *src, void *smk, void *pal, uint32 len) {
	uint16 *pDest = (uint16 *)dest;
	uint16 *pPal = (uint16 *)pal;
	uint16 *t = (uint16 *)smk;
	uint8 *pSrc = (uint8 *)src;

	for (uint32 i = 0; i < len; i++) {
		uint8 color = *pSrc++;
		if (color == 0)
			*pDest++ = *t++;
		else {
			*pDest++ = pPal[color];
			t++;
		}
	}
}

} // End of namespace Trecision
