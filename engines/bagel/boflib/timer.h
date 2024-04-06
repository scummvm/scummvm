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

namespace Bagel {

struct WindowTimer {
	uint32 _interval = 0;
	uint32 _lastExpiryTime = 0;
	UINT _id = 0;
	BOFCALLBACK _callback = nullptr;

	WindowTimer() {
	}
	WindowTimer(uint32 interval, UINT id, BOFCALLBACK callback) :
		_interval(interval), _id(id), _callback(callback),
		_lastExpiryTime(0) {
	}
};


class CBofTimer: public CBofObject, public CLList {
public:
	CBofTimer();
	CBofTimer(UINT nID, UINT nInterval, void *lUserInfo, BOFCALLBACK pCallBack);
	~CBofTimer();

	VOID Start()                    {
		m_bActive = TRUE;
	}
	VOID Stop()                     {
		m_bActive = FALSE;
	}

	BOOL IsActive()                 {
		return (m_bActive);
	}

	VOID SetID(UINT nID)                {
		m_nID = nID;
	}
	UINT GetID()                    {
		return (m_nID);
	}

	VOID SetInterval(UINT nInterval)    {
		m_nInterval = nInterval;
	}
	UINT GetInterval()              {
		return (m_nInterval);
	}

	VOID SetUserInfo(void *lUserInfo)   {
		m_lUserInfo = lUserInfo;
	}
	void *GetUserInfo()             {
		return (m_lUserInfo);
	}

	VOID SetCallBack(BOFCALLBACK pCallBack) {
		m_pCallBack = pCallBack;
	}
	BOFCALLBACK GetCallBack()       {
		return (m_pCallBack);
	}

	static VOID HandleTimers();

	//
	// members
	//

protected:

	static CBofTimer *m_pTimerList;
	static BOOL m_bModified;

public:

	ULONG       m_lLastTime;
	UINT        m_nID;
	UINT        m_nInterval;
	BOFCALLBACK m_pCallBack;
	void       *m_lUserInfo;
	BOOL        m_bActive;
};

} // namespace Bagel

#endif
