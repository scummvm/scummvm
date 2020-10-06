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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/floors.h"
#include "engines/icb/debug.h"
#include "engines/icb/prim_route_builder.h"
#include "engines/icb/remora.h"
#include "engines/icb/global_objects.h"

#include "common/system.h"
#include "common/math.h"

namespace ICB {

uint32 pen;
uint32 timer;

void _prim_route_builder::Reset_barrier_list() {
	total_points = 0;
}

void _prim_route_builder::Add_barrier(_route_barrier *new_barrier) {
	barrier_list[total_points].x = new_barrier->x1();
	barrier_list[total_points++].z = new_barrier->z1();
	barrier_list[total_points].x = new_barrier->x2();
	barrier_list[total_points++].z = new_barrier->z2();

	if (!ExtrapolateLine(&barrier_list[total_points - 2], &barrier_list[total_points - 1], &barrier_list[total_points - 2], &barrier_list[total_points - 1], extrap_size))
		Fatal_error("extrapolate line failed on line %3.2f %3.2f  %3.2f %3.2f", new_barrier->x1(), new_barrier->z1(), new_barrier->x2(), new_barrier->z2());

	_ASSERT(total_points < MAX_barriers);
}

void _prim_route_builder::Give_barrier_list(_route_description *route) {
	// this may seem daft, but now we're giving the barriers back - for NETHACK diagnostics, not the logic
	// this wont be called in final .exe

	if (!total_points) {
		route->number_of_diag_bars = 0;
		return;
	}

	// Turn off the new's for PSX under certain conditions
	// i.e. a CD 2MB build
	// set passed number of points
	route->number_of_diag_bars = total_points;

	if (route->diag_bars)
		Message_box("multiple calls to Give_barrier_list");

	route->diag_bars = new _point[total_points];

	// move
	memcpy((unsigned char *)route->diag_bars, (unsigned char *)barrier_list, ((total_points) * sizeof(_point)));
}

void _prim_route_builder::Give_route(_route_description *route) {
	// copy the route to pass_to

	Zdebug("give route %d points", final_points + 1);

	for (uint32 j = 0; j < final_points + 1; j++)
		Zdebug(" %3.1f %3.1f", final_route[j].x, final_route[j].z);

	if (!final_points)
		Fatal_error("_prim_route_builder::Give_route no route to give!");

	// do a check for length exceeding MAX_final_route as this isnt really done anyway - it will have already scribbled of course but hey we're hanging on in there

	if (final_points + 1 >= MAX_final_route)
		Fatal_error("route too big");

	// pass number of points
	route->total_points = final_points + 1;

	//	routes start at point 1 as route creator gives our start position as point 0
	route->current_position = 1;

	memcpy((unsigned char *)route->prim_route, (unsigned char *)final_route, ((final_points + 1) * sizeof(_point)));

	final_points = 0; // effectively clear the route
}

_route_stat _prim_route_builder::Calc_route(PXreal startx, PXreal startz, PXreal endx, PXreal endz) {
	// total_points is the number of dots derives from lines - i.e lines*2
	// we then add 2 to this to get our total number of points

	// true_points == lines*2 +2 where lines*2 means a point at either end of each barrier +2 for start and end points

	// uses 'points' as waypoints plus points[0] is start and points[total_points+1] is end point
	// used 'lines' as list of barriers

	uint32 j, l, thisp, lastp, go;
	uint32 to;

	// Used for computing how to index into the bit based arrays
	uint32 jplace, jremainder;

	Zdebug("-*-calc route-*-");

	if (total_points + 2 >= MAX_barriers) {
		Fatal_error("too many barriers %d MAX %d", total_points + 2, MAX_barriers);
	}
	// insert start and end positions
	barrier_list[total_points].x = startx;
	barrier_list[total_points++].z = startz;
	barrier_list[total_points].x = endx;
	barrier_list[total_points++].z = endz;

	// reset the hit tables
	for (go = 0; go < total_points; go++) { // goes for total number of points excluding target
#if PRIM_BYTE_ARRAYS
		for (l = 0; l < total_points; l++) {
			hits[go][l] = 0;
			gohits[go][l] = 0;
		}
#endif
		for (l = 0; l < (total_points + 7) / 8; l++) {
			hitsBits[go][l] = 0;
			gohitsBits[go][l] = 0;
		}
	}

	// create the exclusion table
	//	exclude = new    uint32 [total_points];
	memset((unsigned char *)exclude, 0, (total_points) * sizeof(uint8)); // zero the table

	// create the temporary exclusion table
	//	temp_exclude = new   uint32 [total_points];
	memset((unsigned char *)temp_exclude, 0, (total_points) * sizeof(uint8)); // zero the table

	// firsly prime the system by seeing what gets to the target
	to = total_points - 1; // last point defined is target

	//	test all points against target 'to'
	for (j = 0; j < total_points - 1; j++) { //-1 means don't test the last point against itself
		                                 //		ok, point 'j' can be tested to see if can reach point 'to'

		//		test against all our lines
		for (l = 0; l < total_points - 2; l += 2) {
			//			dont test point J against the line it is derived from
			if (l != (j & 0xfffffffe)) {
				if (Get_intersect(/*firing line from*/ barrier_list[j].x, barrier_list[j].z, /*firing line to*/ barrier_list[to].x, barrier_list[to].z,
				                  /*barrier*/ barrier_list[l].x, barrier_list[l].z, barrier_list[l + 1].x, barrier_list[l + 1].z)) {
					break; // intersection found j does not get to to
				}
			}
		}

		//		if it gets there then mark in
		if (l == total_points - 2) {
			if (j == (total_points - 2)) { // target got there?
				Zdebug("straight line HIT!");
				final_points = 1; // end point is an off the end value (final_points+1)=dest

				//				insert start point
				final_route[0].x = startx;
				final_route[0].z = startz;
				//				insert end point
				final_route[1].x = endx;
				final_route[1].z = endz;

				return (__PRIM_ROUTE_OK);
			}
#if PRIM_BYTE_ARRAYS
			hits[to][j] = 1;
			gohits[0][j] = 1; // what hit in go 0
#endif
			exclude[j] = 1; // no int32er check this point

			// Set the bit arrays to 1
			jplace = j >> 3;
			jremainder = j & 0x7;
			hitsBits[to][jplace] |= (1 << jremainder);
			gohitsBits[0][jplace] |= (1 << jremainder);
		}
	}

	// test every thing that hit in go[last-go][-] against everything not excluded so far
	for (go = 1; go < total_points - 1; go++) { // goes for total number of points excluding the target
		// read along finding the points that connected up last go
		for (lastp = 0; lastp < total_points - 2; lastp++) { // never the start point or the end point
			uint32 flag;
			jplace = lastp >> 3;
			jremainder = lastp & 0x7;
			flag = gohitsBits[go - 1][jplace] & (1 << jremainder);

#if PRIM_BYTE_ARRAYS
			if ((flag != 0) != (gohits[go - 1][lastp] != 0)) {
				Fatal_error("bits i:%d j:%d p:%d r:%d flag:%d gohits:%d", go - 1, lastp, jplace, jremainder, flag, gohits[go - 1][lastp]);
			}
#endif
			if (flag != 0) { // find points that connected last go
				// now check all non excluded points against lastp - lastp being the point that connected last go
				for (thisp = 0; thisp < total_points - 1; thisp++) // all points to that point last go
					if (!exclude[thisp]) { // not excluded points
						// test point thisp to point lastp
						// test against all our lines
						for (l = 0; l < total_points - 2; l += 2) {
							// dont test point J against the line it is derived from
							if ((l != (thisp & 0xfffffffe)) && (l != (lastp & 0xfffffffe))) {
								if (Get_intersect(/*firing line*/ barrier_list[thisp].x, barrier_list[thisp].z, barrier_list[lastp].x,
								                  barrier_list[lastp].z,
								                  /*barrier*/ barrier_list[l].x, barrier_list[l].z, barrier_list[l + 1].x, barrier_list[l + 1].z)) {
									break; // intersection found thisp does not get to lastp
								}
							}
						}

						// if it gets there then mark in
						if (l == total_points - 2) {

#if PRIM_BYTE_ARRAYS
							hits[lastp][thisp] = 1; // hits[point-hit-last-go] [point-that's-just-connected-this-go]
							gohits[go][thisp] = 1; // this point hit this go
#endif

							// Set the bit arrays to 1
							jplace = thisp >> 3;
							jremainder = thisp & 0x7;
							hitsBits[lastp][jplace] |= (1 << jremainder);
							gohitsBits[go][jplace] |= (1 << jremainder);

							if (thisp == total_points - 2) { // the start point has connected so abandon the search for hits to lastp
								break; // quit the FOR(thisp loop
							}

							temp_exclude[thisp] = 1; // never exclude point 0
							// allow thisp to be tested against other points this cycle
						}
					}
			}
		}
		// update exclude list
		for (j = 0; j < total_points; j++) {
			if (temp_exclude[j]) {
				exclude[j] = 1; // update main exclude list
				temp_exclude[j] = 0; // reset for next cycle
			}
		}
	}

	// now work out what different routes we found - if any

	final_points = 0; // number of points in route
	final_len = REAL_MAX; // primer value - any route will be shorter than this

	timer = g_system->getMillis();

	pen = 15;

	Find_connects(total_points - 2, REAL_ZERO, 0);
	Tdebug("trout.txt", "\n\n\n\n\n\n\n\n\n\n");

	timer = g_system->getMillis() - timer;

	// paste in the destination
	final_route[final_points].x = endx;
	final_route[final_points].z = endz;

	if (!final_points)
		return (__PRIM_ROUTE_FAIL);

	return (__PRIM_ROUTE_OK);
}

// line length == PXsqrt ( (x-x2)*(x-x2) + (y-y2)*(y-y2) )
void _prim_route_builder::Find_connects(uint32 point, PXreal cur_len, uint32 level) {
	// what does 'point' get to?
	// hurray, reentrant code
	// when the target room is met we see if its the shortest route yet found - if so copy temp route to final in case its 'the one'

	uint32 j, k;
	PXreal branch_len;
	PXreal sub1;
	PXreal sub2;

	uint32 flag;
	// Used for computing how to index into the bit based arrays
	uint32 jplace, jremainder;

	temp_route[level] = (uint8)point;

	Tdebug("trout.txt", "l %d", level);

	for (j = 0; j < total_points; j++) {
		// test the bit array
		jplace = point >> 3;
		jremainder = point & 0x7;
		flag = hitsBits[j][jplace] & (1 << jremainder);
#if PRIM_BYTE_ARRAYS
		if ((flag != 0) != (hits[j][point] != 0)) {
			Fatal_error("bits i:%d j:%d p:%d r:%d flag:%d hits:%d", j, point, jplace, jremainder, flag, hits[j][point]);
		}

#endif
		if (flag != 0) { // we say - what points does point get to?
			sub1 = (PXreal)barrier_list[j].x - barrier_list[point].x;
			sub2 = (PXreal)barrier_list[j].z - barrier_list[point].z;

			// pythagoras no less

			// Don't need m_fabs as sqrt is always +ve
			branch_len = (PXfloat)PXsqrt((sub1 * sub1) + (sub2 * sub2));
			Tdebug("trout.txt", "branch %.1f", branch_len);

			if (j != total_points - 1) { // not end point then keep going
				Find_connects(j, cur_len + branch_len, level + 1);
			} else {
				pen += 20;
				Tdebug("trout.txt", "hit the target");
				if ((branch_len + cur_len) < final_len) { // point we got to is final point, so
					Tdebug("trout.txt", "%d shorter than %d", (int)(branch_len + cur_len), (int)final_len);
					final_len = branch_len + cur_len;
					final_points = level + 1;

					for (k = 0; k < level + 1; k++) {
						final_route[k].x = barrier_list[temp_route[k]].x;
						final_route[k].z = barrier_list[temp_route[k]].z;
					}
				} else
					Tdebug("trout.txt", "%d is int32er then %d", (int)(branch_len + cur_len), (int)final_len);
			}
		}
	}
}

uint32 _prim_route_builder::ExtrapolateLine(_point *pSrc0, _point *pSrc1, _point *pDst0, _point *pDst1, int32 d) {
	//	FUNCTION RETURNS FALSE IF IT CANNOT WORK OUT THE NEW POINTS!

	// pSrc0, pSrc1 ->  Source points!
	// pDst0, pDst1 ->   Destination points!

	_fpoint p0, p1;
	int swapped = 0;

	//  Make sure points are going from left to right!
	if (pSrc0->x < pSrc1->x) {
		p0.x = (PXdouble)(pSrc0->x);
		p0.z = (PXdouble)(pSrc0->z);
		p1.x = (PXdouble)(pSrc1->x);
		p1.z = (PXdouble)(pSrc1->z);
	} else {
		p1.x = (PXdouble)(pSrc0->x);
		p1.z = (PXdouble)(pSrc0->z);
		p0.x = (PXdouble)(pSrc1->x);
		p0.z = (PXdouble)(pSrc1->z);
		swapped = 1;
	}

	// Jake - for readability and to make sure we don't rely on the optimiser
	_fpoint delta;
	delta.x = (p1.x - p0.x);
	delta.z = (p1.z - p0.z);

	PXdouble val = delta.x * delta.x + delta.z * delta.z;
	// This is only trying to test for a zero length line
	if (val <= 0.0)
		return 0;
	PXdouble l = sqrt(val);
	PXdouble dx = (d * delta.x) / l;
	PXdouble dz = (d * delta.z) / l;

	if (swapped) {
		pDst0->x = (PXreal)(p1.x + dx);
		pDst0->z = (PXreal)(p1.z + dz);
		pDst1->x = (PXreal)(p0.x - dx);
		pDst1->z = (PXreal)(p0.z - dz);
	} else {
		pDst0->x = (PXreal)(p0.x - dx);
		pDst0->z = (PXreal)(p0.z - dz);
		pDst1->x = (PXreal)(p1.x + dx);
		pDst1->z = (PXreal)(p1.z + dz);
	}

	return 1;
}

PXfloat _prim_route_builder::m_fabs(PXfloat val) {
	if (val < FLOAT_ZERO)
		return FLOAT_ZERO - val;
	else
		return val;
}

int32 _prim_route_builder::Get_intersect(PXreal x0, PXreal y0, PXreal x1, PXreal y1, PXreal x2, PXreal y2, PXreal x3, PXreal y3) { // S2.1(19Mar96tw)
	PXfloat fAX, fBX, fCX, fAY, fBY, fCY;
	PXfloat fX1Low, fX1High, fY1Low, fY1High;
	PXfloat fD, fE, fF;
	// PXfloat  fNum, fOffset;

	// Work out some commonly used terms.
	fAX = x1 - x0;
	fBX = x2 - x3;

	// X bounding box test.
	if (fAX < FLOAT_ZERO) {
		fX1Low = x1;
		fX1High = x0;
	} else {
		fX1High = x1;
		fX1Low = x0;
	}

	if (fBX > FLOAT_ZERO) {
		if ((fX1High < x3) || (x2 < fX1Low))
			return 0;
	} else {
		if ((fX1High < x2) || (x3 < fX1Low))
			return 0;
	}

	// More common terms.
	fAY = y1 - y0;
	fBY = y2 - y3;

	// Y bounding box test.
	if (fAY < FLOAT_ZERO) {
		fY1Low = y1;
		fY1High = y0;
	} else {
		fY1High = y1;
		fY1Low = y0;
	}

	if (fBY > FLOAT_ZERO) {
		if ((fY1High < y3) || (y2 < fY1Low))
			return 0;
	} else {
		if ((fY1High < y2) || (y3 < fY1Low))
			return 0;
	}

	// Couldn't dismiss the lines on their bounding rectangles, so do a proper intersection.
	fCX = x0 - x2;
	fCY = y0 - y2;
	fD = (fBY * fCX) - (fBX * fCY);
	fF = (fAY * fBX) - (fAX * fBY);

	if (fF > FLOAT_ZERO) {
		if ((fD < FLOAT_ZERO) || (fD > fF))
			return 0;
	} else {
		if ((fD > FLOAT_ZERO) || (fD < fF))
			return 0;
	}

	fE = (fAX * fCY) - (fAY * fCX);

	if (fF > FLOAT_ZERO) {
		if ((fE < FLOAT_ZERO) || (fE > fF))
			return 0;
	} else {
		if ((fE > FLOAT_ZERO) || (fE < fF))
			return 0;
	}

	// At this point, we can say that the lines do intersect as int32 as they are not
	// colinear (colinear is indicated by fF == 0.0).
	if (fabs(fF - FLOAT_ZERO) < (FLT_MIN * 10.0f))
		return 0;

	return 1;
}

bool8 _prim_route_builder::LineIntersectsRect(DXrect oRect, int32 nX1, int32 nY1, int32 nX2, int32 nY2) const {
	return (g_oRemora->CohenSutherland(oRect, nX1, nY1, nX2, nY2, FALSE8));
}

} // End of namespace ICB
