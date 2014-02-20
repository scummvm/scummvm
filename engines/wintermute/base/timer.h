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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_TIMER_H
#define WINTERMUTE_TIMER_H

#include "common/scummsys.h"

namespace Wintermute {

class BasePersistenceManager;

class Timer {
	uint32 _timer;
	uint32 _timerDelta;
	uint32 _timerLast;
public:
	Timer();
	void setTime(uint32 time);
	void setTimeDelta(uint32 timeDelta);
	void setTimeLast(uint32 timeLast);
	void updateTime(uint32 delta, uint32 max);
	uint32 getTime() const;
	uint32 getTimeDelta() const;
	uint32 getTimeLast() const;
	void persist(BasePersistenceManager *persistMgr);
};


} // End of namespace Wintermute

#endif
