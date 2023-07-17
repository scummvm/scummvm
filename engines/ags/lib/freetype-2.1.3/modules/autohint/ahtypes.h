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
/*  ahtypes.h                                                              */
/*    General types and definitions for the auto-hint module               */
/*    (specification only).                                                */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_AHTYPES_H
#define AGS_LIB_FREETYPE_AHTYPES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

#ifdef DEBUG_HINTER
#include <../modules/autohint/ahloader.h>
#else
#include "engines/ags/lib/freetype-2.1.3/modules/autohint/ahloader.h"
#endif


#define xxAH_DEBUG


#ifdef AH_DEBUG

#include <stdio.h>
#define AH_LOG(x) printf##x

#else

#define AH_LOG(x)  do ; while(0) /* nothing */

#endif /* AH_DEBUG */

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/**** COMPILE-TIME BUILD OPTIONS ****/

/*************************************************************************/
/*                                                                       */
/* If this option is defined, only strong interpolation will be used to  */
/* place the points between edges.  Otherwise, `smooth' points are       */
/* detected and later hinted through weak interpolation to correct some  */
/* unpleasant artefacts.                                                 */
/*                                                                       */
#undef AH_OPTION_NO_WEAK_INTERPOLATION

/*************************************************************************/
/*                                                                       */
/* If this option is defined, only weak interpolation will be used to    */
/* place the points between edges.  Otherwise, `strong' points are       */
/* detected and later hinted through strong interpolation to correct     */
/* some unpleasant artefacts.                                            */
/*                                                                       */
#undef AH_OPTION_NO_STRONG_INTERPOLATION

/*************************************************************************/
/*                                                                       */
/* Undefine this macro if you don't want to hint the metrics.  There is  */
/* no reason to do this (at least for non-CJK scripts), except for       */
/* experimentation.                                                      */
/*                                                                       */
#undef  AH_HINT_METRICS

/*************************************************************************/
/*                                                                       */
/* Define this macro if you do not want to insert extra edges at a       */
/* glyph's x and y extremum (if there isn't one already available).      */
/* This helps to reduce a number of artefacts and allows hinting of      */
/* metrics.                                                              */
/*                                                                       */
#undef AH_OPTION_NO_EXTREMUM_EDGES

/* don't touch for now */
#define AH_MAX_WIDTHS   12
#define AH_MAX_HEIGHTS  12


/**** TYPE DEFINITIONS ****/

/* see agangles.h */
typedef FT_Int AH_Angle;

/* hint flags */
#define AH_FLAG_NONE       0

/* bezier control points flags */
#define AH_FLAG_CONIC                 1
#define AH_FLAG_CUBIC                 2
#define AH_FLAG_CONTROL               ( AH_FLAG_CONIC | AH_FLAG_CUBIC )

/* extrema flags */
#define AH_FLAG_EXTREMA_X             4
#define AH_FLAG_EXTREMA_Y             8

/* roundness */
#define AH_FLAG_ROUND_X              16
#define AH_FLAG_ROUND_Y              32

/* touched */
#define AH_FLAG_TOUCH_X              64
#define AH_FLAG_TOUCH_Y             128

/* weak interpolation */
#define AH_FLAG_WEAK_INTERPOLATION  256
#define AH_FLAG_INFLECTION          512

typedef FT_Int AH_Flags;

/* edge hint flags */
#define AH_EDGE_NORMAL  0
#define AH_EDGE_ROUND   1
#define AH_EDGE_SERIF   2
#define AH_EDGE_DONE    4

typedef FT_Int AH_Edge_Flags;

/* hint directions -- the values are computed so that two vectors are */
/* in opposite directions iff `dir1+dir2 == 0'                        */
#define AH_DIR_NONE    4
#define AH_DIR_RIGHT   1
#define AH_DIR_LEFT   -1
#define AH_DIR_UP      2
#define AH_DIR_DOWN   -2

typedef FT_Int  AH_Direction;

typedef struct AH_PointRec_   *AH_Point;
typedef struct AH_SegmentRec_ *AH_Segment;
typedef struct AH_EdgeRec_ 	  *AH_Edge;

typedef struct AH_PointRec_ {
	AH_Flags flags; /* point flags used by hinter */
	FT_Pos ox, oy;
	FT_Pos fx, fy;
	FT_Pos x, y;
	FT_Pos u, v;

	AH_Direction in_dir;  /* direction of inwards vector  */
	AH_Direction out_dir; /* direction of outwards vector */

	AH_Angle in_angle;
	AH_Angle out_angle;

	AH_Point next; /* next point in contour     */
	AH_Point prev; /* previous point in contour */
} AH_PointRec;


