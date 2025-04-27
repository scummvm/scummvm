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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"

#include "btimeut.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

///DEFS btimeut.h

// extern "C" void WaitEvent(void) ;

BOOL bTimeDelayPassed = TRUE ;

void CALLBACK EXPORT DelayMsCallback(HWND /* hWnd */,
	UINT /* uMsg */, UINT /* uTimerId */, DWORD /* dwTime */) ;


//* CTimeUtil::DelayMs -- delay for specified # of milliseconds
BOOL CTimeUtil::DelayMs(UINT uMs)
// uMs -- number of milliseconds to delay
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CTimeUtil::DelayMs) ;
    int iError = 0 ;		// error code
    UINT uTimerId ;		// timer id returned by SetTimer

    // set timer, and test for success
    if ((uTimerId = SetTimer(NULL, 0, uMs, ::DelayMsCallback)) == 0) {
        iError = 100 ;      // SetTimer failed
        goto cleanup ;
    }

    bTimeDelayPassed = FALSE ;	// time hasn't passed yet
    while (!bTimeDelayPassed)	// loop until flag gets set again
        DoPendingEvents() ;

    // kill timer and test success
    if (!KillTimer(NULL, uTimerId)) {
        iError = 101 ;      // KillTimer failed
        goto cleanup ;
    }

cleanup:

    JXELEAVE(CTimeUtil::DelayMs) ;
    RETURN(iError != 0) ;
}

///* ::DelayMsCallback -- SetTimer callback routine for DelayMs
void CALLBACK EXPORT ::DelayMsCallback(HWND /* hWnd */,
		UINT /* uMsg */, UINT /* uTimerId */, DWORD /* dwTime */)
// hWnd -- handle of window (always NULL in this case)
// uMsg -- WM_TIMER message
// uTimerId -- timer identifier
// dwTime -- current system time
// returns: void
{
    JXENTER(::DelayMsCallback) ;
    int iError = 0 ;		// error code
    bTimeDelayPassed = TRUE ;	// elapsed time passed

    JXELEAVE(::DelayMsCallback) ;
}

extern "C" {
void DoPendingEvents()
{
   MSG  msg;

   while ( PeekMessage(&msg, NULL,0,0,PM_REMOVE) ) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}
}	/* extern "C" */

#if 0

////* CMnkWindow::FlushInputEvents --
void CMnkWindow::FlushInputEvents(void)
{
    MSG msg ;

    // find and remove all keyboard events
    while(TRUE) {
        if (!PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE))
            break ;
    }

    while(TRUE) {
        // find and remove all mouse events
        if (!PeekMessage(&msg,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE))
            break ;
    }
}

#endif

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
