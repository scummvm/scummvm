// btimeut.cpp -- Boffo Games time utilities -- class CTimeUtil
// Written by John J. Xenakis, 1994, for Boffo Games

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "copyrite.h"	// mandatory internal copyright notice

#include "btimeut.h"

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
//    MSG FAR * lpMsg ;		// message area storage

//    RETURN(FALSE) ;	// ***************

    if ( !(uTimerId = SetTimer(NULL, 0, uMs, ::DelayMsCallback)) )
    			// set timer, and test for success
    {
	iError = 100 ;		// SetTimer failed
	goto cleanup ;
    }

    bTimeDelayPassed = FALSE ;	// time hasn't passed yet
    while (!bTimeDelayPassed)	// loop until flag gets set again
	DoPendingEvents() ;


    if (!KillTimer(NULL, uTimerId))	// kill timer and test success
    {
	iError = 101 ;		// KillTimer failed
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

   while ( PeekMessage(&msg, NULL,0,0,PM_REMOVE) )
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}
}	/* extern "C" */

#if 0

////* CMnkWindow::FlushInputEvents --
PRIVATE void CMnkWindow::FlushInputEvents(void)
{
    MSG msg ;

    // find and remove all keyboard events
    while(TRUE)
    {
	if (!PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE))
	    break ;
    }

    while(TRUE)
    {    // find and remove all mouse events
	if (!PeekMessage(&msg,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE))
	    break ;
    }
}

#endif


