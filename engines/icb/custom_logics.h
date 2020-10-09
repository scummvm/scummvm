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

namespace ICB {

#define SA_INDEX L->list[0]

#define BOD_OPENING 1
#define BOD_CLOSING 2
#define BOD_WAITING 3
#define BOD_WAIT_COUNT L->list[4]
#define BOD_STATE L->list[5]
#define BOD_STATE_INDEX 5
#define BOD_CONTROL L->list[6]
// animation index
#define BOD_INDEX L->list[7]
#define BOD_OPEN_NO L->list[8]
#define BOD_CLOSE_NO L->list[9]

#define CAD_OPEN 0
#define CAD_CLOSED 1
#define CAD_OPENING 2
#define CAD_CLOSING 3

#define EXT_CAD_STATE list[0]
#define CAD_STATE L->list[0]
#define CAD_STATE_INDEX 0

#define CAD_DIST L->list[1]

// animation index
#define CAD_INDEX L->list[2]
#define CAD_OPEN_NO L->list[3]
#define CAD_CLOSE_NO L->list[4]
#define CAD_LOCKED L->list[5]
#define CAD_WAIT L->list[6]

#define CAD_TIMER 48

} // End of namespace ICB
