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

#ifndef MAGNETIC_DEFS_H
#define MAGNETIC_DEFS_H

#include "common/scummsys.h"

namespace Glk {
namespace Magnetic {

/*****************************************************************************\
* Type definitions for Magnetic
*
* Note: When running into trouble please ensure that these types have the
*       correct number of bits on your system !!!
\*****************************************************************************/

typedef byte type8;
typedef int8 type8s;
typedef uint16 type16;
typedef int16 type16s;
typedef uint32 type32;
typedef int32 type32s;

enum { BYTE_MAX_VAL = 255 };
enum { BITS_PER_BYTE = 8 };
enum { UINT16_MAX_VAL = 0xffff };
enum { INT32_MAX_VAL = 0x7fffffff };

#define MAX_HINTS 260
#define MAX_HCONTENTS 30000

#define MAX_POSITIONS 20
#define MAX_ANIMS 200
#define MAX_FRAMES 20
#define MAX_STRING_SIZE  0xFF00
#define MAX_PICTURE_SIZE 0xC800
#define MAX_MUSIC_SIZE   0x4E20
#define MAX_HITEMS 25

/****************************************************************************\
* Compile time switches
\****************************************************************************/

/* Switch:  SAVEMEM
   Purpose: Magnetic loads a complete graphics file into memory by default.
            Setting this switch you tell Magnetic to load images on request
            (saving memory, wasting load time)

#define SAVEMEM
*/

/* Switch:  NO_ANIMATION
   Purpose: By default Magnetic plays animated graphics.
            Setting this switch to ignore animations, Magnetic shows the
            static parts of the images anyway!

#define NO_ANIMATION
*/

/****************************************************************************\
* Miscellaneous enums/types
*
\****************************************************************************/

enum { GMS_PALETTE_SIZE = 16 };

enum { GMS_INPUTBUFFER_LENGTH = 256 };

enum { GMS_STATBUFFER_LENGTH = 1024 };

enum GammaMode {
	GAMMA_OFF, GAMMA_NORMAL, GAMMA_HIGH
};

/* Hint type definitions. */
enum {
	GMS_HINT_TYPE_FOLDER = 1,
	GMS_HINT_TYPE_TEXT = 2
};

} // End of namespace Magnetic
} // End of namespace Glk

#endif
