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

#ifndef AGS_ENGINE_MAIN_ENGINE_SETUP_H
#define AGS_ENGINE_MAIN_ENGINE_SETUP_H

#include "ags/shared/util/geometry.h"
#include "ags/engine/gfx/gfx_defines.h"

namespace AGS3 {

// Setup engine after the graphics mode has changed
void engine_post_gfxmode_setup(const Size &init_desktop, const DisplayMode &old_dm);
// Prepare engine for graphics mode release; could be called before switching display mode too
void engine_pre_gfxmode_release();
// Prepare engine to the graphics mode shutdown and gfx driver destruction
void engine_pre_gfxsystem_shutdown();
// Applies necessary changes after screen<->virtual coordinate transformation has changed
void on_coordinates_scaling_changed();
// prepares game screen for rotation setting
void engine_adjust_for_rotation_settings();

} // namespace AGS3

#endif
