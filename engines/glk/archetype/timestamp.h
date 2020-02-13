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

#ifndef ARCHETYPE_TIMESTAMP
#define ARCHETYPE_TIMESTAMP

#include "common/scummsys.h"

namespace Glk {
namespace Archetype {

typedef uint32 TimestampType;

extern TimestampType GTimeStamp;

extern void timestamp_init();

/**
 * Creates a compressed long integer that contains all the necessary time information.
 * There are enough bits in a 32-bit word to do this :
 * 
 * Variable      Range                             Bits
 * --------      -----                             ----
 * Year          0-63                                6
 * Month         1-12                                4
 * Day           0-31                                5
 * Hour          0-23                                5
 * Minute        0-59                                6
 * Second        0-59                                6
 * 
 * Note that Year does not quite fit comfortably into this scheme.The actual returned value
 * is 1980-2099, a span of 119 years; but we are using only 63.  Year 0 is considered 1992
 * and the upper limit is 2055 before it goes back to year 0 (1992) again.
 * 
 * The DayOfWeek information is thrown away because it is redundant, and the Sec100 information
 * is thrown away because it is unnecessarily precise
 */
extern void get_time_stamp(TimestampType &tstamp);

} // End of namespace Archetype
} // End of namespace Glk

#endif
