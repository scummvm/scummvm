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
#include "access/globals.h"

namespace Access {

Globals::Globals() {
	_startData = 0;
	_rawPlayerXLow = 0;
	_rawPlayerX = 0;
	_rawPlayerYLow = 0;
	_rawPlayerY = 0;
	_conversation = 0;
	_currentMan = 0;
	_newTime = 0;
	_newDate = 0;
	_intTim[3] = 0;
	_timer[3] = 0;
	_timerFlag = false;
	Common::fill(&_flags[0], &_flags[99], 0);
	Common::fill(&_useItem[0], &_useItem[23], 0);
	_guardLoc = 0;
	_guardFind = 0;
	_helpLevel = 0;
	_jasMayaFlag = 0;
	_moreHelp = 0;
	_startup = 0;
	_flashbackFlag = false;
	_manScaleOff = 0;
	_riverFlag = false;
	_antOutFlag = false;
	_badEnd = 0;
	_noHints = false;
	_antFlag = false;
	_allenFlag = false;
	_noSound = false;
	Common::fill(&inv[0], &inv[85], 0);
	Common::fill(&_help1[0], &_help1[366], 0);
	Common::fill(&_help2[0], &_help2[366], 0);
	Common::fill(&_help1[0], &_help3[366], 0);
	_travel = 0;
	_ask = 0;
	_rScrollRow = 0;
	_rScrollCol = 0;
	_rSrcollX = 0;
	_rScrollY = 0;
	_rOldRectCount = 0;
	_rNewRectCount = 0;
	_rKeyFlag = 0;
	_mapOffset = 0;
	_screenVirtX = 0;
}

} // End of namespace Access
