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

#include "atari-supervidel.h"

#include "common/scummsys.h"

bool g_hasSuperVidel = false;

#ifdef USE_SUPERVIDEL

#ifdef USE_SV_BLITTER
int g_superVidelFwVersion = 0;
const byte *g_blitMask = nullptr;

static bool isSuperBlitterLocked;

void SyncSuperBlitter() {
	// if externally locked, let the owner decide when to sync (unlock)
	if (isSuperBlitterLocked)
		return;

	// while FIFO not empty...
	if (g_superVidelFwVersion >= 9)
		while (!(*SV_BLITTER_FIFO & 1));
	// while busy blitting...
	while (*SV_BLITTER_CONTROL & 1);
}
#endif	// USE_SV_BLITTER

void LockSuperBlitter() {
#ifdef USE_SV_BLITTER
	assert(!isSuperBlitterLocked);

	isSuperBlitterLocked = true;
#endif
}

void UnlockSuperBlitter() {
#ifdef USE_SV_BLITTER
	assert(isSuperBlitterLocked);

	isSuperBlitterLocked = false;
	if (g_hasSuperVidel)
		SyncSuperBlitter();
#endif
}

#endif	// USE_SUPERVIDEL