typedef struct AH_SegmentRec_ {
	AH_Edge_Flags flags;
	AH_Direction dir;

	AH_Point first;    /* first point in edge segment             */
	AH_Point last;     /* last point in edge segment              */
	AH_Point *contour; /* ptr to first point of segment's contour */

	FT_Pos pos;       /* position of segment           */
	FT_Pos min_coord; /* minimum coordinate of segment */
	FT_Pos max_coord; /* maximum coordinate of segment */

	AH_Edge edge;
	AH_Segment edge_next;

	AH_Segment link;   /* link segment               */
	AH_Segment serif;  /* primary segment for serifs */
	FT_Pos num_linked; /* number of linked segments  */
	FT_Pos score;
} AH_SegmentRec;


typedef struct AH_EdgeRec_ {
	AH_Edge_Flags flags;
	AH_Direction dir;

	AH_Segment first;
	AH_Segment last;

	FT_Pos fpos;
	FT_Pos opos;
	FT_Pos pos;

	AH_Edge link;
	AH_Edge serif;
	FT_Int num_linked;

	FT_Int score;
	FT_Pos *blue_edge;
} AH_EdgeRec;


/* an outline as seen by the hinter */
typedef struct AH_OutlineRec_ {
	FT_Memory memory;

	AH_Direction vert_major_dir; /* vertical major direction   */
	AH_Direction horz_major_dir; /* horizontal major direction */

	FT_Fixed x_scale;
	FT_Fixed y_scale;
	FT_Pos edge_distance_threshold;

	FT_Int max_points;
	FT_Int num_points;
	AH_Point points;

	FT_Int max_contours;
	FT_Int num_contours;
	AH_Point *contours;

	FT_Int num_hedges;
	AH_Edge horz_edges;

	FT_Int num_vedges;
	AH_Edge vert_edges;

	FT_Int num_hsegments;
	AH_Segment horz_segments;

	FT_Int num_vsegments;
	AH_Segment vert_segments;
} AH_OutlineRec, *AH_Outline;


#define AH_BLUE_CAPITAL_TOP     0                              /* THEZOCQS */
#define AH_BLUE_CAPITAL_BOTTOM  ( AH_BLUE_CAPITAL_TOP + 1 )    /* HEZLOCUS */
#define AH_BLUE_SMALL_TOP       ( AH_BLUE_CAPITAL_BOTTOM + 1 ) /* xzroesc  */
#define AH_BLUE_SMALL_BOTTOM    ( AH_BLUE_SMALL_TOP + 1 )      /* xzroesc  */
#define AH_BLUE_SMALL_MINOR     ( AH_BLUE_SMALL_BOTTOM + 1 )   /* pqgjy    */
#define AH_BLUE_MAX             ( AH_BLUE_SMALL_MINOR + 1 )

typedef FT_Int AH_Blue;


#define AH_HINTER_MONOCHROME  1
#define AH_HINTER_OPTIMIZE    2

typedef FT_Int AH_Hinter_Flags;


typedef struct AH_GlobalsRec_ {
	FT_Int num_widths;
	FT_Int num_heights;

	FT_Pos stds[2];

	FT_Pos widths[AH_MAX_WIDTHS];
	FT_Pos heights[AH_MAX_HEIGHTS];

	FT_Pos blue_refs[AH_BLUE_MAX];
	FT_Pos blue_shoots[AH_BLUE_MAX];
} AH_GlobalsRec, *AH_Globals;


typedef struct AH_Face_GlobalsRec_ {
	FT_Face face;
	AH_GlobalsRec design;
	AH_GlobalsRec scaled;
	FT_Fixed x_scale;
	FT_Fixed y_scale;
	FT_Bool control_overshoot;
} AH_Face_GlobalsRec, *AH_Face_Globals;


typedef struct AH_HinterRec {
	FT_Memory memory;
	AH_Hinter_Flags flags;

	FT_Int algorithm;
	FT_Face face;

	AH_Face_Globals globals;

	AH_Outline glyph;

	AH_Loader loader;
	FT_Vector pp1;
	FT_Vector pp2;

	FT_Bool transformed;
	FT_Vector trans_delta;
	FT_Matrix trans_matrix;

	FT_Bool do_horz_hints;    /* disable X hinting            */
	FT_Bool do_vert_hints;    /* disable Y hinting            */
	FT_Bool do_horz_snapping; /* disable X stem size snapping */
	FT_Bool do_vert_snapping; /* disable Y stem size snapping */
} AH_HinterRec, *AH_Hinter;


#ifdef  DEBUG_HINTER
extern AH_Hinter   ah_debug_hinter;
extern FT_Bool     ah_debug_disable_horz;
extern FT_Bool     ah_debug_disable_vert;
#else
#define ah_debug_disable_horz  0
#define ah_debug_disable_vert  0
#endif /* DEBUG_HINTER */


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHTYPES_H */
