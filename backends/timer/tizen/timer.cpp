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

#if defined(TIZEN)

#include "backends/timer/tizen/timer.h"

//
// TimerSlot - an event driven thread
//
TimerSlot::TimerSlot(Common::TimerManager::TimerProc callback, uint32 interval, void *refCon) :
	_timer(0),
	_callback(callback),
	_interval(interval),
	_refCon(refCon) {
}

TimerSlot::~TimerSlot() {
	delete _timer;
}

bool TimerSlot::OnStart() {
	_timer = new Tizen::Base::Runtime::Timer();
	if (!_timer || IsFailed(_timer->Construct(*this))) {
		AppLog("Failed to create timer");
		return false;
	}

	if (IsFailed(_timer->StartAsRepeatable(_interval))) {
		AppLog("failed to start timer");
		return false;
	}

	AppLog("started timer %d", _interval);
	return true;
}

void TimerSlot::OnStop() {
	AppLog("timer stopped");
	if (_timer) {
		_timer->Cancel();
		delete _timer;
		_timer = NULL;
	}
}

void TimerSlot::OnTimerExpired(Timer &timer) {
	_callback(_refCon);
}

//
// TizenTimerManager
//
TizenTimerManager::TizenTimerManager() {
}

TizenTimerManager::~TizenTimerManager() {
	for (Common::List<TimerSlot *>::iterator it = _timers.begin(); it != _timers.end(); ) {
		TimerSlot *slot = (*it);
		slot->Quit();
		slot->Join();
		delete slot;
		it = _timers.erase(it);
	}
}

bool TizenTimerManager::installTimerProc(TimerProc proc, int32 interval, void *refCon, const Common::String &id) {
	TimerSlot *slot = new TimerSlot(proc, interval / 1000, refCon);

	if (IsFailed(slot->Construct())) {
		AppLog("Failed to create timer thread");
		delete slot;
		return false;
	}

	if (IsFailed(slot->Start())) {
		delete slot;
		AppLog("Failed to start timer thread");
		return false;
	}

	_timers.push_back(slot);
	return true;
}

void TizenTimerManager::removeTimerProc(TimerProc proc) {
	for (Common::List<TimerSlot *>::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		TimerSlot *slot = (*it);
		if (slot->_callback == proc) {
			slot->Quit();
			slot->Join();
			delete slot;
			it = _timers.erase(it);
		}
	}
}

#endif
