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

#ifndef __ART_SVP_INTERSECT_H__
#define __ART_SVP_INTERSECT_H__

/* The funky new SVP intersector. */

#include "art.h"

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

#endif /* __ART_SVP_INTERSECT_H__ */
