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

#ifndef AGS_ENGINE_AC_SCREEN_H
#define AGS_ENGINE_AC_SCREEN_H

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

namespace AGS {
namespace Engine {
class IDriverDependantBitmap;
} // namespace Engine
} // namespace AGS

void my_fade_in(PALETTE p, int speed);
void current_fade_out_effect();
AGS::Engine::IDriverDependantBitmap *prepare_screen_for_transition_in();

// Screenshot made in the last room, used during some of the transition effects
extern AGS::Shared::Bitmap *saved_viewport_bitmap;

} // namespace AGS3

#endif
