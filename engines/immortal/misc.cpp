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

void ImmortalEngine::miscInit() {
	// In the source, this is where the seed for the rng is set, but we don't need to do that as we used _randomSource
	_lastGauge = 0;
}

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

bool ImmortalEngine::textPrint(Str s) {
	return true;
}
void ImmortalEngine::textSub() {}
void ImmortalEngine::textEnd(Str s) {}
void ImmortalEngine::textMiddle(Str s) {}
void ImmortalEngine::textBeginning(Str s) {}
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

} // namespace Immortal
























