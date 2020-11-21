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

#ifndef AGS_ENGINE_MAIN_MAINHEADER_H
#define AGS_ENGINE_MAIN_MAINHEADER_H

#include "core/platform.h"

#include "main/maindefines_ex.h"

#include "ac/math.h"
#include "script/script_runtime.h"
#include "gui/animatingguibutton.h"
#include "gui/guibutton.h"
#include "gfx/gfxfilter.h"
#include "util/string_utils.h"
#include "device/mousew32.h"
#include "ac/route_finder.h"
#include "util/misc.h"
#include "script/cc_error.h"

// include last since we affect windows includes
#include "ac/file.h"

#if AGS_PLATFORM_OS_ANDROID
#include <sys/stat.h>
#include <android/log.h>

extern "C" void selectLatestSavegame();
extern bool psp_load_latest_savegame;
#endif

#endif
