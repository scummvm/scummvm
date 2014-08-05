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

#ifndef ACCESS_GLOBALS_H
#define ACCESS_GLOBALS_H

#include "common/scummsys.h"

namespace Access {

class Globals {
public:
	int _startData;
	int _rawPlayerXLow;
	int _rawPlayerX;
	int _rawPlayerYLow;
	int _rawPlayerY;
	int _conversation;
	int _currentMan;
	uint32 _newTime;
	uint32 _newDate;
	int _intTim[3];
	int _timer[3];
	bool _timerFlag;
	byte _flags[99];
	byte _useItem[23];
	int _guardLoc;
	int _guardFind;
	int _helpLevel;
	int _jasMayaFlag;
	int _moreHelp;
	int _startup;
	bool _flashbackFlag;
	int _manScaleOff;
	bool _riverFlag;
	bool _antOutFlag;
	int _badEnd;
	bool _noHints;
	bool _antFlag;
	bool _allenFlag;
	bool _noSound;
	int inv[85];
	byte _help1[366];
	byte _help2[366];
	byte _help3[366];
	int _travel;
	int _ask;
	int _rScrollRow;
	int _rScrollCol;
	int _rSrcollX;
	int _rScrollY;
	int _rOldRectCount;
	int _rNewRectCount;
	int _rKeyFlag;
	int _mapOffset;
	int _screenVirtX;
public:
	Globals();

	virtual ~Globals() {}
};

} // End of namespace Access

#endif /* ACCESS_GLOBALS_H */
