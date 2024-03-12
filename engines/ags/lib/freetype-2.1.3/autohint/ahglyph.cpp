/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  ahglyph.c                                                              */
/*                                                                         */
/*    Routines used to load and analyze a given glyph before hinting       */
/*    (body).                                                              */
/*                                                                         */
/*  Copyright 2000-2001, 2002 Catharon Productions Inc.                    */
/*  Author: David Turner                                                   */
/*                                                                         */
/*  This file is part of the Catharon Typography Project and shall only    */
/*  be used, modified, and distributed under the terms of the Catharon     */
/*  Open Source License that should come with this file under the name     */
/*  `CatharonLicense.txt'.  By continuing to use, modify, or distribute    */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  Note that this license is compatible with the FreeType license.        */
/*                                                                         */
/***************************************************************************/

#include <ft2build.h>

#include "ags/lib/freetype-2.1.3/ftmemory.h"
#include "ags/lib/freetype-2.1.3/autohint/ahglyph.h"
#include "ags/lib/freetype-2.1.3/autohint/ahangles.h"
#include "ags/lib/freetype-2.1.3/autohint/ahglobal.h"
#include "ags/lib/freetype-2.1.3/autohint/ahtypes.h"

#include "common/debug.h"
// util.h for ABS
#include "common/util.h"


