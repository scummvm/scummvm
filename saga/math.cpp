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
 
    Math routines

 Notes: 
*/

#include <stdio.h>
#include <stdlib.h>

#include "reinherit.h"

namespace Saga {

int
MATH_HitTestPoly(R_POINT * points, unsigned int npoints, R_POINT test_point)
{
	int yflag0;
	int yflag1;
	int inside_flag = 0;
	unsigned int pt;

	R_POINT *vtx0 = &points[npoints - 1];
	R_POINT *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);

	for (pt = 0; pt < npoints; pt++, vtx1++) {

		yflag1 = (vtx1->y >= test_point.y);

		if (yflag0 != yflag1) {

			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y -
				    vtx1->y)) == yflag1) {

				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

} // End of namespace Saga
