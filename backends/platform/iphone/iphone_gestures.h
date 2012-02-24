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

#include "backends/keymapper/hardware-key.h"

#ifdef ENABLE_KEYMAPPER

#ifndef BACKENDS_PLATFORM_IPHONE_IPHONE_GESTURES_H
#define BACKENDS_PLATFORM_IPHONE_IPHONE_GESTURES_H

namespace Common {

enum IphoneGestureCode {
	kIphoneGestureOneFingerSwipeUp = 1,
	kIphoneGestureOneFingerSwipeDown = 2,
	kIphoneGestureOneFingerSwipeLeft = 3,
	kIphoneGestureOneFingerSwipeRight = 4,
	kIphoneGestureOneFingerHoldSecondFingerSwipeUp = 5,
	kIphoneGestureOneFingerHoldSecondFingerSwipeDown = 6,
	kIphoneGestureOneFingerHoldSecondFingerSwipeLeft = 7,
	kIphoneGestureOneFingerHoldSecondFingerSwipeRight = 8,
	kIphoneGestureInvalid = 0
};

static const GestureTableEntry iphoneGestures[] = {
	{"1HOLD2SWIPEDOWN", kIphoneGestureOneFingerHoldSecondFingerSwipeDown, "Hold Swipe Down"},
	{0, kIphoneGestureInvalid, 0}
};

} //namespace common

#endif // #ifndef BACKENDS_PLATFORM_IPHONE_IPHONE_GESTURES_H
#endif // #ifdef ENABLE_KEYMAPPER


