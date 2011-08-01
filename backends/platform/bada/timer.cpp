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

#include "timer.h"
#include "system.h"

//
// TimerSlot
//
TimerSlot::TimerSlot(Common::TimerManager::TimerProc callback,
                     uint32 interval, void* refCon) :
  timer(0),
  callback(callback),
  interval(interval),
  refCon(refCon) {
  logEntered();
}

TimerSlot::~TimerSlot() {
  logEntered();
  if (timer) {
    delete timer;
    timer = null;
  }
}

bool TimerSlot::OnStart() {
  logEntered();

  timer = new Osp::Base::Runtime::Timer();
  if (!timer || IsFailed(timer->Construct(*this))) {
    AppLog("Failed to create timer");
    return false;
  }

  if (IsFailed(timer->Start(interval))) {
    AppLog("failed to start timer");
    return false;
  }
  
  AppLog("started timer %d", interval);
  return true;
}

void TimerSlot::OnStop() {
  logEntered();
  if (timer) {
    timer->Cancel();    
    delete timer;
    timer = null;
  }
}

void TimerSlot::OnTimerExpired(Timer& timer) {
  callback(refCon);
  timer.Start(interval);
}

//
// BadaTimerManager
//
BadaTimerManager::BadaTimerManager() {
  logEntered();
}

BadaTimerManager::~BadaTimerManager() {
  logEntered();
	for (Common::List<TimerSlot>::iterator slot = timers.begin();
       slot != timers.end(); ++slot) {
    slot->Stop();
    slot = timers.erase(slot);
  }
}

bool BadaTimerManager::installTimerProc(TimerProc proc, int32 interval, void *refCon) {
  logEntered();
  TimerSlot* slot = new TimerSlot(proc, interval / 1000, refCon);

  if (IsFailed(slot->Construct(THREAD_TYPE_EVENT_DRIVEN))) {
    AppLog("Failed to create timer thread");
    delete slot;
    return false;
  }

  if (IsFailed(slot->Start())) {
    delete slot;
    AppLog("Failed to start timer thread");
    return false;
  }

  timers.push_back(*slot);
  return true;
}

void BadaTimerManager::removeTimerProc(TimerProc proc) {
  logEntered();
	for (Common::List<TimerSlot>::iterator slot = timers.begin();
       slot != timers.end(); ++slot) {
    if (slot->callback == proc) {
      slot->Stop();
      slot = timers.erase(slot);
    }
  }
}

//
// end of timer.cpp 
//
