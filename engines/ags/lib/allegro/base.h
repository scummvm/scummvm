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

#ifndef AGS_LIB_ALLEGRO_BASE_H
#define AGS_LIB_ALLEGRO_BASE_H

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/endian.h"
#include "ags/lib/allegro/error.h"

namespace AGS3 {

#define ALLEGRO_VERSION          4
#define ALLEGRO_SUB_VERSION      4
#define ALLEGRO_WIP_VERSION      2
#define ALLEGRO_VERSION_STR      "4.4.2"
#define ALLEGRO_DATE_STR         "2011"
#define ALLEGRO_DATE             20110519    /* yyyymmdd */

/* Returns the median of x, y, z */
#define MID(x,y,z)   ((x) > (y) ? ((y) > (z) ? (y) : ((x) > (z) ?    \
                                   (z) : (x))) : ((y) > (z) ? ((z) > (x) ? (z) : \
                                           (x)): (y)))

#define AL_ID MKTAG

#define AL_FUNC(type, name, args)               type name args

#define SCUMMVM_ID MKTAG('S', 'C', 'V', 'M')

} // namespace AGS3

#endif
