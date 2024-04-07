/*----------------------------------------------------------------------
FILE        :   MSJDBG.h
FROM        :   "BugSlayer" Microsoft Systems Journal, Oct. 97
                John Robbins
HISTORY     :
    Feb. 98 column - Added TRACE macros.
    Feb. 99 column - Removed TRACE/ASSERT macros.  Use the new
                     DiagAssert ASSERT/SUPERASSERT macros.
----------------------------------------------------------------------*/

#ifndef _MSJDBG_H
#define _MSJDBG_H

// Force the include of the debug run-time header file and make sure the
//  allocation mapping functions are in effect.
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#endif      // _MSJDBG_H


