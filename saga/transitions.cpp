/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:	
 
	Background transition routines

 Notes: 
*/

#include <stdio.h>
#include <stdlib.h>

#include "yslib.h"

#include "reinherit.h"

namespace Saga {

int
TRANSITION_Dissolve(byte * dst_img,
    int dst_w,
    int dst_h,
    int dst_p, const byte * src_img, int src_p, int flags, double percent)
{
#define XOR_MASK 0xB400;

	int pixelcount = dst_w * dst_h;
	int seqlimit = (int)(65535 * percent);

	int seq = 1;
	int i;

	YS_IGNORE_PARAM(flags);
	YS_IGNORE_PARAM(src_p);
	YS_IGNORE_PARAM(dst_p);

	for (i = 0; i < seqlimit; i++) {

		if (seq & 1) {
			seq = (seq >> 1) ^ XOR_MASK;
		} else {
			seq = seq >> 1;
		}

		if (seq == 1) {
			return 0;
		}

		if (seq >= pixelcount) {
			continue;
		} else {

			dst_img[seq] = src_img[seq];

		}
	}

	return 1;
}

} // End of namespace Saga

