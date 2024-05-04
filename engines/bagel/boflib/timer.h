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
 * along with this program.  If not, see <http://www.gnu.org/licenses/".
 *
 */

#ifndef BAGEL_BOFLIB_TIMER_H
#define BAGEL_BOFLIB_TIMER_H

#include "bagel/boflib/timer.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/gfx/palette.h"

namespace Bagel {

struct WindowTimer {
	uint32 _interval = 0;
	uint32 _lastExpiryTime = 0;
	uint32 _id = 0;
	BofCallback _callback = nullptr;

	WindowTimer() { }
	WindowTimer(uint32 interval, uint32 id, BofCallback callback);
};


class CBofTimer: public CBofObject, public CLList {
public:
	CBofTimer();
	CBofTimer(uint32 nID, uint32 nInterval, void *lUserInfo, BofCallback pCallBack);
	~CBofTimer();

	void start()                    {
		_bActive = true;
	}
	void stop()                     {
		_bActive = false;
	}

	bool isActive()                 {
		return _bActive;
	}

	void setID(uint32 nID)                {
		_nID = nID;
	}
	uint32 getID()                    {
		return _nID;
	}

	void setInterval(uint32 nInterval)    {
		_nInterval = nInterval;
	}
	uint32 getInterval()              {
		return _nInterval;
	}

	void setUserInfo(void *lUserInfo)   {
		_lUserInfo = lUserInfo;
	}
	void *getUserInfo()             {
		return _lUserInfo;
	}

	void setCallBack(BofCallback pCallBack) {
		_pCallBack = pCallBack;
	}
	BofCallback getCallBack()       {
		return _pCallBack;
	}

	static void handleTimers();

	//
	// members
	//

protected:

	static CBofTimer *_pTimerList;
	static bool _bModified;

public:

	uint32       _lLastTime;
	uint32        _nID;
	uint32        _nInterval;
	BofCallback _pCallBack;
	void       *_lUserInfo;
	bool        _bActive;
};

} // namespace Bagel

#endif
