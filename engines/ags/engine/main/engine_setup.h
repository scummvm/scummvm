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

#ifndef AGS_ENGINE_MAIN_ENGINESETUP_H
#define AGS_ENGINE_MAIN_ENGINESETUP_H

#include "ags/shared/util/geometry.h"
#include "ags/engine/gfx/gfxdefines.h"

namespace AGS3 {

// Sets up game viewport and object scaling parameters depending on game.
// TODO: this is part of the game init, not engine init, move it later
void engine_init_resolution_settings(const Size game_size);
// Setup engine after the graphics mode has changed
void engine_post_gfxmode_setup(const Size &init_desktop);
// Prepare engine for graphics mode release; could be called before switching display mode too
void engine_pre_gfxmode_release();
// Prepare engine to the graphics mode shutdown and gfx driver destruction
void engine_pre_gfxsystem_shutdown();
// Applies necessary changes after screen<->virtual coordinate transformation has changed
void on_coordinates_scaling_changed();

} // namespace AGS3

#endif
