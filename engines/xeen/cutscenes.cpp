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

#include "xeen/cutscenes.h"
#include "xeen/xeen.h"

namespace Xeen {

uint Cutscenes::getSpeakingFrame(uint minFrame, uint maxFrame) {
	uint interval = g_system->getMillis() / 100;
	return minFrame + interval % (maxFrame + 1 - minFrame);
}

bool Cutscenes::doScroll(bool rollUp, bool fadeIn) {
	return _vm->_screen->doScroll(rollUp, fadeIn);
}

} // End of namespace Xeen
