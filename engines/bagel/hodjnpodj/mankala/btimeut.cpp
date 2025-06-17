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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/mankala/bgen.h"
#include "bagel/hodjnpodj/mankala/btimeut.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

BOOL bTimeDelayPassed = TRUE ;

//* CTimeUtil::DelayMs -- delay for specified # of milliseconds
BOOL CTimeUtil::DelayMs(UINT uMs)
// uMs -- number of milliseconds to delay
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CTimeUtil::DelayMs) ;
	int iError = 0 ;        // error code
	UINT uTimerId ;     // timer id returned by SetTimer
//    MSG FAR * lpMsg ;     // message area storage

//    RETURN(FALSE) ;   // ***************

	if (!(uTimerId = SetTimer(nullptr, 0, uMs, DelayMsCallback)))
		// set timer, and test for success
	{
		iError = 100 ;      // SetTimer failed
		goto cleanup ;
	}

	bTimeDelayPassed = FALSE ;  // time hasn't passed yet
	while (!bTimeDelayPassed)   // loop until flag gets set again
		DoPendingEvents() ;


	if (!KillTimer(nullptr, uTimerId)) { // kill timer and test success
		iError = 101 ;      // KillTimer failed
		goto cleanup ;
	}

cleanup:

	JXELEAVE(CTimeUtil::DelayMs) ;
	RETURN(iError != 0) ;
}

void CTimeUtil::DelayMsCallback(HWND hWnd, UINT uMsg,
                                UINT_PTR uTimerId, DWORD dwTime) {
	JXENTER(::DelayMsCallback) ;
	//int iError = 0 ;      // error code
	bTimeDelayPassed = TRUE ;   // elapsed time passed

	JXELEAVE(::DelayMsCallback) ;
}

void DoPendingEvents() {
	MSG  msg;

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel
