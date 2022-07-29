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

#include "immortal/immortal.h"

namespace Immortal {

/* 
 *
 * -----                -----
 * ----- Main Functions -----
 * -----                -----
 *
 */

void ImmortalEngine::delay(int j) {             // Delay is measured in jiffies, which are 56.17ms
	g_system->delayMillis(j * 56);
}

void ImmortalEngine::delay4(int j) {            // Named in source quarterClock for some reason, 1/4 jiffies are 14.04ms
	g_system->delayMillis(j * 14);
}

void ImmortalEngine::delay8(int j) {            // 1/8 jiffies are 7.02ms
	g_system->delayMillis(j * 7);
}

void ImmortalEngine::miscInit() {}
void ImmortalEngine::setRandomSeed() {}
void ImmortalEngine::getRandom() {}
void ImmortalEngine::myDelay() {}


/* 
 *
 * -----               -----
 * ----- Text Printing -----
 * -----               -----
 *
 */

bool ImmortalEngine::textPrint(const Common::String s) {
    return true;
}
void ImmortalEngine::textSub() {}
void ImmortalEngine::textEnd() {}
void ImmortalEngine::textMiddle() {}
void ImmortalEngine::textBeginning() {}
void ImmortalEngine::yesNo() {}


/* 
 *
 * -----               -----
 * ----- Input Related -----
 * -----               -----
 *
 */

void ImmortalEngine::buttonPressed() {}
void ImmortalEngine::firePressed() {}


/* 
 *
 * -----                -----
 * ----- Screen Related -----
 * -----                -----
 *
 */

void ImmortalEngine::inside(int p, int p2, int a) {}
void ImmortalEngine::insideRect(int p, int r) {}


} // namespace Immortal
























