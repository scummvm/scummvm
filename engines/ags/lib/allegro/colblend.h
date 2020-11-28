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

#ifndef AGS_LIB_ALLEGRO_COLBLEND_H
#define AGS_LIB_ALLEGRO_COLBLEND_H

#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

extern unsigned long _blender_black(unsigned long x, unsigned long y, unsigned long n);

extern unsigned long _blender_trans15(unsigned long x, unsigned long y, unsigned long n);
extern unsigned long _blender_trans16(unsigned long x, unsigned long y, unsigned long n);

extern unsigned long _blender_alpha15(unsigned long x, unsigned long y, unsigned long n);
extern unsigned long _blender_alpha16(unsigned long x, unsigned long y, unsigned long n);
extern unsigned long _blender_alpha24(unsigned long x, unsigned long y, unsigned long n);

} // namespace AGS3

#endif
