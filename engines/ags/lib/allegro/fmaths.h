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

#ifndef AGS_LIB_ALLEGRO_FMATHS_H
#define AGS_LIB_ALLEGRO_FMATHS_H

#include "ags/lib/allegro/fixed.h"

namespace AGS3 {

AL_FUNC(fixed, fixsqrt, (fixed x));
AL_FUNC(fixed, fixhypot, (fixed x, fixed y));
AL_FUNC(fixed, fixatan, (fixed x));
AL_FUNC(fixed, fixatan2, (fixed y, fixed x));

AL_ARRAY(fixed, _cos_tbl);
AL_ARRAY(fixed, _tan_tbl);
AL_ARRAY(fixed, _acos_tbl);

} // namespace AGS3

#endif
