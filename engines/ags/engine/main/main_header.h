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

#ifndef AGS_ENGINE_MAIN_MAIN_HEADER_H
#define AGS_ENGINE_MAIN_MAIN_HEADER_H

#include "ags/shared/core/platform.h"
#include "ags/engine/main/main_defines_ex.h"
#include "ags/engine/ac/math.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/util/string_utils.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/shared/script/cc_error.h"

// include last since we affect windows includes
#include "ags/engine/ac/file.h"

#if AGS_PLATFORM_OS_ANDROID
//include <sys/stat.h>
//include <android/log.h>

namespace AGS3 {
extern "C" void selectLatestSavegame();
extern bool psp_load_latest_savegame;
} // namespace AGS3
#endif

#endif
