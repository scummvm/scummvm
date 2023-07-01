/***************************************************************************/
/*                                                                         */
/*  pshalgo3.h                                                             */
/*                                                                         */
/*    PostScript hinting algorithm 3 (specification).                      */
/*                                                                         */
/*  Copyright 2001, 2002 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __PSHALGO3_H__
#define __PSHALGO3_H__


#include "pshrec.h"
#include "pshglob.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fttrigon.h"


FT2_1_3_BEGIN_HEADER


/* handle to Hint structure */
typedef struct PSH3_HintRec_*  PSH3_Hint;

/* hint bit-flags */
typedef enum {
	PSH3_HINT_GHOST  = PS_HINT_FLAG_GHOST,
	PSH3_HINT_BOTTOM = PS_HINT_FLAG_BOTTOM,
	PSH3_HINT_ACTIVE = 4,
	PSH3_HINT_FITTED = 8

} PSH3_Hint_Flags;


#define psh3_hint_is_active( x )  ( ( (x)->flags & PSH3_HINT_ACTIVE ) != 0 )
#define psh3_hint_is_ghost( x )   ( ( (x)->flags & PSH3_HINT_GHOST  ) != 0 )
#define psh3_hint_is_fitted( x )  ( ( (x)->flags & PSH3_HINT_FITTED ) != 0 )

#define psh3_hint_activate( x )    (x)->flags |=  PSH3_HINT_ACTIVE
#define psh3_hint_deactivate( x )  (x)->flags &= ~PSH3_HINT_ACTIVE
#define psh3_hint_set_fitted( x )  (x)->flags |=  PSH3_HINT_FITTED

/* hint structure */
typedef struct  PSH3_HintRec_ {
	FT2_1_3_Int     org_pos;
	FT2_1_3_Int     org_len;
	FT2_1_3_Pos     cur_pos;
	FT2_1_3_Pos     cur_len;
	FT2_1_3_UInt    flags;
	PSH3_Hint  parent;
	FT2_1_3_Int     order;

} PSH3_HintRec;


/* this is an interpolation zone used for strong points;  */
/* weak points are interpolated according to their strong */
/* neighbours                                             */
typedef struct  PSH3_ZoneRec_ {
	FT2_1_3_Fixed  scale;
	FT2_1_3_Fixed  delta;
	FT2_1_3_Pos    min;
	FT2_1_3_Pos    max;

} PSH3_ZoneRec, *PSH3_Zone;


typedef struct  PSH3_Hint_TableRec_ {
	FT2_1_3_UInt        max_hints;
	FT2_1_3_UInt        num_hints;
	PSH3_Hint      hints;
	PSH3_Hint*     sort;
	PSH3_Hint*     sort_global;
	FT2_1_3_UInt        num_zones;
	PSH3_ZoneRec*  zones;
	PSH3_Zone      zone;
	PS_Mask_Table  hint_masks;
	PS_Mask_Table  counter_masks;

} PSH3_Hint_TableRec, *PSH3_Hint_Table;


typedef struct PSH3_PointRec_*    PSH3_Point;
typedef struct PSH3_ContourRec_*  PSH3_Contour;

enum {
	PSH3_DIR_NONE  =  4,
	PSH3_DIR_UP    = -1,
	PSH3_DIR_DOWN  =  1,
	PSH3_DIR_LEFT  = -2,
	PSH3_DIR_RIGHT =  2
};

#define PSH3_DIR_HORIZONTAL  2
#define PSH3_DIR_VERTICAL    1

#define PSH3_DIR_COMPARE( d1, d2 )  ( (d1) == (d2) || (d1) == -(d2) )
#define PSH3_DIR_IS_HORIZONTAL( d )  PSH3_DIR_COMPARE( d, PSH3_DIR_HORIZONTAL )
#define PSH3_DIR_IS_VERTICAL( d )    PSH3_DIR_COMPARE( d, PSH3_DIR_VERTICAL )


/* the following bit-flags are computed once by the glyph */
/* analyzer, for both dimensions                          */
enum {
	PSH3_POINT_OFF         = 1,   /* point is off the curve          */
	PSH3_POINT_SMOOTH      = 2,   /* point is smooth                 */
	PSH3_POINT_INFLEX      = 4    /* point is inflection             */
};

#define psh3_point_is_smooth( p )  ( (p)->flags & PSH3_POINT_SMOOTH )
#define psh3_point_is_off( p )     ( (p)->flags & PSH3_POINT_OFF    )
#define psh3_point_is_inflex( p )  ( (p)->flags & PSH3_POINT_INFLEX )

#define psh3_point_set_smooth( p )  (p)->flags |= PSH3_POINT_SMOOTH
#define psh3_point_set_off( p )     (p)->flags |= PSH3_POINT_OFF
#define psh3_point_set_inflex( p )  (p)->flags |= PSH3_POINT_INFLEX

