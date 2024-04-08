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
	uint32 _id = 0;
	BOFCALLBACK _callback = nullptr;

	WindowTimer() {
	}
	WindowTimer(uint32 interval, uint32 id, BOFCALLBACK callback);
};


class CBofTimer: public CBofObject, public CLList {
public:
	CBofTimer();
	CBofTimer(uint32 nID, uint32 nInterval, void *lUserInfo, BOFCALLBACK pCallBack);
	~CBofTimer();

	void Start()                    {
		m_bActive = TRUE;
	}
	void Stop()                     {
		m_bActive = FALSE;
	}

	BOOL IsActive()                 {
		return (m_bActive);
	}

	void SetID(uint32 nID)                {
		m_nID = nID;
	}
	uint32 GetID()                    {
		return (m_nID);
	}

	void SetInterval(uint32 nInterval)    {
		m_nInterval = nInterval;
	}
	uint32 GetInterval()              {
		return (m_nInterval);
	}

	void SetUserInfo(void *lUserInfo)   {
		m_lUserInfo = lUserInfo;
	}
	void *GetUserInfo()             {
		return (m_lUserInfo);
	}

	void SetCallBack(BOFCALLBACK pCallBack) {
		m_pCallBack = pCallBack;
	}
	BOFCALLBACK GetCallBack()       {
		return (m_pCallBack);
	}

	static void HandleTimers();

	//
	// members
	//

protected:

	static CBofTimer *m_pTimerList;
	static BOOL m_bModified;

public:

	uint32       m_lLastTime;
	uint32        m_nID;
	uint32        m_nInterval;
	BOFCALLBACK m_pCallBack;
	void       *m_lUserInfo;
	BOOL        m_bActive;
};

} // namespace Bagel

#endif
