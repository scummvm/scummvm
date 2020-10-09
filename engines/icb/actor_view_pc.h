/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_ACTOR_VIEW_PC_H_INCLUDED
#define ICB_ACTOR_VIEW_PC_H_INCLUDED

#include "engines/icb/common/px_rccommon.h"

namespace ICB {

// Set this to true if the actor view (character profiles) should act as voxview
extern bool8 g_av_userControlled;

// Return values for ActorViewDraw
#define MID_ANIMATION 0x00
#define ANIMATION_END 0x01

void InitActorView(const char *name,   // Character to draw    (ie "cord")
                   const char *outfit, // Characters outfit    (ie "casual_wear")
                   const char *weapon, // Character pose       (ie "unarmed")
                   const char *anim,   // Character anim       (ie "walk")
                   int16 ix,           // Initial render x coord
                   int16 iy,           // Initial render y coord
                   int16 iz);          // Initial render z coord

int ActorViewDraw(); // Call each cycle to draw

void ChangeAnimPlaying(const char *pose, // Pose to swap to NULL if keep the same
                       const char *anim, // Anim to change to
                       bool8 forwards,   // Play anim forwards or backwards
                       int32 repeats,    // Number of times to play this anim before ActorViewDraw() returns ANIMATION_END
                       int16 ix,         // New render x coordinate
                       int16 iy,         // New render y coordinate
                       int16 iz);        // New render z coordinate

} // End of namespace ICB

#endif