/* the following bit-flags are re-computed for each dimension */
enum {
	PSH3_POINT_STRONG   = 16,   /* point is strong                             */
	PSH3_POINT_FITTED   = 32,   /* point is already fitted                     */
	PSH3_POINT_EXTREMUM = 64,   /* point is local extremum                     */
	PSH3_POINT_POSITIVE = 128,  /* extremum has positive contour flow          */
	PSH3_POINT_NEGATIVE = 256,  /* extremum has negative contour flow          */
	PSH3_POINT_EDGE_MIN = 512,  /* point is aligned to left/bottom stem edge   */
	PSH3_POINT_EDGE_MAX = 1024  /* point is aligned to top/right stem edge     */
};

#define psh3_point_is_strong( p )    ( (p)->flags2 & PSH3_POINT_STRONG )
#define psh3_point_is_fitted( p )    ( (p)->flags2 & PSH3_POINT_FITTED )
#define psh3_point_is_extremum( p )  ( (p)->flags2 & PSH3_POINT_EXTREMUM )
#define psh3_point_is_positive( p )  ( (p)->flags2 & PSH3_POINT_POSITIVE )
#define psh3_point_is_negative( p )  ( (p)->flags2 & PSH3_POINT_NEGATIVE )
#define psh3_point_is_edge_min( p )  ( (p)->flags2 & PSH3_POINT_EDGE_MIN )
#define psh3_point_is_edge_max( p )  ( (p)->flags2 & PSH3_POINT_EDGE_MAX )

#define psh3_point_set_strong( p )    (p)->flags2 |= PSH3_POINT_STRONG
#define psh3_point_set_fitted( p )    (p)->flags2 |= PSH3_POINT_FITTED
#define psh3_point_set_extremum( p )  (p)->flags2 |= PSH3_POINT_EXTREMUM
#define psh3_point_set_positive( p )  (p)->flags2 |= PSH3_POINT_POSITIVE
#define psh3_point_set_negative( p )  (p)->flags2 |= PSH3_POINT_NEGATIVE
#define psh3_point_set_edge_min( p )  (p)->flags2 |= PSH3_POINT_EDGE_MIN
#define psh3_point_set_edge_max( p )  (p)->flags2 |= PSH3_POINT_EDGE_MAX


typedef struct  PSH3_PointRec_ {
	PSH3_Point    prev;
	PSH3_Point    next;
	PSH3_Contour  contour;
	FT2_1_3_UInt       flags;
	FT2_1_3_UInt       flags2;
	FT2_1_3_Char       dir_in;
	FT2_1_3_Char       dir_out;
	FT2_1_3_Angle      angle_in;
	FT2_1_3_Angle      angle_out;
	PSH3_Hint     hint;
	FT2_1_3_Pos        org_u;
	FT2_1_3_Pos        org_v;
	FT2_1_3_Pos        cur_u;
#ifdef DEBUG_HINTER
	FT2_1_3_Pos        org_x;
	FT2_1_3_Pos        cur_x;
	FT2_1_3_Pos        org_y;
	FT2_1_3_Pos        cur_y;
	FT2_1_3_UInt       flags_x;
	FT2_1_3_UInt       flags_y;
#endif

} PSH3_PointRec;


#define PSH3_POINT_EQUAL_ORG( a, b )  ( (a)->org_u == (b)->org_u && \
                                        (a)->org_v == (b)->org_v )

#define PSH3_POINT_ANGLE( a, b )  FT2_1_3_Atan2( (b)->org_u - (a)->org_u,  \
                                            (b)->org_v - (a)->org_v )

typedef struct  PSH3_ContourRec_ {
	PSH3_Point  start;
	FT2_1_3_UInt     count;

} PSH3_ContourRec;


typedef struct  PSH3_GlyphRec_ {
	FT2_1_3_UInt             num_points;
	FT2_1_3_UInt             num_contours;

	PSH3_Point          points;
	PSH3_Contour        contours;

	FT2_1_3_Memory           memory;
	FT2_1_3_Outline*         outline;
	PSH_Globals         globals;
	PSH3_Hint_TableRec  hint_tables[2];

	FT2_1_3_Bool             vertical;
	FT2_1_3_Int              major_dir;
	FT2_1_3_Int              minor_dir;

	FT2_1_3_Bool             do_horz_hints;
	FT2_1_3_Bool             do_vert_hints;
	FT2_1_3_Bool             do_horz_snapping;
	FT2_1_3_Bool             do_vert_snapping;

} PSH3_GlyphRec, *PSH3_Glyph;


#ifdef DEBUG_HINTER
extern PSH3_Hint_Table  ps3_debug_hint_table;

typedef void
(*PSH3_HintFunc)( PSH3_Hint  hint,
                  FT2_1_3_Bool    vertical );

extern PSH3_HintFunc    ps3_debug_hint_func;

extern PSH3_Glyph       ps3_debug_glyph;
#endif


extern FT2_1_3_Error
ps3_hints_apply( PS_Hints        ps_hints,
                 FT2_1_3_Outline*     outline,
                 PSH_Globals     globals,
                 FT2_1_3_Render_Mode  hint_mode );


FT2_1_3_END_HEADER


#endif /* __PSHALGO3_H__ */


/* END */
