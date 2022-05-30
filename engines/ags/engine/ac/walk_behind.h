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

// Walk-behinds calculation logic.

#ifndef AGS_ENGINE_AC_WALK_BEHIND_H
#define AGS_ENGINE_AC_WALK_BEHIND_H

#include "ags/shared/util/geometry.h"

namespace AGS3 {

// A method of rendering walkbehinds on screen:
// DrawAsSeparateSprite - draws whole walkbehind as a sprite;
//     this method is most simple and is optimal for 3D renderers.
// DrawOverCharSprite - turns parts of the character and object sprites
//     transparent when they are covered by walkbehind (walkbehind itself
//     is not drawn separately in this case);
//     this method is optimized for software render.
enum WalkBehindMethodEnum {
	DrawOverCharSprite,
	DrawAsSeparateSprite
};

// An info on vertical column of walk-behind mask, which may contain WB area
struct WalkBehindColumn {
	bool Exists = false; // whether any WB area is in this column
	int Y1 = 0, Y2 = 0; // WB top and bottom Y coords
};

namespace AGS { namespace Shared { class Bitmap; } }
using namespace AGS; // FIXME later

// Recalculates walk-behind positions
void walkbehinds_recalc();
// Generates walk-behinds as separate sprites
void walkbehinds_generate_sprites();
// Edits the given game object's sprite, cutting out pixels covered by walk-behinds;
// returns whether any pixels were updated
bool walkbehinds_cropout(Shared::Bitmap *sprit, int sprx, int spry, int basel, int zoom = 100);

} // namespace AGS3

#endif
