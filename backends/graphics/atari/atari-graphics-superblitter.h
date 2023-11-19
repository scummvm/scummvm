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

#ifndef BACKENDS_GRAPHICS_ATARI_SUPERBLITTER_H
#define BACKENDS_GRAPHICS_ATARI_SUPERBLITTER_H

#ifdef USE_SUPERVIDEL

#include <mint/cookie.h>
#include <mint/falcon.h>

// bits 9:0
#define SV_VERSION	((volatile long*)0x8001007C)

inline static bool hasSuperVidel() {
	// this works also on the TT
	static bool hasSuperVidel = Getcookie(C_SupV, NULL) == C_FOUND && VgetMonitor() == MON_VGA;
	return hasSuperVidel;
}

static int superVidelFwVersion = hasSuperVidel() ? *SV_VERSION & 0x01ff : 0;

#else

constexpr bool hasSuperVidel() {
	return false;
}

constexpr int superVidelFwVersion = 0;

#endif	// USE_SUPERVIDEL

void lockSuperBlitter();
void unlockSuperBlitter();

#endif
