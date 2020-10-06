/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_D_DEBUG
#define ICB_D_DEBUG

#include "engines/icb/p4_generic.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/debug_pc.h"

namespace ICB {

extern bool8 terminate_debugging;

// This flag indicates whether or not the debug simulated feature is activated or not.
extern bool8 debug_auto_save;

// And this controls how frequently the autosave gets done.
#define DEBUG_AUTO_SAVE_SKIP_CYCLES 10

#define EXCEPTION_LOG "exception_log.txt"

#define PXTRY
#define PXCATCH if (0) {
#define PXENDCATCH }

// headup switch stub mode
void Headup_debug_switcher();
void Reset_headup_switcher();

} // End of namespace ICB

#endif // #ifndef D_DEBUG
