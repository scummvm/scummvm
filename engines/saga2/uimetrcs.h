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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_UIMETRCS_H
#define SAGA2_UIMETRCS_H

namespace Saga2 {

const int           kIconWidth = 32,
                    kIconHeight = 32,
                    kIconSpacingX = 14,
                    kIconSpacingY = 14,
                    kIconOriginX = kIconWidth / 2,
                    kIconOriginY = kIconHeight / 2;

const int           kBackWidth = 42,
                    kBackHeight = 42,
                    kBackSpacingX = 12,
                    kBackSpacingY = 12,
                    kBackOriginX = kBackWidth / 4,
                    kBackOriginY = kBackHeight / 4 - 8;

// control button position defines
const   int16   kNumButtons      = 7;
const   int16   kYContOffset     = 150;

// facial button position defines
const  int16  kYFaceOffset       = 150;

extern Rect16 julFrameBox, phiFrameBox,
       kevFrameBox, julPlateBox,
       phiPlateBox, kevPlateBox;

} // end of namespace Saga2

#endif
