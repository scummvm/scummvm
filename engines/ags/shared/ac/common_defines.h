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

#ifndef AGS_SHARED_AC_COMMON_DEFINES_H
#define AGS_SHARED_AC_COMMON_DEFINES_H

#include "ags/shared/core/platform.h"

namespace AGS3 {

#define EXIT_NORMAL 0
#define EXIT_CRASH  92
#define EXIT_ERROR  93


#if defined (OBSOLETE)
#define NUM_MISC      20
#define NUMOTCON      7                 // number of conditions before standing on
#define NUM_CONDIT    (120 + NUMOTCON)
#endif

#define MAX_SCRIPT_NAME_LEN 20

//const int MISC_COND = MAX_WALK_BEHINDS * 4 + NUMOTCON + MAX_ROOM_OBJECTS * 4;

// NUMCONDIT : whataction[0]:  Char walks off left
//                       [1]:  Char walks off right
//                       [2]:  Char walks off bottom
//                       [3]:  Char walks off top
//			 [4]:  First enters screen
//                       [5]:  Every time enters screen
//                       [6]:  execute every loop
//                [5]...[19]:  Char stands on lookat type
//               [20]...[35]:  Look at type
//               [36]...[49]:  Action on type
//               [50]...[65]:  Use inv on type
//               [66]...[75]:  Look at object
//               [76]...[85]:  Action on object
//               [86]...[95]:  Speak to object
//		[96]...[105]:  Use inv on object
//             [106]...[124]:  Misc conditions 1-20

// game ver     whataction[]=
// v1.00              0  :  Go to screen
//                    1  :  Don't do anything
//                    2  :  Can't walk
//                    3  :  Man dies
//                    4  :  Run animation
//                    5  :  Display message
//                    6  :  Remove an object (set object.on=0)
//		                7  :  Remove object & add Val2 to inventory
//                    8  :  Add Val1 to inventory (Val2=num times)
//                    9  :  Run a script
// v1.00 SR-1        10  :  Run graphical script
// v1.1              11  :  Play sound effect SOUND%d.WAV
// v1.12             12  :  Play FLI/FLC animation FLIC%d.FLC or FLIC%d.FLI
//                   13  :  Turn object on
// v2.00             14  :  Run conversation
#define NUMRESPONSE   14
#define NUMCOMMANDS   15
#define GO_TO_SCREEN  0
#define NO_ACTION     1
#define NO_WALK       2
#define MAN_DIES      3
#define RUN_ANIMATE   4
#define SHOW_MESSAGE  5
#define OBJECT_OFF    6
#define OBJECT_INV    7
#define ADD_INV       8
#define RUNSCRIPT     9
#define GRAPHSCRIPT   10
#define PLAY_SOUND    11
#define PLAY_FLI      12
#define OBJECT_ON     13
#define RUN_DIALOG    14

// Number of state-saved rooms
#define MAX_ROOMS 300
// Some obsolete room data, likely pre-2.5
#define MAX_LEGACY_ROOM_FLAGS 15

#define LEGACY_MAXOBJNAMELEN 30

#define LEGACY_MAX_SPRITES_V25  6000
#define LEGACY_MAX_SPRITES      30000

#if AGS_PLATFORM_OS_WINDOWS
#define AGS_INLINE inline
#else
// the linux compiler won't allow extern inline
#define AGS_INLINE
#endif

// The game to screen coordinate conversion multiplier in hi-res type games
#define HIRES_COORD_MULTIPLIER 2

// object flags (currently only a char)
#define OBJF_NOINTERACT        1  // not clickable
#define OBJF_NOWALKBEHINDS     2  // ignore walk-behinds
#define OBJF_HASTINT           4  // the tint_* members are valid
#define OBJF_USEREGIONTINTS    8  // obey region tints/light areas
#define OBJF_USEROOMSCALING 0x10  // obey room scaling areas
#define OBJF_SOLID          0x20  // blocks characters from moving
#define OBJF_LEGACY_LOCKED  0x40  // object position is locked in the editor (OBSOLETE since 3.5.0)
#define OBJF_HASLIGHT       0x80  // the tint_light is valid and treated as brightness

// Animation flow mode
// NOTE: had to move to common_defines, because used by CharacterInfo
// Animates once and stops at the *last* frame
#define ANIM_ONCE              0
// Animates infinitely until stopped by command
#define ANIM_REPEAT            1
// Animates once and stops, resetting to the very first frame
#define ANIM_ONCERESET         2

} // namespace AGS3

#endif
