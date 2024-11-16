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

#ifndef PLATFORM_ATARI_DEBUG_H
#define PLATFORM_ATARI_DEBUG_H

#include "common/debug.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

#ifdef DISABLE_TEXT_CONSOLE

void atari_debug(const char *s, ...);
#define atari_warning atari_debug

#else

#define atari_debug debug
#define atari_warning warning

#endif

#endif
