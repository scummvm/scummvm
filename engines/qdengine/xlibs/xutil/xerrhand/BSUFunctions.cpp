/*----------------------------------------------------------------------
       John Robbins - Microsoft Systems Journal Bugslayer Column

The functions that handle wrapping differences between NT and 95/98.
----------------------------------------------------------------------*/

#include "xglobal.h"
#include "PCH.h"
#include "BugslayerUtil.h"
#include "Internal.h"

// The Win95 version of GetModuleBaseName.
static DWORD __stdcall Win95GetModuleBaseName ( HANDLE  hProcess   ,
                                                HMODULE hModule    ,
                                                LPSTR   lpBaseName ,
                                                DWORD   nSize       ) ;

/*//////////////////////////////////////////////////////////////////////
                        Function Implementation
//////////////////////////////////////////////////////////////////////*/

DWORD __stdcall BSUGetModuleBaseName ( HANDLE  hProcess   ,
                                       HMODULE hModule    ,
                                       LPTSTR  lpBaseName ,
                                       DWORD   nSize       )
{
    if ( TRUE == IsNT ( ) )
    {
        // Call the NT version.  It is in NT4ProcessInfo because that is
        //  where all the PSAPI wrappers are kept.
        return ( NTGetModuleBaseName ( hProcess     ,
                                       hModule      ,
                                       lpBaseName   ,
                                       nSize         ) ) ;
    }
    return ( Win95GetModuleBaseName ( hProcess     ,
                                      hModule      ,
                                      lpBaseName   ,
                                      nSize         ) ) ;

}

static DWORD __stdcall
            Win95GetModuleBaseName ( HANDLE  /*hProcess*/  ,
                                     HMODULE hModule       ,
                                     LPSTR   lpBaseName    ,
                                     DWORD   nSize          )
{
    ASSERT ( FALSE == IsBadWritePtr ( lpBaseName , nSize ) ) ;
    if ( TRUE == IsBadWritePtr ( lpBaseName , nSize ) )
    {
        TRACE0 ( "Win95GetModuleBaseName Invalid string buffer\n" ) ;
        SetLastError ( ERROR_INVALID_PARAMETER ) ;
        return ( 0 ) ;
    }

    // This could blow the stack...
    char szBuff[ MAX_PATH + 1 ] ;
    DWORD dwRet = GetModuleFileName ( hModule , szBuff , MAX_PATH ) ;
    ASSERT ( 0 != dwRet ) ;
    if ( 0 == dwRet )
    {
        return ( 0 ) ;
    }

    // Find the last '\' mark.
    char * pStart = strrchr ( szBuff , '\\' ) ;
    int iMin ;
    if ( NULL != pStart )
    {
        // Move up one character.
        pStart++ ;
        //lint -e666
        iMin = min ( (int)nSize , (lstrlen ( pStart ) + 1) ) ;
        //lint +e666
        lstrcpyn ( lpBaseName , pStart , iMin ) ;
    }
    else
    {
        // Copy the szBuff buffer in.
        //lint -e666
        iMin = min ( (int)nSize , (lstrlen ( szBuff ) + 1) ) ;
        //lint +e666
        lstrcpyn ( lpBaseName , szBuff , iMin ) ;
    }
    // Always NULL terminate.
    lpBaseName[ iMin ] = '\0' ;
    return ( (DWORD)(iMin - 1) ) ;
}


