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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_screenshakereffects.h"

#include "engines/util.h"

namespace Illusions {

static const ScreenShakerPoint kShakerPoints0[] = {
	{0, -2}, {0, -4}, {0, -3}, {0, -1}, {0, 1}
};

static const ScreenShakeEffect kShakerEffect0 = {
	ARRAYSIZE(kShakerPoints0), 5, kShakerPoints0
};

static const ScreenShakerPoint kShakerPoints1[] = {
	{-4, -5}, {4,  5}, {-3, -4}, {3, 4}, {-2, -3}, {2, 3}, {-1, -2},
	{ 1,  2}, {0, -1}
};

static const ScreenShakeEffect kShakerEffect1 = {
	ARRAYSIZE(kShakerPoints1), 2, kShakerPoints1
};

static const ScreenShakerPoint kShakerPoints2[] = {
	{0, -3}, {0,  3}, {0, -2}, {0, 2}, {0, -2}, {0, 2}, {0, -1},
	{0,  1}, {0, -1},
};

static const ScreenShakeEffect kShakerEffect2 = {
	ARRAYSIZE(kShakerPoints2), 2, kShakerPoints2
};

static const ScreenShakerPoint kShakerPoints3[] = {
	{0, 1}, {0, -1}, {0, -2}, {0, 0}, {(int16)32768, 0}
};

static const ScreenShakeEffect kShakerEffect3 = {
	ARRAYSIZE(kShakerPoints3), 2, kShakerPoints3
};

static const ScreenShakerPoint kShakerPoints4[] = {
	{0, 4}, {0, -1}, {0, 3}, {0, -2}, {0, 1}, {0, -1}, {0, 1}, {0, -1}
};

static const ScreenShakeEffect kShakerEffect4 = {
	ARRAYSIZE(kShakerPoints4), 5, kShakerPoints4
};

static const ScreenShakerPoint kShakerPoints5[] = {
	{0, -1}, {0, 0}, {0, 1}, {0, 0}, {0, -1}, {0, 0}, {0, 1}, {0, 0},
	{0, -1}, {0, 0}, {0, 1}, {0, 0}, {0, -1}, {0, 0}, {0, 1}, {0, 0},
	{0, -1}, {0, 0}, {0, 1}, {0, 0}, {0, -1}, {0, 0}, {0, 1}, {0, 0},
	{0, -1}, {0, 0}, {0, 1}, {0, 0}, {0, -1}, {0, 0}, {0, 1}, {0, 0}
};

static const ScreenShakeEffect kShakerEffect5 = {
	ARRAYSIZE(kShakerPoints5), 2, kShakerPoints5
};

static const ScreenShakeEffect *kShakerEffects[] = {
	&kShakerEffect0,
	&kShakerEffect1,
	&kShakerEffect2,
	&kShakerEffect3,
	&kShakerEffect4,
	&kShakerEffect5
};

const ScreenShakeEffect *getScreenShakeEffect(byte index) {
	return kShakerEffects[index];
}

} // End of namespace Illusions
