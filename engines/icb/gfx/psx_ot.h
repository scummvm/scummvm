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

#ifndef ICB_PSX_OT_H
#define ICB_PSX_OT_H

namespace ICB {

// By default setup for a reverse OT
#if defined FORWARD_OT
#define OT_LAST OT_SIZE - 1
#define OT_FIRST 0
#define OT_DIRECTION 1
#else
#define OT_LAST 0
#define OT_FIRST OT_SIZE - 1
#define OT_DIRECTION -1
#endif // #if defined FORWARD_OT

// Where to put the DR_LOAD primitives in the OT list
#define DRLOAD_OT OT_FIRST

// Where to put the profiling bars
#ifdef PROFILE_FIRST
#define PROFILE_OT OT_FIRST
#else
#define PROFILE_OT (OT_LAST - OT_DIRECTION)
#endif

// Where to put the BG sprites in the OT list
#define OT_BG (OT_FIRST + OT_DIRECTION)

// Where to put the PROP sprites in the OT list
#define OT_PROPS (OT_BG + OT_DIRECTION)

// Where to put the LAYER sprites in the OT list
#define OT_LAYERS (OT_PROPS + OT_DIRECTION)

// Where to put the text
#define OT_TEXT (OT_LAST - OT_DIRECTION - OT_DIRECTION - OT_DIRECTION)

// Where to put the mission based darkening semi-trans tile
#define OT_DARKEN (OT_TEXT + OT_DIRECTION)

// Where to put the icons
#define OT_ICONS (OT_TEXT)

// Where to put the set gfx fade up/down tiles
#define OT_GFX (OT_DARKEN - OT_DIRECTION)

} // End of namespace ICB

#endif // #ifndef PSX_OT_H
