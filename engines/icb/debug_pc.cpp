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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/keyboard.h"
#include "engines/icb/debug.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/session.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/mouse.h"
#include "engines/icb/p4.h"

#include "common/textconsole.h"

namespace ICB {

bool8 zdebug = false;

// For the overloads of new & delete
extern unsigned int newMemAlloc;
extern unsigned int newMemUsed;
extern unsigned int newNptrs;

void Fatal_error(const char *format...) {
	char buf[256];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 256, const_cast<char *>(format), args);
	error("%s", buf);
}
void Message_box(const char *, ...) { ; }
void Zdebug(const char *, ...) { ; }
void Zdebug(uint32, const char *, ...) { ; }
void Tdebug(const char *, const char *, ...) { ; }
void Whap_test() { ; }
void Display_prop_stats() { ; }
void Headup_debug_switcher() { ; }
void Reset_headup_switcher() { ; }
void _game_session::Display_mega_times() { ; }

} // End of namespace ICB
