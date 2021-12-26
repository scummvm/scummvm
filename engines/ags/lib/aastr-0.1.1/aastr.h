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

/*
 * aastr.h --- anti-aliased stretching and rotation for Allegro
 *
 * This file is gift-ware.  This file is given to you freely
 * as a gift.  You may use, modify, redistribute, and generally hack
 * it about in any way you like, and you do not have to give anyone
 * anything in return.
 *
 * I do not accept any responsibility for any effects, adverse or
 * otherwise, that this code may have on just about anything that
 * you can think of.  Use it at your own risk.
 *
 * Copyright (C) 1998, 1999  Michael Bukin
 */

#ifndef AGS_LIB_AASTR_AASTR_H
#define AGS_LIB_AASTR_AASTR_H

#include "ags/lib/allegro.h"

namespace AGS3 {

#ifdef __cplusplus
extern "C" {
#endif

/* Stretching.  */
void aa_stretch_blit(BITMAP *src, BITMAP *dst,
                     int sx, int sy, int sw, int sh,
                     int dx, int dy, int dw, int dh);
void aa_stretch_sprite(BITMAP *dst, BITMAP *src,
                       int dx, int dy, int dw, int dh);

/* Rotation.  */
void aa_rotate_scaled_bitmap(BITMAP *src, BITMAP *dst,
                             int x, int y, fixed angle,
                             fixed scalex, fixed scaley);
void aa_rotate_scaled_sprite(BITMAP *dst, BITMAP *src,
                             int x, int y, fixed angle,
                             fixed scalex, fixed scaley);
void aa_rotate_bitmap(BITMAP *src, BITMAP *dst,
                      int x, int y, fixed angle);
void aa_rotate_sprite(BITMAP *dst, BITMAP *src,
                      int x, int y, fixed angle);

#ifdef __cplusplus
}
#endif

} // namespace AGS3

#endif
