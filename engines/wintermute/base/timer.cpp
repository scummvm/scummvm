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

#include "engines/wintermute/base/timer.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/persistent.h"

namespace Wintermute {
Timer::Timer() : _timer(0), _timerDelta(0), _timerLast(0) {

}

void Timer::setTime(uint32 time) {
	_timer = time;
}

void Timer::setTimeDelta(uint32 timeDelta) {
	_timerDelta = timeDelta;
}

void Timer::setTimeLast(uint32 timeLast) {
	_timerLast = timeLast;
}

void Timer::updateTime(uint32 delta, uint32 max) {
	_timerDelta = _timer - _timerLast;
	_timerLast = _timer;
	_timer += MIN((uint32)max, delta);
}

uint32 Timer::getTime() const {
	return _timer;
}

uint32 Timer::getTimeDelta() const {
	return _timerDelta;
}

uint32 Timer::getTimeLast() const {
	return _timerLast;
}

void Timer::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferUint32(TMEMBER(_timer));
	persistMgr->transferUint32(TMEMBER(_timerDelta));
	persistMgr->transferUint32(TMEMBER(_timerLast));
}

} // End of namespace Wintermute
