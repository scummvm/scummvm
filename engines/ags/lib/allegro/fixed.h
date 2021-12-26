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

#ifndef AGS_LIB_ALLEGRO_FIXED_H
#define AGS_LIB_ALLEGRO_FIXED_H

#include "common/scummsys.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

typedef int32_t fixed;

extern const fixed _cos_tbl[];
extern const fixed _tan_tbl[];
extern const fixed _acos_tbl[];

extern fixed ftofix(double x);
extern double fixtof(fixed x);
extern fixed fixadd(fixed x, fixed y);
extern fixed fixsub(fixed x, fixed y);
extern fixed fixmul(fixed x, fixed y);
extern fixed fixdiv(fixed x, fixed y);
extern int fixfloor(fixed x);
extern int fixceil(fixed x);
extern fixed itofix(int x);
extern int fixtoi(fixed x);
extern fixed fixcos(fixed x);
extern fixed fixsin(fixed x);
extern fixed fixtan(fixed x);
extern fixed fixacos(fixed x);
extern fixed fixasin(fixed x);

} // namespace AGS3

#endif
