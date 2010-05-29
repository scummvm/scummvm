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

#ifndef BACKENDS_SDL_SUBSYS_TIMER_H
#define BACKENDS_SDL_SUBSYS_TIMER_H

#include "backends/base-subsys-timer.h"

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

class SdlSubSys_Timer : public BaseSubSys_Timer {
public:
	SdlSubSys_Timer();
	~SdlSubSys_Timer();

	virtual void timerInit(OSystem *mainSys);
	virtual void timerDone();

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	virtual void getTimeAndDate(TimeDate &t) const;
	virtual Common::TimerManager *getTimerManager();

protected:
	bool _inited;

	SDL_TimerID _timerID;
	Common::TimerManager *_timer;

private:
	OSystem *_mainSys;
};


#endif
