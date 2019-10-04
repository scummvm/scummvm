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

namespace Glk {
namespace JACL {

#define MAX_WORDS               20
#define STACK_SIZE              20
#define MAX_UNDO                100
#define MAX_OBJECTS             1000

/* LOCATION ATTRIBUTE VALUES */

#define VISITED         1
#define DARK            2
#define ON_WATER        4
#define UNDER_WATER     8
#define WITHOUT_AIR     16
#define OUTDOORS        32
#define MID_AIR         64
#define TIGHT_ROPE      128
#define POLLUTED        256
#define SOLVED          512
#define MID_WATER       1024
#define DARKNESS        2048
#define MAPPED          4096
#define KNOWN           8192

/* OBJECT ATTRIBUTE VALUES */

#define CLOSED          1
#define LOCKED          2
#define DEAD            4
#define IGNITABLE       8
#define WORN            16
#define CONCEALING      32
#define LUMINOUS        64
#define WEARABLE        128
#define CLOSABLE        256
#define LOCKABLE        512
#define ANIMATE         1024
#define LIQUID          2048
#define CONTAINER       4096
#define SURFACE         8192
#define PLURAL          16384
#define FLAMMABLE       32768
#define BURNING         65536
#define LOCATION        131072
#define ON              262144
#define DAMAGED         524288
#define FEMALE          1048576
#define POSSESSIVE      2097152
#define OUT_OF_REACH    4194304
#define TOUCHED         8388608
#define SCORED          16777216
#define SITTING         33554432
#define NPC             67108864
#define DONE            134217728
#define GAS             268435456
#define NO_TAB          536870912
#define NOT_IMPORTANT   1073741824

/* LOCATION INTEGER ARRAY INDEXES */

#define NORTH_DIR           0
#define SOUTH_DIR           1
#define EAST_DIR            2
#define WEST_DIR            3
#define NORTHEAST_DIR       4
#define NORTHWEST_DIR       5
#define SOUTHEAST_DIR       6
#define SOUTHWEST_DIR       7
#define UP_DIR              8
#define DOWN_DIR            9
#define IN_DIR              10
#define OUT_DIR             11

/* ALL UP, THERE ARE 16 OBJECT ELEMENTS, THESE 6
   ARE THE ONLY ONES ACCESSED BY THE INTERPRETER */

#define PARENT          integer[0]
#define QUANTITY        integer[1]
#define MASS            integer[2]
#define BEARING         integer[3]
#define VELOCITY        integer[4]
#define X               integer[14]
#define Y               integer[15]

/* SYSTEM VARIABLES */

#define COMPASS                 integer_resolve("compass")
#define DESTINATION             integer_resolve("destination")
#define TOTAL_MOVES             integer_resolve("total_moves")
#define TIME                    integer_resolve("time")
#define SCORE                   integer_resolve("score")
#define INTERNAL_VERSION        integer_resolve("internal_version")
#define DISPLAY_MODE            integer_resolve("display_mode")
#define MAX_RAND                integer_resolve("max_rand")
#define INTERRUPTED             integer_resolve("interrupted")
#define SOUND_ENABLED           integer_resolve("sound_enabled")
#define GRAPHICS_ENABLED        integer_resolve("graphics_enabled")
#define TIMER_ENABLED           integer_resolve("timer_enabled")
#define MULTI_PREFIX            integer_resolve("multi_prefix")
#define NOTIFY                  integer_resolve("notify")
#define DEBUG                   integer_resolve("debug")

/* SYSTEM INTEGER CONSTANTS */

#define SOUND_SUPPORTED         cinteger_resolve("sound_supported")
#define GRAPHICS_SUPPORTED      cinteger_resolve("graphics_supported")
#define TIMER_SUPPORTED         cinteger_resolve("timer_supported")

/* ABBREVIATIONS */

#define HELD                    player
#define HERE                    get_here()

/* CONSTANTS */

#define SYSTEM_ATTRIBUTE        0
#define USER_ATTRIBUTE          1

#define FALSE                   0
#define TRUE                    1

#define UNRESTRICT              0
#define RESTRICT                1

#define SEEK_SET                0

#define SEEK_END                2

#define SCENERY                 100
#define HEAVY                   99
#define NOWHERE                 0

#define LOG_ONLY                0
#define PLUS_STDOUT             1
#define PLUS_STDERR             2
#define ONLY_STDERR             3
#define ONLY_STDOUT             4

#define INT_TYPE                1
#define STR_TYPE                2
#define CINT_TYPE               3
#define CSTR_TYPE               4
#define ATT_TYPE                5
#define OBJ_TYPE                6

#define CRI_NONE                0
#define CRI_ATTRIBUTE           1
#define CRI_USER_ATTRIBUTE      2
#define CRI_PARENT              3
#define CRI_SCOPE               4

#define BOLD                    1
#define NOTE                    2
#define INPUT                   3
#define HEADER                  4
#define SUBHEADER               5
#define REVERSE                 6
#define PRE                     7

} // End of namespace JACL
} // End of namespace Glk
