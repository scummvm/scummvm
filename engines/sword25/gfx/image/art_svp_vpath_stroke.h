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

#ifndef __ART_SVP_VPATH_STROKE_H__
#define __ART_SVP_VPATH_STROKE_H__

/* Sort vector paths into sorted vector paths. */

#include "art.h"

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

#endif /* __ART_SVP_VPATH_STROKE_H__ */
