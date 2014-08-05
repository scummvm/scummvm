/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software = 0; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation = 0; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY = 0; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program = 0; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "access/amazon/amazon_globals.h"

namespace Access {

namespace Amazon {

AmazonGlobals::AmazonGlobals() : Globals() {
	_canoeLane = 0;
	_canoeYPos = 0;
	_hitCount = 0;
	_saveRiver = 0;
	_hitSafe = 0;
	_chapter = 0;
	_topList = 0;
	_botList = 0;
	_riverIndex = 0;
	_rawInactiveX = 0;
	_rawInactiveY = 0;
	_inactiveYOff = 0;
	Common::fill(&_esTabTable[0], &_esTabTable[100], 0);
}

} // End of namespace Amazon

} // End of namespace Access
