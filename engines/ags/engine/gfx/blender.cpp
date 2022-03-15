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

#include "ags/engine/gfx/blender.h"
#include "ags/lib/allegro.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

void set_my_trans_blender(int r, int g, int b, int a) {
	// use standard allegro 15 and 16 bit blenders, but customize
	// the 32-bit one to preserve the alpha channel
	set_blender_mode(kAlphaPreservedBlenderMode, r, g, b, a);
}

void set_additive_alpha_blender() {
	set_blender_mode(kAdditiveBlenderMode, 0, 0, 0, 0);
}

void set_argb2argb_blender(int alpha) {
	set_blender_mode(kArgbToArgbBlender, 0, 0, 0, alpha);
}

void set_opaque_alpha_blender() {
	set_blender_mode(kOpaqueBlenderMode, 0, 0, 0, 0);
}

void set_argb2any_blender() {
	// TODO: Properly implement this new mode
	set_blender_mode(kArgbToArgbBlender, 0, 0, 0, 0xff);
}

} // namespace AGS3
