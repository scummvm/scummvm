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

#ifndef AGS_ENGINE_AC_SPRITE_H
#define AGS_ENGINE_AC_SPRITE_H

#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

// Converts from 32-bit RGBA image, to a 15/16/24-bit destination image,
// replacing more than half-translucent alpha pixels with transparency mask pixels.
Shared::Bitmap *remove_alpha_channel(Shared::Bitmap *from);
Size get_new_size_for_sprite(const Size &size, const uint32_t sprite_flags);
// Initializes a loaded sprite for use in the game, adjusts the sprite flags.
// Returns a resulting bitmap, which may be a new or old bitmap; or null on failure.
// Original bitmap **gets deleted** if a new bitmap had to be created,
// or if failed to properly initialize one.
Shared::Bitmap *initialize_sprite(Shared::sprkey_t index, Shared::Bitmap *image, uint32_t &sprite_flags);
void post_init_sprite(Shared::sprkey_t index);

} // namespace AGS3

#endif
