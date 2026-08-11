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

#ifndef SCUMM_HE_BASKETBALL_GEO_TRANSLATIONS_H
#define SCUMM_HE_BASKETBALL_GEO_TRANSLATIONS_H

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"

namespace Scumm {

#define BB_SCREEN_SIZE_Y            640     // Please note, this is intentional! It shouldn't be 480!

#define COURT_X_OFFSET              18      // Pixels from left of screen to beginning of court
#define COURT_Y_OFFSET              33      // Pixels from bottom of screen to beginning of court

#define TRANSLATED_MAX_Y            302     // Pixels from bottom of court to top of court
#define TRANSLATED_MID_Y            186     // Pixels from bottom of the court to center court
#define TRANSLATED_DIAGONAL_Y       431     // Pixel length of the baseline
#define TRANSLATED_MAX_START_X      308     // This is how far over x = 0 is pushed when y = MAX_WORLD_Y
#define TRANSLATED_FAR_MAX_X        950     // Length of the far side of field ( world_y = MAX_WORLD_Y) when translated
#define TRANSLATED_NEAR_MAX_X       1564    // Length of near side of field (world_y = MAX_WORLD_Y when translated

// Length and width of the court in game world units
#define WORLD_UNIT_MULTIPLIER       160
#define MAX_WORLD_X                 (75 * WORLD_UNIT_MULTIPLIER)
#define MAX_WORLD_Y                 (50 * WORLD_UNIT_MULTIPLIER)

// Coordinates of the center of the hoop
#define BASKET_PUSH_BACK_DIST (0.5 * WORLD_UNIT_MULTIPLIER)
 
#define BASKET_X              (int)((5.25 * WORLD_UNIT_MULTIPLIER) - BASKET_PUSH_BACK_DIST)
#define BASKET_Y              (25 * WORLD_UNIT_MULTIPLIER)
#define BASKET_Z              (10 * WORLD_UNIT_MULTIPLIER)

// The screen coordinate that sandwich where on the screen scaling occurs;
// these coordinates are given in pixels from the bottom of the court
#define TOP_SCALING_PIXEL_CUTOFF    (1000 - COURT_Y_OFFSET)
#define BOTTOM_SCALING_PIXEL_CUTOFF (0 - COURT_Y_OFFSET)

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_GEO_TRANSLATIONS_H
