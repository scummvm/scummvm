/*----------------------------------------------------------------------
       John Robbins - Microsoft Systems Journal Bugslayer Column

The internal header for the BugslayerUtil.DLL code.
----------------------------------------------------------------------*/

#ifndef _INTERNAL_H
#define _INTERNAL_H

// The NT4 specific version of GetLoadedModules.
BOOL NT4GetLoadedModules ( DWORD     dwPID        ,
                           UINT      uiCount      ,
                           HMODULE * paModArray   ,
                           LPUINT    puiRealCount   ) ;

// The TOOLHELP32 specific version of GetLoadedModules.
BOOL TLHELPGetLoadedModules ( DWORD     dwPID        ,
                              UINT      uiCount      ,
                              HMODULE * paModArray   ,
                              LPUINT    puiRealCount   ) ;

// The NT version of GetModuleBaseName.
DWORD __stdcall NTGetModuleBaseName ( HANDLE  hProcess   ,
                                      HMODULE hModule    ,
                                      LPTSTR  lpBaseName ,
                                      DWORD   nSize       ) ;

#endif  // _INTERNAL_H


