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

#include "hollywood/gameplay/frankenstein_reward.h"

namespace Hollywood {

const byte kFrankensteinRewardFrameMap[kFrankensteinRewardFrameCount] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 4, 8, 9, 10, 9, 8, 4, 3, 2, 1, 0
};

const byte kFrankensteinPartItems[kFrankensteinPartCount] = {
	0x30, 0x42, 0x4c
};

} // End of namespace Hollywood
