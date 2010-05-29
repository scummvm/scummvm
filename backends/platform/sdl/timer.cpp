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
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/sdl/timer.h"
#include "backends/timer/default/default-timer.h"
#include "common/EventRecorder.h"
#include <time.h>	// for getTimeAndDate()

static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

SdlSubSys_Timer::SdlSubSys_Timer()
	:
	_inited(false),
	_timerID(),
	_timer(0) {

}

SdlSubSys_Timer::~SdlSubSys_Timer() {
	if (_inited) {
		timerDone();
	}
}

void SdlSubSys_Timer::timerInit(OSystem *mainSys) {
	if (_inited) {
		return;
	}
	_mainSys = mainSys;

	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		error("Could not initialize SDL Timer: %s", SDL_GetError());
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timer == 0) {
		// Note: We could implement a custom SDLTimerManager by using
		// SDL_AddTimer. That might yield better timer resolution, but it would
		// also change the semantics of a timer: Right now, ScummVM timers
		// *never* run in parallel, due to the way they are implemented. If we
		// switched to SDL_AddTimer, each timer might run in a separate thread.
		// However, not all our code is prepared for that, so we can't just
		// switch. Still, it's a potential future change to keep in mind.
		_timer = new DefaultTimerManager();
		_timerID = SDL_AddTimer(10, &timer_handler, _timer);
	}

	_inited = true;
}

void SdlSubSys_Timer::timerDone() {
	delete _timer;
	SDL_RemoveTimer(_timerID);
	SDL_QuitSubSystem(SDL_INIT_TIMER);

	_inited = false;
}

bool SdlSubSys_Timer::hasFeature(Feature f) {
	return false;
}

void SdlSubSys_Timer::setFeatureState(Feature f, bool enable) {
	
}

bool SdlSubSys_Timer::getFeatureState(Feature f) {
	return false;
}

uint32 SdlSubSys_Timer::getMillis() {
	uint32 millis = SDL_GetTicks();
	g_eventRec.processMillis(millis);
	return millis;
}

void SdlSubSys_Timer::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void SdlSubSys_Timer::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
}

Common::TimerManager *SdlSubSys_Timer::getTimerManager() {
	assert(_timer);
	return _timer;
}
