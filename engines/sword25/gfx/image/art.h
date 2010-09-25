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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Libart_LGPL - library of basic graphic primitives
 *
 * Copyright (c) 1998 Raph Levien
 *
 * Licensed under GNU LGPL v2
 *
 */

/* Simple macros to set up storage allocation and basic types for libart
   functions. */

#ifndef __ART_MISC_H__
#define __ART_MISC_H__

#include "common/scummsys.h"

typedef byte art_u8;
typedef uint16 art_u16;
typedef uint32 art_u32;

/* These aren't, strictly speaking, configuration macros, but they're
   damn handy to have around, and may be worth playing with for
   debugging. */
#define art_new(type, n) ((type *)malloc ((n) * sizeof(type)))

#define art_renew(p, type, n) ((type *)realloc (p, (n) * sizeof(type)))

/* This one must be used carefully - in particular, p and max should
   be variables. They can also be pstruct->el lvalues. */
#define art_expand(p, type, max) do { if(max) { p = art_renew (p, type, max <<= 1); } else { max = 1; p = art_new(type, 1); } } while (0)

typedef int art_boolean;
#define ART_FALSE 0
#define ART_TRUE 1

/* define pi */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  /*  M_PI  */

#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880  /* sqrt(2) */
#endif  /* M_SQRT2 */

/* Provide macros to feature the GCC function attribute.
 */
#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4))
#define ART_GNUC_PRINTF( format_idx, arg_idx )    \
	__attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define ART_GNUC_NORETURN                         \
	__attribute__((__noreturn__))
#else   /* !__GNUC__ */
#define ART_GNUC_PRINTF( format_idx, arg_idx )
#define ART_GNUC_NORETURN
#endif  /* !__GNUC__ */

void ART_GNUC_NORETURN
art_die(const char *fmt, ...) ART_GNUC_PRINTF(1, 2);

void
art_warn(const char *fmt, ...) ART_GNUC_PRINTF(1, 2);

typedef struct _ArtDRect ArtDRect;
typedef struct _ArtIRect ArtIRect;

struct _ArtDRect {
	/*< public >*/
	double x0, y0, x1, y1;
};

struct _ArtIRect {
	/*< public >*/
	int x0, y0, x1, y1;
};

typedef struct _ArtPoint ArtPoint;

struct _ArtPoint {
	/*< public >*/
	double x, y;
};

/* Basic data structures and constructors for sorted vector paths */

typedef struct _ArtSVP ArtSVP;
typedef struct _ArtSVPSeg ArtSVPSeg;

struct _ArtSVPSeg {
	int n_points;
	int dir; /* == 0 for "up", 1 for "down" */
	ArtDRect bbox;
	ArtPoint *points;
};

struct _ArtSVP {
	int n_segs;
	ArtSVPSeg segs[1];
};

void
art_svp_free(ArtSVP *svp);

int
art_svp_seg_compare(const void *s1, const void *s2);

/* Basic data structures and constructors for bezier paths */

typedef enum {
	ART_MOVETO,
	ART_MOVETO_OPEN,
	ART_CURVETO,
	ART_LINETO,
	ART_END
} ArtPathcode;

typedef struct _ArtBpath ArtBpath;

struct _ArtBpath {
	/*< public >*/
	ArtPathcode code;
	double x1;
	double y1;
	double x2;
	double y2;
	double x3;
	double y3;
};

/* Basic data structures and constructors for simple vector paths */

typedef struct _ArtVpath ArtVpath;

/* CURVETO is not allowed! */
struct _ArtVpath {
	ArtPathcode code;
	double x;
	double y;
};

/* Some of the functions need to go into their own modules */

void
art_vpath_add_point(ArtVpath **p_vpath, int *pn_points, int *pn_points_max,
                    ArtPathcode code, double x, double y);

ArtVpath *art_bez_path_to_vec(const ArtBpath *bez, double flatness);

/* The funky new SVP intersector. */

#ifndef ART_WIND_RULE_DEFINED
#define ART_WIND_RULE_DEFINED
typedef enum {
	ART_WIND_RULE_NONZERO,
	ART_WIND_RULE_INTERSECT,
	ART_WIND_RULE_ODDEVEN,
	ART_WIND_RULE_POSITIVE
} ArtWindRule;
#endif

typedef struct _ArtSvpWriter ArtSvpWriter;

struct _ArtSvpWriter {
	int (*add_segment)(ArtSvpWriter *self, int wind_left, int delta_wind,
	                   double x, double y);
	void (*add_point)(ArtSvpWriter *self, int seg_id, double x, double y);
	void (*close_segment)(ArtSvpWriter *self, int seg_id);
};

ArtSvpWriter *
art_svp_writer_rewind_new(ArtWindRule rule);

ArtSVP *
art_svp_writer_rewind_reap(ArtSvpWriter *self);

int
art_svp_seg_compare(const void *s1, const void *s2);

void
art_svp_intersector(const ArtSVP *in, ArtSvpWriter *out);


/* Sort vector paths into sorted vector paths. */

ArtSVP *
art_svp_from_vpath(ArtVpath *vpath);

/* Sort vector paths into sorted vector paths. */

typedef enum {
	ART_PATH_STROKE_JOIN_MITER,
	ART_PATH_STROKE_JOIN_ROUND,
	ART_PATH_STROKE_JOIN_BEVEL
} ArtPathStrokeJoinType;

typedef enum {
	ART_PATH_STROKE_CAP_BUTT,
	ART_PATH_STROKE_CAP_ROUND,
	ART_PATH_STROKE_CAP_SQUARE
} ArtPathStrokeCapType;

ArtSVP *
art_svp_vpath_stroke(ArtVpath *vpath,
                     ArtPathStrokeJoinType join,
                     ArtPathStrokeCapType cap,
                     double line_width,
                     double miter_limit,
                     double flatness);

/* This version may have winding numbers exceeding 1. */
ArtVpath *
art_svp_vpath_stroke_raw(ArtVpath *vpath,
                         ArtPathStrokeJoinType join,
                         ArtPathStrokeCapType cap,
                         double line_width,
                         double miter_limit,
                         double flatness);


/* The spiffy antialiased renderer for sorted vector paths. */

typedef struct _ArtSVPRenderAAStep ArtSVPRenderAAStep;
typedef struct _ArtSVPRenderAAIter ArtSVPRenderAAIter;

struct _ArtSVPRenderAAStep {
	int x;
	int delta; /* stored with 16 fractional bits */
};

ArtSVPRenderAAIter *
art_svp_render_aa_iter(const ArtSVP *svp,
                       int x0, int y0, int x1, int y1);

void
art_svp_render_aa_iter_step(ArtSVPRenderAAIter *iter, int *p_start,
                            ArtSVPRenderAAStep **p_steps, int *p_n_steps);

void
art_svp_render_aa_iter_done(ArtSVPRenderAAIter *iter);

void
art_svp_render_aa(const ArtSVP *svp,
                  int x0, int y0, int x1, int y1,
                  void (*callback)(void *callback_data,
                                   int y,
                                   int start,
                                   ArtSVPRenderAAStep *steps, int n_steps),
                  void *callback_data);


#endif /* __ART_MISC_H__ */
