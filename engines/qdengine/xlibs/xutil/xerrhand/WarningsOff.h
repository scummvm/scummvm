/*----------------------------------------------------------------------
       John Robbins - Microsoft Systems Journal Bugslayer Column
------------------------------------------------------------------------
FILE        :   WarningsOff.h
DISCUSSION  :
    Turns off warnings so standard headers will compile.  Any warning
turned off, must be turned back in in WarningsOn.h
----------------------------------------------------------------------*/

#ifndef _WARNINGSOFF_H
#define _WARNINGSOFF_H

/* Nonstandard extension : nameless struct/union */
#pragma warning ( disable : 4201 )

/* Unreferenced in inline function removed */
#pragma warning ( disable : 4514 )

/* conditional expression is constant */
#pragma warning ( disable : 4127 )

/* default constructor could not be generated */
#pragma warning ( disable : 4510 )

#endif  /* _WARNINGSOFF_H */


