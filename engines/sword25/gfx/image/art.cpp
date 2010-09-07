/* Libart_LGPL - library of basic graphic primitives
 * Copyright (C) 1998 Raph Levien
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* Various utility functions RLL finds useful. */

#include "art.h"

#ifdef HAVE_UINSTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdarg.h>

/**
 * art_die: Print the error message to stderr and exit with a return code of 1.
 * @fmt: The printf-style format for the error message.
 *
 * Used for dealing with severe errors.
 **/
void
art_die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * art_warn: Print the warning message to stderr.
 * @fmt: The printf-style format for the warning message.
 *
 * Used for generating warnings.
 **/
void
art_warn(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

/**
 * art_svp_free: Free an #ArtSVP structure.
 * @svp: #ArtSVP to free.
 *
 * Frees an #ArtSVP structure and all the segments in it.
 **/
void
art_svp_free(ArtSVP *svp) {
	int n_segs = svp->n_segs;
	int i;

	for (i = 0; i < n_segs; i++)
		free(svp->segs[i].points);
	free(svp);
}

#ifdef ART_USE_NEW_INTERSECTOR
#define EPSILON 0
#else
#define EPSILON 1e-6
#endif

/**
 * art_svp_seg_compare: Compare two segments of an svp.
 * @seg1: First segment to compare.
 * @seg2: Second segment to compare.
 *
 * Compares two segments of an svp. Return 1 if @seg2 is below or to the
 * right of @seg1, -1 otherwise.
 **/
int
art_svp_seg_compare(const void *s1, const void *s2) {
	const ArtSVPSeg *seg1 = (ArtSVPSeg *)s1;
	const ArtSVPSeg *seg2 = (ArtSVPSeg *)s2;

	if (seg1->points[0].y - EPSILON > seg2->points[0].y) return 1;
	else if (seg1->points[0].y + EPSILON < seg2->points[0].y) return -1;
	else if (seg1->points[0].x - EPSILON > seg2->points[0].x) return 1;
	else if (seg1->points[0].x + EPSILON < seg2->points[0].x) return -1;
	else if ((seg1->points[1].x - seg1->points[0].x) *
	         (seg2->points[1].y - seg2->points[0].y) -
	         (seg1->points[1].y - seg1->points[0].y) *
	         (seg2->points[1].x - seg2->points[0].x) > 0) return 1;
	else return -1;
}

/**
 * art_vpath_add_point: Add point to vpath.
 * @p_vpath: Where the pointer to the #ArtVpath structure is stored.
 * @pn_points: Pointer to the number of points in *@p_vpath.
 * @pn_points_max: Pointer to the number of points allocated.
 * @code: The pathcode for the new point.
 * @x: The X coordinate of the new point.
 * @y: The Y coordinate of the new point.
 *
 * Adds a new point to *@p_vpath, reallocating and updating *@p_vpath
 * and *@pn_points_max as necessary. *@pn_points is incremented.
 *
 * This routine always adds the point after all points already in the
 * vpath. Thus, it should be called in the order the points are
 * desired.
 **/
void
art_vpath_add_point(ArtVpath **p_vpath, int *pn_points, int *pn_points_max,
                    ArtPathcode code, double x, double y) {
	int i;

	i = (*pn_points)++;
	if (i == *pn_points_max)
		art_expand(*p_vpath, ArtVpath, *pn_points_max);
	(*p_vpath)[i].code = code;
	(*p_vpath)[i].x = x;
	(*p_vpath)[i].y = y;
}