namespace AGS3 {
namespace FreeType213 {


void ah_dump_edges(AH_Outline outline) {
	AH_Edge edges;
	AH_Edge edge_limit;
	FT_Int dimension;

	edges = outline->horz_edges;
	edge_limit = edges + outline->num_hedges;

	for (dimension = 1; dimension >= 0; dimension--) {
		AH_Edge edge;

		debug(6, "Table of %s edges:", !dimension ? "vertical" : "horizontal");
		debug(6, "  [ index |  pos |  dir  | link |"
			   " serif | blue | opos  |  pos  ]");

		for (edge = edges; edge < edge_limit; edge++) {
			debug(6, "  [ %5d | %4d | %5s | %4d | %5d |  %c  | %5.2f | %5.2f ]",
				   (int)(edge - edges),
				   (int)edge->fpos,
				   edge->dir == AH_DIR_UP
					   ? "up"
					   : (edge->dir == AH_DIR_DOWN
							  ? "down"
							  : (edge->dir == AH_DIR_LEFT
									 ? "left"
									 : (edge->dir == AH_DIR_RIGHT
											? "right"
											: "none"))),
				   edge->link ? (int)(edge->link - edges) : -1,
				   edge->serif ? (int)(edge->serif - edges) : -1,
				   edge->blue_edge ? 'y' : 'n',
				   edge->opos / 64.0,
				   edge->pos / 64.0);
		}

		edges = outline->vert_edges;
		edge_limit = edges + outline->num_vedges;
	}
}

/* A function used to dump the array of linked segments */
void ah_dump_segments(AH_Outline outline) {
	AH_Segment segments;
	AH_Segment segment_limit;
	AH_Point points;
	FT_Int dimension;

	points = outline->points;
	segments = outline->horz_segments;
	segment_limit = segments + outline->num_hsegments;

	for (dimension = 1; dimension >= 0; dimension--) {
		AH_Segment seg;

		debug(6, "Table of %s segments:",
			   !dimension ? "vertical" : "horizontal");
		debug(6, "  [ index |  pos |  dir  | link | serif |"
			   " numl | first | start ]");

		for (seg = segments; seg < segment_limit; seg++) {
			debug(6, "  [ %5d | %4d | %5s | %4d | %5d | %4d | %5d | %5d ]",
				   (int)(seg - segments),
				   (int)seg->pos,
				   seg->dir == AH_DIR_UP
					   ? "up"
					   : (seg->dir == AH_DIR_DOWN
							  ? "down"
							  : (seg->dir == AH_DIR_LEFT
									 ? "left"
									 : (seg->dir == AH_DIR_RIGHT
											? "right"
											: "none"))),
				   seg->link ? (int)(seg->link - segments) : -1,
				   seg->serif ? (int)(seg->serif - segments) : -1,
				   (int)seg->num_linked,
				   (int)(seg->first - points),
				   (int)(seg->last - points));
		}

		segments = outline->vert_segments;
		segment_limit = segments + outline->num_vsegments;
	}
}


/* compute the direction value of a given vector.. */
static AH_Direction ah_compute_direction(FT_Pos dx, FT_Pos dy) {
	AH_Direction dir;
	FT_Pos ax = ABS(dx);
	FT_Pos ay = ABS(dy);

	dir = AH_DIR_NONE;

	/* test for vertical direction */
	if (ax * 12 < ay) {
		dir = dy > 0 ? AH_DIR_UP : AH_DIR_DOWN;
	}
	/* test for horizontal direction */
	else if (ay * 12 < ax) {
		dir = dx > 0 ? AH_DIR_RIGHT : AH_DIR_LEFT;
	}

	return dir;
}

/* this function is used by ah_get_orientation (see below) to test */
/* the fill direction of a given bbox extrema                      */
static FT_Int ah_test_extrema(FT_Outline *outline, FT_Int n) {
	FT_Vector *prev, *cur, *next;
	FT_Pos product;
	FT_Int first, last, c;
	FT_Int retval;

	/* we need to compute the `previous' and `next' point */
	/* for these extrema                                  */
	cur = outline->points + n;
	prev = cur - 1;
	next = cur + 1;

	first = 0;
	for (c = 0; c < outline->n_contours; c++) {
		last = outline->contours[c];

		if (n == first)
			prev = outline->points + last;

		if (n == last)
			next = outline->points + first;

		first = last + 1;
	}

	product = FT_MulDiv(cur->x - prev->x, /* in.x  */
						next->y - cur->y, /* out.y */
						0x40) -
			  FT_MulDiv(cur->y - prev->y, /* in.y  */
						next->x - cur->x, /* out.x */
						0x40);

	retval = 0;
	if (product)
		retval = product > 0 ? 2 : 1;

	return retval;
}

/* Compute the orientation of path filling.  It differs between TrueType */
/* and Type1 formats                                                     */
static FT_Int ah_get_orientation(FT_Outline *outline) {
	FT_BBox box;
	FT_Int indices_xMin, indices_yMin, indices_xMax, indices_yMax;
	FT_Int n, last;

	indices_xMin = -1;
	indices_yMin = -1;
	indices_xMax = -1;
	indices_yMax = -1;

	box.xMin = box.yMin = 32767L;
	box.xMax = box.yMax = -32768L;

	/* is it empty? */
	if (outline->n_contours < 1)
		return 1;

	last = outline->contours[outline->n_contours - 1];

	for (n = 0; n <= last; n++) {
		FT_Pos x, y;

		x = outline->points[n].x;
		if (x < box.xMin) {
			box.xMin = x;
			indices_xMin = n;
		}
		if (x > box.xMax) {
			box.xMax = x;
			indices_xMax = n;
		}

		y = outline->points[n].y;
		if (y < box.yMin) {
			box.yMin = y;
			indices_yMin = n;
		}
		if (y > box.yMax) {
			box.yMax = y;
			indices_yMax = n;
		}
	}

	/* test orientation of the xmin */
	n = ah_test_extrema(outline, indices_xMin);
	if (n)
		goto Exit;

	n = ah_test_extrema(outline, indices_yMin);
	if (n)
		goto Exit;

	n = ah_test_extrema(outline, indices_xMax);
	if (n)
		goto Exit;

	n = ah_test_extrema(outline, indices_yMax);
	if (!n)
		n = 1;

Exit:
	return n;
}

FT_Error ah_outline_new(FT_Memory memory, AH_Outline *aoutline) {
	FT_Error error;
	AH_Outline outline;

	if (!FT_NEW(outline)) {
		outline->memory = memory;
		*aoutline = outline;
	}

	return error;
}

void ah_outline_done(AH_Outline outline) {
	FT_Memory memory = outline->memory;

	FT_FREE(outline->horz_edges);
	FT_FREE(outline->horz_segments);
	FT_FREE(outline->contours);
	FT_FREE(outline->points);

	FT_FREE(outline);
}

void ah_outline_save(AH_Outline outline, AH_Loader gloader) {
	AH_Point point = outline->points;
	AH_Point point_limit = point + outline->num_points;
	FT_Vector *vec = gloader->current.outline.points;
	char *tag = gloader->current.outline.tags;

	/* we assume that the glyph loader has already been checked for storage */
	for (; point < point_limit; point++, vec++, tag++) {
		vec->x = point->x;
		vec->y = point->y;

		if (point->flags & AH_FLAG_CONIC)
			tag[0] = FT_CURVE_TAG_CONIC;
		else if (point->flags & AH_FLAG_CUBIC)
			tag[0] = FT_CURVE_TAG_CUBIC;
		else
			tag[0] = FT_CURVE_TAG_ON;
	}
}

FT_Error ah_outline_load(AH_Outline outline, FT_Face face) {
	FT_Memory memory = outline->memory;
	FT_Error error = FT_Err_Ok;
	FT_Outline *source = &face->glyph->outline;
	FT_Int num_points = source->n_points;
	FT_Int num_contours = source->n_contours;
	AH_Point points;

	/* check arguments */
	if (!face ||
		!face->size ||
		face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		return FT_Err_Invalid_Argument;

	/* first of all, reallocate the contours array if necessary */
	if (num_contours > outline->max_contours) {
		FT_Int new_contours = (num_contours + 3) & -4;

		if (FT_RENEW_ARRAY(outline->contours,
								outline->max_contours,
								new_contours))
			goto Exit;

		outline->max_contours = new_contours;
	}

	/* then, reallocate the points, segments & edges arrays if needed -- */
	/* note that we reserved two additional point positions, used to     */
	/* hint metrics appropriately                                        */
	/*                                                                   */
	if (num_points + 2 > outline->max_points) {
		FT_Int news = (num_points + 2 + 7) & -8;
		FT_Int max = outline->max_points;

		if (FT_RENEW_ARRAY(outline->points, max, news) ||
			FT_RENEW_ARRAY(outline->horz_edges, max * 2, news * 2) ||
			FT_RENEW_ARRAY(outline->horz_segments, max * 2, news * 2))
			goto Exit;

		/* readjust some pointers */
		outline->vert_edges = outline->horz_edges + news;
		outline->vert_segments = outline->horz_segments + news;
		outline->max_points = news;
	}

	outline->num_points = num_points;
	outline->num_contours = num_contours;

	outline->num_hedges = 0;
	outline->num_vedges = 0;
	outline->num_hsegments = 0;
	outline->num_vsegments = 0;

	/* We can't rely on the value of `FT_Outline.flags' to know the fill  */
	/* direction used for a glyph, given that some fonts are broken (e.g. */
	/* the Arphic ones). We thus recompute it each time we need to.       */
	/*                                                                    */
	outline->vert_major_dir = AH_DIR_UP;
	outline->horz_major_dir = AH_DIR_LEFT;

	if (ah_get_orientation(source) > 1) {
		outline->vert_major_dir = AH_DIR_DOWN;
		outline->horz_major_dir = AH_DIR_RIGHT;
	}

	outline->x_scale = face->size->metrics.x_scale;
	outline->y_scale = face->size->metrics.y_scale;

	points = outline->points;
	if (outline->num_points == 0)
		goto Exit;

	{
		/* do one thing at a time -- it is easier to understand, and */
		/* the code is clearer                                       */
		AH_Point point;
		AH_Point point_limit = points + outline->num_points;

		/* compute coordinates */
		{
			FT_Vector *vec = source->points;
			FT_Fixed x_scale = outline->x_scale;
			FT_Fixed y_scale = outline->y_scale;

			for (point = points; point < point_limit; vec++, point++) {
				point->fx = vec->x;
				point->fy = vec->y;
				point->ox = point->x = FT_MulFix(vec->x, x_scale);
				point->oy = point->y = FT_MulFix(vec->y, y_scale);

				point->flags = 0;
			}
		}

		/* compute Bezier flags */
		{
			char *tag = source->tags;

			for (point = points; point < point_limit; point++, tag++) {
				switch (FT_CURVE_TAG(*tag)) {
				case FT_CURVE_TAG_CONIC:
					point->flags = AH_FLAG_CONIC;
					break;
				case FT_CURVE_TAG_CUBIC:
					point->flags = AH_FLAG_CUBIC;
					break;
				default:;
				}
			}
		}

		/* compute `next' and `prev' */
		{
			FT_Int contour_index;
			AH_Point prev;
			AH_Point first;
			AH_Point end;

			contour_index = 0;

			first = points;
			end = points + source->contours[0];
			prev = end;

			for (point = points; point < point_limit; point++) {
				point->prev = prev;
				if (point < end) {
					point->next = point + 1;
					prev = point;
				} else {
					point->next = first;
					contour_index++;
					if (point + 1 < point_limit) {
						end = points + source->contours[contour_index];
						first = point + 1;
						prev = end;
					}
				}
			}
		}

		/* set-up the contours array */
		{
			AH_Point *contour = outline->contours;
			AH_Point *contour_limit = contour + outline->num_contours;
			short *end = source->contours;
			short idx = 0;

			for (; contour < contour_limit; contour++, end++) {
				contour[0] = points + idx;
				idx = (short)(end[0] + 1);
			}
		}

		/* compute directions of in & out vectors */
		{
			for (point = points; point < point_limit; point++) {
				AH_Point prev;
				AH_Point next;
				FT_Vector ivec, ovec;

				prev = point->prev;
				ivec.x = point->fx - prev->fx;
				ivec.y = point->fy - prev->fy;

				point->in_dir = ah_compute_direction(ivec.x, ivec.y);

				next = point->next;
				ovec.x = next->fx - point->fx;
				ovec.y = next->fy - point->fy;

				point->out_dir = ah_compute_direction(ovec.x, ovec.y);

#ifndef AH_OPTION_NO_WEAK_INTERPOLATION
				if (point->flags & (AH_FLAG_CONIC | AH_FLAG_CUBIC)) {
				Is_Weak_Point:
					point->flags |= AH_FLAG_WEAK_INTERPOLATION;
				} else if (point->out_dir == point->in_dir) {
					AH_Angle angle_in, angle_out, delta;

					if (point->out_dir != AH_DIR_NONE)
						goto Is_Weak_Point;

					angle_in = ah_angle(&ivec);
					angle_out = ah_angle(&ovec);
					delta = angle_in - angle_out;

					if (delta > AH_PI)
						delta = AH_2PI - delta;

					if (delta < 0)
						delta = -delta;

					if (delta < 2)
						goto Is_Weak_Point;
				} else if (point->in_dir == -point->out_dir)
					goto Is_Weak_Point;
#endif
			}
		}
	}

Exit:
	return error;
}

void ah_setup_uv(AH_Outline outline, AH_UV source) {
	AH_Point point = outline->points;
	AH_Point point_limit = point + outline->num_points;

	for (; point < point_limit; point++) {
		FT_Pos u, v;

		switch (source) {
		case AH_UV_FXY:
			u = point->fx;
			v = point->fy;
			break;
		case AH_UV_FYX:
			u = point->fy;
			v = point->fx;
			break;
		case AH_UV_OXY:
			u = point->ox;
			v = point->oy;
			break;
		case AH_UV_OYX:
			u = point->oy;
			v = point->ox;
			break;
		case AH_UV_YX:
			u = point->y;
			v = point->x;
			break;
		case AH_UV_OX:
			u = point->x;
			v = point->ox;
			break;
		case AH_UV_OY:
			u = point->y;
			v = point->oy;
			break;
		default:
			u = point->x;
			v = point->y;
			break;
		}
		point->u = u;
		point->v = v;
	}
}

/* compute all inflex points in a given glyph */
static void ah_outline_compute_inflections(AH_Outline outline) {
	AH_Point *contour = outline->contours;
	AH_Point *contour_limit = contour + outline->num_contours;

	/* load original coordinates in (u,v) */
	ah_setup_uv(outline, AH_UV_FXY);

	/* do each contour separately */
	for (; contour < contour_limit; contour++) {
		FT_Vector vec;
		AH_Point point = contour[0];
		AH_Point first = point;
		AH_Point start = point;
		AH_Point end = point;
		AH_Point before;
		AH_Point after;
		AH_Angle angle_in, angle_seg, angle_out;
		AH_Angle diff_in, diff_out;
		FT_Int finished = 0;

		/* compute first segment in contour */
		first = point;

		start = end = first;
		do {
			end = end->next;
			if (end == first)
				goto Skip;

		} while (end->u == first->u && end->v == first->v);

		vec.x = end->u - start->u;
		vec.y = end->v - start->v;
		angle_seg = ah_angle(&vec);

		/* extend the segment start whenever possible */
		before = start;
		do {
			do {
				start = before;
				before = before->prev;
				if (before == first)
					goto Skip;

			} while (before->u == start->u && before->v == start->v);

			vec.x = start->u - before->u;
			vec.y = start->v - before->v;
			angle_in = ah_angle(&vec);

		} while (angle_in == angle_seg);

		first = start;
		diff_in = ah_angle_diff(angle_in, angle_seg);

		/* now, process all segments in the contour */
		do {
			/* first, extend current segment's end whenever possible */
			after = end;
			do {
				do {
					end = after;
					after = after->next;
					if (after == first)
						finished = 1;

				} while (end->u == after->u && end->v == after->v);

				vec.x = after->u - end->u;
				vec.y = after->v - end->v;
				angle_out = ah_angle(&vec);

			} while (angle_out == angle_seg);

			diff_out = ah_angle_diff(angle_seg, angle_out);

			if ((diff_in ^ diff_out) < 0) {
				/* diff_in and diff_out have different signs, we have */
				/* inflection points here...                          */

				do {
					start->flags |= AH_FLAG_INFLECTION;
					start = start->next;

				} while (start != end);

				start->flags |= AH_FLAG_INFLECTION;
			}

			start = end;
			end = after;
			angle_seg = angle_out;
			diff_in = diff_out;

		} while (!finished);

	Skip:;
	}
}

void ah_outline_compute_segments(AH_Outline outline) {
	int dimension;
	AH_Segment segments;
	FT_Int *p_num_segments;
	AH_Direction segment_dir;
	AH_Direction major_dir;

	segments = outline->horz_segments;
	p_num_segments = &outline->num_hsegments;
	major_dir = AH_DIR_RIGHT; /* This value must be positive! */
	segment_dir = major_dir;

	/* set up (u,v) in each point */
	ah_setup_uv(outline, AH_UV_FYX);

	for (dimension = 1; dimension >= 0; dimension--) {
		AH_Point *contour = outline->contours;
		AH_Point *contour_limit = contour + outline->num_contours;
		AH_Segment segment = segments;
		FT_Int num_segments = 0;

#ifdef AH_HINT_METRICS
		AH_Point min_point = 0;
		AH_Point max_point = 0;
		FT_Pos min_coord = 32000;
		FT_Pos max_coord = -32000;
#endif

		/* do each contour separately */
		for (; contour < contour_limit; contour++) {
			AH_Point point = contour[0];
			AH_Point last = point->prev;
			int on_edge = 0;
			FT_Pos min_pos = +32000; /* minimum segment pos != min_coord */
			FT_Pos max_pos = -32000; /* maximum segment pos != max_coord */
			FT_Bool passed;

#ifdef AH_HINT_METRICS
			if (point->u < min_coord) {
				min_coord = point->u;
				min_point = point;
			}
			if (point->u > max_coord) {
				max_coord = point->u;
				max_point = point;
			}
#endif

			if (point == last) /* skip singletons -- just in case? */
				continue;

			if (ABS(last->out_dir) == major_dir &&
				ABS(point->out_dir) == major_dir) {
				/* we are already on an edge, try to locate its start */
				last = point;

				for (;;) {
					point = point->prev;
					if (ABS(point->out_dir) != major_dir) {
						point = point->next;
						break;
					}
					if (point == last)
						break;
				}
			}

			last = point;
			passed = 0;

			for (;;) {
				FT_Pos u, v;

				if (on_edge) {
					u = point->u;
					if (u < min_pos)
						min_pos = u;
					if (u > max_pos)
						max_pos = u;

					if (point->out_dir != segment_dir || point == last) {
						/* we are just leaving an edge; record a new segment! */
						segment->last = point;
						segment->pos = (min_pos + max_pos) >> 1;

						/* a segment is round if either its first or last point */
						/* is a control point                                   */
						if ((segment->first->flags | point->flags) &
							AH_FLAG_CONTROL)
							segment->flags |= AH_EDGE_ROUND;

						/* compute segment size */
						min_pos = max_pos = point->v;

						v = segment->first->v;
						if (v < min_pos)
							min_pos = v;
						if (v > max_pos)
							max_pos = v;

						segment->min_coord = min_pos;
						segment->max_coord = max_pos;

						on_edge = 0;
						num_segments++;
						segment++;
						/* fallthrough */
					}
				}

				/* now exit if we are at the start/end point */
				if (point == last) {
					if (passed)
						break;
					passed = 1;
				}

				if (!on_edge && ABS(point->out_dir) == major_dir) {
					/* this is the start of a new segment! */
					segment_dir = point->out_dir;

					/* clear all segment fields */
					FT_ZERO(segment);

					segment->dir = segment_dir;
					segment->flags = AH_EDGE_NORMAL;
					min_pos = max_pos = point->u;
					segment->first = point;
					segment->last = point;
					segment->contour = contour;
					on_edge = 1;

#ifdef AH_HINT_METRICS
					if (point == max_point)
						max_point = 0;

					if (point == min_point)
						min_point = 0;
#endif
				}

				point = point->next;
			}

		} /* contours */

#ifdef AH_HINT_METRICS
		/* we need to ensure that there are edges on the left-most and  */
		/* right-most points of the glyph in order to hint the metrics; */
		/* we do this by inserting fake segments when needed            */
		if (dimension == 0) {
			AH_Point point = outline->points;
			AH_Point point_limit = point + outline->num_points;

			FT_Pos min_pos = 32000;
			FT_Pos max_pos = -32000;

			min_point = 0;
			max_point = 0;

			/* compute minimum and maximum points */
			for (; point < point_limit; point++) {
				FT_Pos x = point->fx;

				if (x < min_pos) {
					min_pos = x;
					min_point = point;
				}
				if (x > max_pos) {
					max_pos = x;
					max_point = point;
				}
			}

			/* insert minimum segment */
			if (min_point) {
				/* clear all segment fields */
				FT_ZERO(segment);

				segment->dir = segment_dir;
				segment->flags = AH_EDGE_NORMAL;
				segment->first = min_point;
				segment->last = min_point;
				segment->pos = min_pos;

				num_segments++;
				segment++;
			}

			/* insert maximum segment */
			if (max_point) {
				/* clear all segment fields */
				FT_ZERO(segment);

				segment->dir = segment_dir;
				segment->flags = AH_EDGE_NORMAL;
				segment->first = max_point;
				segment->last = max_point;
				segment->pos = max_pos;

				num_segments++;
				segment++;
			}
		}
#endif /* AH_HINT_METRICS */

		*p_num_segments = num_segments;

		segments = outline->vert_segments;
		major_dir = AH_DIR_UP;
		p_num_segments = &outline->num_vsegments;
		ah_setup_uv(outline, AH_UV_FXY);
	}
}

void ah_outline_link_segments(AH_Outline outline) {
	AH_Segment segments;
	AH_Segment segment_limit;
	int dimension;

	ah_setup_uv(outline, AH_UV_FYX);

	segments = outline->horz_segments;
	segment_limit = segments + outline->num_hsegments;

	for (dimension = 1; dimension >= 0; dimension--) {
		AH_Segment seg1;
		AH_Segment seg2;

		/* now compare each segment to the others */
		for (seg1 = segments; seg1 < segment_limit; seg1++) {
			FT_Pos best_score;
			AH_Segment best_segment;

			/* the fake segments are introduced to hint the metrics -- */
			/* we must never link them to anything                     */
			if (seg1->first == seg1->last)
				continue;

			best_segment = seg1->link;
			if (best_segment)
				best_score = seg1->score;
			else
				best_score = 32000;

			for (seg2 = segments; seg2 < segment_limit; seg2++)
				if (seg1 != seg2 && seg1->dir + seg2->dir == 0) {
					FT_Pos pos1 = seg1->pos;
					FT_Pos pos2 = seg2->pos;
					FT_Bool is_dir;
					FT_Bool is_pos;

					/* check that the segments are correctly oriented and */
					/* positioned to form a black distance                */

					is_dir = (FT_Bool)(seg1->dir == outline->horz_major_dir ||
									   seg1->dir == outline->vert_major_dir);
					is_pos = (FT_Bool)(pos1 > pos2);

					if (pos1 == pos2 || !(is_dir ^ is_pos))
						continue;

					{
						FT_Pos min = seg1->min_coord;
						FT_Pos max = seg1->max_coord;
						FT_Pos len, dist, score;

						if (min < seg2->min_coord)
							min = seg2->min_coord;

						if (max > seg2->max_coord)
							max = seg2->max_coord;

						len = max - min;
						if (len >= 8) {
							dist = seg2->pos - seg1->pos;
							if (dist < 0)
								dist = -dist;

							score = dist + 3000 / len;

							if (score < best_score) {
								best_score = score;
								best_segment = seg2;
							}
						}
					}
				}

			if (best_segment) {
				seg1->link = best_segment;
				seg1->score = best_score;

				best_segment->num_linked++;
			}

		} /* edges 1 */

		/* now, compute the `serif' segments */
		for (seg1 = segments; seg1 < segment_limit; seg1++) {
			seg2 = seg1->link;

			if (seg2 && seg2->link != seg1) {
				seg1->link = 0;
				seg1->serif = seg2->link;
			}
		}

		ah_setup_uv(outline, AH_UV_FXY);

		segments = outline->vert_segments;
		segment_limit = segments + outline->num_vsegments;
	}
}

static void ah_outline_compute_edges(AH_Outline outline) {
	AH_Edge edges;
	AH_Segment segments;
	AH_Segment segment_limit;
	AH_Direction up_dir;
	FT_Int *p_num_edges;
	FT_Int dimension;
	FT_Fixed scale;
	FT_Pos edge_distance_threshold;

	edges = outline->horz_edges;
	segments = outline->horz_segments;
	segment_limit = segments + outline->num_hsegments;
	p_num_edges = &outline->num_hedges;
	up_dir = AH_DIR_RIGHT;
	scale = outline->y_scale;

	for (dimension = 1; dimension >= 0; dimension--) {
		AH_Edge edge;
		AH_Edge edge_limit; /* really == edge + num_edges */
		AH_Segment seg;

		/*********************************************************************/
		/*                                                                   */
		/* We will begin by generating a sorted table of edges for the       */
		/* current direction.  To do so, we simply scan each segment and try */
		/* to find an edge in our table that corresponds to its position.    */
		/*                                                                   */
		/* If no edge is found, we create and insert a new edge in the       */
		/* sorted table.  Otherwise, we simply add the segment to the edge's */
		/* list which will be processed in the second step to compute the    */
		/* edge's properties.                                                */
		/*                                                                   */
		/* Note that the edges table is sorted along the segment/edge        */
		/* position.                                                         */
		/*                                                                   */
		/*********************************************************************/

		edge_distance_threshold = FT_MulFix(outline->edge_distance_threshold, scale);
		if (edge_distance_threshold > 64 / 4)
			edge_distance_threshold = 64 / 4;

		edge_limit = edges;
		for (seg = segments; seg < segment_limit; seg++) {
			AH_Edge found = 0;

			/* look for an edge corresponding to the segment */
			for (edge = edges; edge < edge_limit; edge++) {
				FT_Pos dist;

				dist = seg->pos - edge->fpos;
				if (dist < 0)
					dist = -dist;

				dist = FT_MulFix(dist, scale);
				if (dist < edge_distance_threshold) {
					found = edge;
					break;
				}
			}

			if (!found) {
				/* insert a new edge in the list and */
				/* sort according to the position    */
				while (edge > edges && edge[-1].fpos > seg->pos) {
					edge[0] = edge[-1];
					edge--;
				}
				edge_limit++;

				/* clear all edge fields */
				FT_MEM_ZERO(edge, sizeof(*edge));

				/* add the segment to the new edge's list */
				edge->first = seg;
				edge->last = seg;
				edge->fpos = seg->pos;
				edge->opos = edge->pos = FT_MulFix(seg->pos, scale);
				seg->edge_next = seg;
			} else {
				/* if an edge was found, simply add the segment to the edge's */
				/* list                                                       */
				seg->edge_next = edge->first;
				edge->last->edge_next = seg;
				edge->last = seg;
			}
		}

		*p_num_edges = (FT_Int)(edge_limit - edges);

		/*********************************************************************/
		/*                                                                   */
		/* Good, we will now compute each edge's properties according to     */
		/* segments found on its position.  Basically, these are:            */
		/*                                                                   */
		/*  - edge's main direction                                          */
		/*  - stem edge, serif edge or both (which defaults to stem then)    */
		/*  - rounded edge, straigth or both (which defaults to straight)    */
		/*  - link for edge                                                  */
		/*                                                                   */
		/*********************************************************************/

		/* first of all, set the `edge' field in each segment -- this is */
		/* required in order to compute edge links                       */
		for (edge = edges; edge < edge_limit; edge++) {
			seg = edge->first;
			if (seg)
				do {
					seg->edge = edge;
					seg = seg->edge_next;
				} while (seg != edge->first);
		}

		/* now, compute each edge properties */
		for (edge = edges; edge < edge_limit; edge++) {
			FT_Int is_round = 0;    /* does it contain round segments?    */
			FT_Int is_straight = 0; /* does it contain straight segments? */
			FT_Pos ups = 0;         /* number of upwards segments         */
			FT_Pos downs = 0;       /* number of downwards segments       */

			seg = edge->first;

			do {
				FT_Bool is_serif;

				/* check for roundness of segment */
				if (seg->flags & AH_EDGE_ROUND)
					is_round++;
				else
					is_straight++;

				/* check for segment direction */
				if (seg->dir == up_dir)
					ups += seg->max_coord - seg->min_coord;
				else
					downs += seg->max_coord - seg->min_coord;

				/* check for links -- if seg->serif is set, then seg->link must */
				/* be ignored                                                   */
				is_serif = (FT_Bool)(seg->serif && seg->serif->edge != edge);

				if (seg->link || is_serif) {
					AH_Edge edge2;
					AH_Segment seg2;

					edge2 = edge->link;
					seg2 = seg->link;

					if (is_serif) {
						seg2 = seg->serif;
						edge2 = edge->serif;
					}

					if (edge2) {
						FT_Pos edge_delta;
						FT_Pos seg_delta;

						edge_delta = edge->fpos - edge2->fpos;
						if (edge_delta < 0)
							edge_delta = -edge_delta;

						seg_delta = seg->pos - seg2->pos;
						if (seg_delta < 0)
							seg_delta = -seg_delta;

						if (seg_delta < edge_delta)
							edge2 = seg2->edge;
					} else
						edge2 = seg2->edge;

					if (is_serif)
						edge->serif = edge2;
					else
						edge->link = edge2;
				}

				seg = seg->edge_next;

			} while (seg != edge->first);

			/* set the round/straight flags */
			edge->flags = AH_EDGE_NORMAL;

			if (is_round > 0 && is_round >= is_straight)
				edge->flags |= AH_EDGE_ROUND;

			/* set the edge's main direction */
			edge->dir = AH_DIR_NONE;

			if (ups > downs)
				edge->dir = up_dir;

			else if (ups < downs)
				edge->dir = -up_dir;

			else if (ups == downs)
				edge->dir = 0; /* both up and down !! */

			/* gets rid of serifs if link is set                */
			/* XXX: This gets rid of many unpleasant artefacts! */
			/*      Example: the `c' in cour.pfa at size 13     */

			if (edge->serif && edge->link)
				edge->serif = 0;
		}

		edges = outline->vert_edges;
		segments = outline->vert_segments;
		segment_limit = segments + outline->num_vsegments;
		p_num_edges = &outline->num_vedges;
		up_dir = AH_DIR_UP;
		scale = outline->x_scale;
	}
}

void ah_outline_detect_features(AH_Outline outline) {
	ah_outline_compute_segments(outline);
	ah_outline_link_segments(outline);
	ah_outline_compute_edges(outline);
	ah_outline_compute_inflections(outline);
}

void ah_outline_compute_blue_edges(AH_Outline outline, AH_Face_Globals face_globals) {
	AH_Edge edge = outline->horz_edges;
	AH_Edge edge_limit = edge + outline->num_hedges;
	AH_Globals globals = &face_globals->design;
	FT_Fixed y_scale = outline->y_scale;

	FT_Bool blue_active[AH_BLUE_MAX];

	/* compute which blue zones are active, i.e. have their scaled */
	/* size < 3/4 pixels                                           */
	{
		AH_Blue blue;
		FT_Bool check = 0;

		for (blue = AH_BLUE_CAPITAL_TOP; blue < AH_BLUE_MAX; blue++) {
			FT_Pos ref, shoot, dist;

			ref = globals->blue_refs[blue];
			shoot = globals->blue_shoots[blue];
			dist = ref - shoot;
			if (dist < 0)
				dist = -dist;

			blue_active[blue] = 0;

			if (FT_MulFix(dist, y_scale) < 48) {
				blue_active[blue] = 1;
				check = 1;
			}
		}

		/* return immediately if no blue zone is active */
		if (!check)
			return;
	}

	/* compute for each horizontal edge, which blue zone is closer */
	for (; edge < edge_limit; edge++) {
		AH_Blue blue;
		FT_Pos *best_blue = 0;
		FT_Pos best_dist; /* initial threshold */

		/* compute the initial threshold as a fraction of the EM size */
		best_dist = FT_MulFix(face_globals->face->units_per_EM / 40, y_scale);
		if (best_dist > 64 / 4)
			best_dist = 64 / 4;

		for (blue = AH_BLUE_CAPITAL_TOP; blue < AH_BLUE_MAX; blue++) {
			/* if it is a top zone, check for right edges -- if it is a bottom */
			/* zone, check for left edges                                      */
			/*                                                                 */
			/* of course, that's for TrueType XXX                              */
			FT_Bool is_top_blue = FT_BOOL(AH_IS_TOP_BLUE(blue));
			FT_Bool is_major_dir = FT_BOOL(edge->dir == outline->horz_major_dir);

			if (!blue_active[blue])
				continue;

			/* if it is a top zone, the edge must be against the major    */
			/* direction; if it is a bottom zone, it must be in the major */
			/* direction                                                  */
			if (is_top_blue ^ is_major_dir) {
				FT_Pos dist;
				FT_Pos *blue_pos = globals->blue_refs + blue;

				/* first of all, compare it to the reference position */
				dist = edge->fpos - *blue_pos;
				if (dist < 0)
					dist = -dist;

				dist = FT_MulFix(dist, y_scale);
				if (dist < best_dist) {
					best_dist = dist;
					best_blue = blue_pos;
				}

				/* now, compare it to the overshoot position if the edge is     */
				/* rounded, and if the edge is over the reference position of a */
				/* top zone, or under the reference position of a bottom zone   */
				if (edge->flags & AH_EDGE_ROUND && dist != 0) {
					FT_Bool is_under_ref = FT_BOOL(edge->fpos < *blue_pos);

					if (is_top_blue ^ is_under_ref) {
						blue_pos = globals->blue_shoots + blue;
						dist = edge->fpos - *blue_pos;
						if (dist < 0)
							dist = -dist;

						dist = FT_MulFix(dist, y_scale);
						if (dist < best_dist) {
							best_dist = dist;
							best_blue = blue_pos;
						}
					}
				}
			}
		}

		if (best_blue)
			edge->blue_edge = best_blue;
	}
}

void ah_outline_scale_blue_edges(AH_Outline outline, AH_Face_Globals globals) {
	AH_Edge edge = outline->horz_edges;
	AH_Edge edge_limit = edge + outline->num_hedges;
	FT_Pos delta;

	delta = globals->scaled.blue_refs - globals->design.blue_refs;

	for (; edge < edge_limit; edge++) {
		if (edge->blue_edge)
			edge->blue_edge += delta;
	}
}

} // End of namespace FreeType213
} // End of namespace AGS3
