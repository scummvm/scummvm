// btimeut.h -- header file for time utilities
// Written by John J. Xenakis, 1994, for Boffo Games


#ifndef __btimeut_H__
#define __btimeut_H__


#include <time.h>
//#include "globals.h"
//#include "resource.h"
//#include "dibdoc.h"
//#include "bagel/hodjnpodj/hnplibs/sprite.h"

#include "bgen.h"

#include "hhead.h"

extern "C" void DoPendingEvents() ;

// class CTimeUtil -- general utilities
class CTimeUtil {

    char m_cStartData ;
    BOOL m_bTimeDelayPassed ;	// flag: DelayMs function time expired


    char m_cEndData ;

public:

    // constructor zeroes out all fields
    CTimeUtil() {memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;}


// btimeut.cpp -- Boffo Games time utilities -- class CTimeUtil

//- CTimeUtil::DelayMs -- delay for specified # of milliseconds
BOOL CTimeUtil::DelayMs(UINT uMs) ;
//- CTimeUtil::DelayMsCallback -- SetTimer callback routine for DelayMs
void __export FAR PASCAL CTimeUtil::DelayMsCallback(HWND /* hWnd */,
		UINT /* uMsg */, UINT /* uTimerId */, DWORD /* dwTime */) ;



} ;

#include "htail.h"

#endif // __btimeut_H__


