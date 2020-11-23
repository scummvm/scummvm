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

#ifndef AGS_STUBS_ALLEGRO_BASE_H
#define AGS_STUBS_ALLEGRO_BASE_H

#include "common/scummsys.h"
#include "common/algorithm.h"

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

extern int *allegro_errno;

/**
 * info about a hardware driver
 */
struct _DRIVER_INFO {
	int id;                          /* integer ID */
	void *driver;                    /* the driver structure */
	int autodetect;                  /* set to allow autodetection */
};

#define AL_FUNC(type, name, args)               type name args

} // namespace AGS3

#endif
