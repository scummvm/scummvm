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

#ifndef BAGEL_BOFLIB_BOF_TIMER_H
#define BAGEL_BOFLIB_BOF_TIMER_H

#include "bagel/boflib/bof_timer.h"
#include "bagel/boflib/llist.h"

namespace Bagel {

class CBofTimer: public CBofObject, public CLList {

    public:
        CBofTimer();
        CBofTimer(UINT nID, UINT nInterval, ULONG lUserInfo, BOFCALLBACK pCallBack);
        ~CBofTimer();

        VOID Start(VOID)                    { m_bActive = TRUE; }
        VOID Stop(VOID)                     { m_bActive = FALSE; }

        BOOL IsActive(VOID)                 { return(m_bActive); }

        VOID SetID(UINT nID)                { m_nID = nID; }
        UINT GetID(VOID)                    { return(m_nID); }

        VOID SetInterval(UINT nInterval)    { m_nInterval = nInterval; }
        UINT GetInterval(VOID)              { return(m_nInterval); }

        VOID SetUserInfo(ULONG lUserInfo)   { m_lUserInfo = lUserInfo; }
        ULONG GetUserInfo(VOID)             { return(m_lUserInfo); }

        VOID SetCallBack(BOFCALLBACK pCallBack) { m_pCallBack = pCallBack; }
        BOFCALLBACK GetCallBack(VOID)       { return(m_pCallBack); }

        static VOID HandleTimers(VOID);

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
        ULONG       m_lUserInfo;
        BOOL        m_bActive;
};

} // namespace Bagel

#endif
