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

#ifndef AGS_ENGINE_AC_SPRITE_LIST_ENTRY_H
#define AGS_ENGINE_AC_SPRITE_LIST_ENTRY_H

#include "ags/engine/gfx/ddb.h"

namespace AGS3 {

// Describes a texture or node description, for sorting and passing into renderer
struct SpriteListEntry {
	// Optional sprite identifier; used as a second factor when sorting
	int id = -1;
	Engine::IDriverDependantBitmap *ddb = nullptr;
	AGS::Shared::Bitmap *pic = nullptr;
	int x = 0, y = 0;
	int zorder = 0;
	// Mark for the render stage callback (if >= 0 other fields are ignored)
	int renderStage = -1;
};

} // namespace AGS3

#endif
