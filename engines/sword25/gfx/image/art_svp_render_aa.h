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

#ifndef __ART_SVP_RENDER_AA_H__
#define __ART_SVP_RENDER_AA_H__

/* The spiffy antialiased renderer for sorted vector paths. */

#include "art.h"

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

#endif /* __ART_SVP_RENDER_AA_H__ */
