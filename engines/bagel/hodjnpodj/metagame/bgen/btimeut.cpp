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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/bgen/btimeut.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

///DEFS btimeut.h
bool bTimeDelayPassed = true ;

void CALLBACK DelayMsCallback(HWND /* hWnd */,
                              unsigned int /* uMsg */, unsigned int /* uTimerId */, uint32 /* dwTime */);


//* CTimeUtil::DelayMs -- delay for specified # of milliseconds
bool CTimeUtil::DelayMs(unsigned int uMs)
// uMs -- number of milliseconds to delay
// returns: true if error, false otherwise
{
	JXENTER(CTimeUtil::DelayMs) ;
	int iError = 0 ;        // error code
	unsigned int uTimerId ;     // timer id returned by SetTimer

	// set timer, and test for success
	if ((uTimerId = SetTimer(nullptr, 0, uMs, DelayMsCallback)) == 0) {
		iError = 100 ;      // SetTimer failed
		goto cleanup ;
	}

	bTimeDelayPassed = false ;  // time hasn't passed yet
	while (!bTimeDelayPassed)   // loop until flag gets set again
		DoPendingEvents() ;

	// kill timer and test success
	if (!KillTimer(nullptr, uTimerId)) {
		iError = 101 ;      // KillTimer failed
		goto cleanup ;
	}

cleanup:

	JXELEAVE(CTimeUtil::DelayMs) ;
	RETURN(iError != 0) ;
}

///* DelayMsCallback -- SetTimer callback routine for DelayMs
void CTimeUtil::DelayMsCallback(HWND /* hWnd */,
                                unsigned int /* uMsg */, uintptr /* uTimerId */, uint32 /* dwTime */)
// hWnd -- handle of window (always nullptr in this case)
// uMsg -- WM_TIMER message
// uTimerId -- timer identifier
// dwTime -- current system time
// returns: void
{
	JXENTER(DelayMsCallback) ;
	bTimeDelayPassed = true ;   // elapsed time passed

	JXELEAVE(DelayMsCallback) ;
}

void DoPendingEvents() {
	MSG  msg;

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
