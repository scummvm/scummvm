/*----------------------------------------------------------------------
       John Robbins - Microsoft Systems Journal Bugslayer Column
------------------------------------------------------------------------

    This is a paper thin layer over the IMAGEHLP.DLL symbol engine which
helps hide some of the differences between the original NT4.0 version
and the November Platform SDK and later version.

    Note that this class only wraps those functions that take the
unique HANDLE value.  Other IMAGEHLP.DLL symbol engine functions are
global in scope so I did not wrap them with this class.

    While IMAGEHLP.DLL has a version API, ImagehlpApiVersion and
ImagehlpApiVersionEx, it is very weird.  No matter which of the many
builds of IMAGEHLP.DLL I checked, they all returned major version 4,
minor version 0, revision 5.  This was even the case with the new NT5
IMAGEHLP.DLL.  Whatever.  What I did instead was to just look for the
line specific exported functions with GetProcAddress.  If they are
there, then they can be used.  To see if the line information functions
are supported, call the classes CanDoSourceLines function.

    I was able to take care of almost all the differences between the
different versions here.  The one difference is that the
IMAGEHLP_DEFERRED_SYMBOL_LOAD structure is a larger size in the November
Platform SDK version.

------------------------------------------------------------------------
Compilation Defines:

FLEXIBLE_SYMBOLENGINE - Define this to alway have the CSymbolEngine
                        class use the GetProcAddress method of
                        determining if the IMAGEHLP.DLL in memory
                        supports the new source and line functions.

DO_NOT_WORK_AROUND_SRCLINE_BUG - Define this to NOT work around the
                                 SymGetLineFromAddr bug where PDB file
                                 lookups fail after the first lookup.
----------------------------------------------------------------------*/

#ifndef _SYMBOLENGINE_H
#define _SYMBOLENGINE_H

#define FLEXIBLE_SYMBOLENGINE

#include "imagehlp.h"
#include <tchar.h>

// Include these in case the user forgets to link against them.
#pragma comment (lib,"imagehlp.lib")
#pragma comment (lib,"version.lib")

// The API_VERSION_NUMBER define is 5 with the NT4 IMAGEHLP.H.  It is
//  7 with the November Platform SDK version.  This seems to be the only
//  reliable way to see which header is being used.
#if ( API_VERSION_NUMBER < 7 )
#define SYMENG_EXTRAWORK
#define SYMENG_NEEDDECLS
#else
#undef SYMENG_EXTRAWORK
#undef SYMENG_NEEDDECLS
#endif

// If FLEXIBLE_SYMBOLENGINE is defined, ALWAYS define SYMENG_EXTRAWORK
#ifdef FLEXIBLE_SYMBOLENGINE
#define SYMENG_EXTRAWORK
#endif // FLEXIBLE_SYMBOLENGINE

// I will define the structures that need defining in case the included
//  IMAGEHLP.H is from NT4.0.

#ifdef SYMENG_NEEDDECLS
// source file line data structure
typedef struct _IMAGEHLP_LINE
{
    DWORD SizeOfStruct;           // set to sizeof(IMAGEHLP_LINE)
    DWORD Key;                    // internal
    DWORD LineNumber;             // line number in file
    PCHAR FileName;               // full filename
    DWORD Address;                // first instruction of line
} IMAGEHLP_LINE, *PIMAGEHLP_LINE;
#endif  // SYMENG_NEEDDECLS

#ifndef SYMOPT_LOAD_LINES
#define SYMOPT_LOAD_LINES        0x00000010
#endif  // SYMOPT_LOAD_LINES

#ifndef SYMOPT_OMAP_FIND_NEAREST
#define SYMOPT_OMAP_FIND_NEAREST 0x00000020
#endif  // SYMOPT_OMAP_FIND_NEAREST

/*//////////////////////////////////////////////////////////////////////
I just lifted the prototypes right out of the NT5 header.

Note: I had a whopper of a bug here.  I had the declarations and
      typedefs like the following:

    BOOL
    IMAGEAPI
    SymGetLineFromAddr(
        IN  HANDLE                  hProcess,
        IN  DWORD                   dwAddr,
        OUT PDWORD                  pdwDisplacement,
        OUT PIMAGEHLP_LINE          Line
        );

    typedef
    BOOL (*PFNSYMGETLINEFROMADDR) ( IN  HANDLE         hProcess        ,
                                    IN  DWORD          dwAddr          ,
                                    OUT PDWORD         pdwDisplacement ,
                                    OUT PIMAGEHLP_LINE Line           );

    Whenever I ran the release build, it always crashed because the
    stack was not restoring the registers when certain functions in this
    class were called.  Looking at the disassembly, I kept seeing
    instructions like ADD ESP,10h after certain calls.  This was driving
    me nuts until it dawned on me that the extra ADD after the calls
    were cleaning up the stack for CDECL calls but that Windows API
    calls are all STDCALL.  I had a calling convention mismatch!

    I just thought I'd mention this so that it might save you some
    problems if you do something like this.  Watch those calling
    conventions!

//////////////////////////////////////////////////////////////////////*/

#ifdef SYMENG_NEEDDECLS
BOOL
__stdcall
SymGetLineFromAddr(
    IN  HANDLE                  hProcess,
    IN  DWORD                   dwAddr,
    OUT PDWORD                  pdwDisplacement,
    OUT PIMAGEHLP_LINE          Line
    );

BOOL
__stdcall
SymGetLineFromName(
    IN     HANDLE               hProcess,
    IN     LPSTR                ModuleName,
    IN     LPSTR                FileName,
    IN     DWORD                dwLineNumber,
       OUT PLONG                plDisplacement,
    IN OUT PIMAGEHLP_LINE       Line
    );

BOOL
__stdcall
SymGetLineNext(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE       Line
    );

BOOL
__stdcall
SymGetLinePrev(
    IN     HANDLE               hProcess,
    IN OUT PIMAGEHLP_LINE       Line
    );

BOOL
__stdcall
SymMatchFileName(
    IN  LPSTR  FileName,
    IN  LPSTR  Match,
    OUT LPSTR *FileNameStop,
    OUT LPSTR *MatchStop
    );
#endif  // SYMENG_NEEDDECLS

// The great Bugslayer idea of creating wrapper classes on structures
//  that have size fields came from fellow MSJ columnist, Paul DiLascia.
// Thanks, Paul!

// I did not wrap IMAGEHLP_SYMBOL because that is a variable size
//  structure.

// The IMAGEHLP_MODULE wrapper class.
struct CImageHlp_Module : public IMAGEHLP_MODULE
{
    CImageHlp_Module ( )
    {
        memset ( this , NULL , sizeof ( IMAGEHLP_MODULE ) ) ;
        SizeOfStruct = sizeof ( IMAGEHLP_MODULE ) ;
    }
} ;

// The IMAGEHLP_LINE wrapper class.
struct CImageHlp_Line : public IMAGEHLP_LINE
{
    CImageHlp_Line ( )
    {
        memset ( this , NULL , sizeof ( IMAGEHLP_LINE ) ) ;
        SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;
    }
} ;

// Typedefs for the new source and line functions.
typedef
BOOL (__stdcall *PFNSYMGETLINEFROMADDR)
                              ( IN  HANDLE         hProcess         ,
                                IN  DWORD          dwAddr           ,
                                OUT PDWORD         pdwDisplacement  ,
                                OUT PIMAGEHLP_LINE Line              ) ;
typedef
BOOL (__stdcall *PFNSYMGETLINEFROMNAME)
                              ( IN     HANDLE         hProcess      ,
                                IN     LPSTR          ModuleName    ,
                                IN     LPSTR          FileName      ,
                                IN     DWORD          dwLineNumber  ,
                                OUT    PLONG          plDisplacement,
                                IN OUT PIMAGEHLP_LINE Line           ) ;
typedef
BOOL (__stdcall *PFNSYMGETLINENEXT) ( IN     HANDLE         hProcess ,
                                      IN OUT PIMAGEHLP_LINE Line      );
typedef
BOOL (__stdcall *PFNSYMGETLINEPREV) ( IN     HANDLE         hProcess ,
                                      IN OUT PIMAGEHLP_LINE Line      );
typedef
BOOL (__stdcall *PFNSYMMATCHFILENAME) ( IN  LPSTR   FileName      ,
                                        IN  LPSTR   Match         ,
                                        OUT LPSTR * FileNameStop  ,
                                        OUT LPSTR * MatchStop      ) ;

// The symbol engine class.
class CSymbolEngine
{
/*----------------------------------------------------------------------
                  Public Construction and Destruction
----------------------------------------------------------------------*/
public      :
    // The constructor just does the work of finding the new functions.
    //  Use this class just line the normal C functions and call the
    //  SymInitialize function to get everything started.
    CSymbolEngine ( void )
    {
#ifdef SYMENG_EXTRAWORK
        HINSTANCE hInstImageHlp = GetModuleHandleA ( "IMAGEHLP.DLL" ) ;

        m_pfnSymGetLineFromAddr =
            (PFNSYMGETLINEFROMADDR)GetProcAddress(hInstImageHlp ,
                                                  "SymGetLineFromAddr");
        if ( NULL == m_pfnSymGetLineFromAddr )
        {
            m_pfnSymGetLineFromName = NULL ;
            m_pfnSymGetLineNext     = NULL ;
            m_pfnSymGetLinePrev     = NULL ;
            m_pfnSymMatchFileName   = NULL ;
        }
        else
        {
            m_pfnSymGetLineFromName =
               (PFNSYMGETLINEFROMNAME)GetProcAddress(hInstImageHlp ,
                                                  "SymGetLineFromName");
            m_pfnSymGetLineNext     =
               (PFNSYMGETLINENEXT)GetProcAddress ( hInstImageHlp  ,
                                                   "SymGetLineNext" ) ;
            m_pfnSymGetLinePrev     =
               (PFNSYMGETLINEPREV)GetProcAddress ( hInstImageHlp  ,
                                                   "SymGetLinePrev"  ) ;
            m_pfnSymMatchFileName   =
               (PFNSYMMATCHFILENAME)GetProcAddress (hInstImageHlp ,
                                                    "SymMatchFileName");
        }
#endif  // SYMENG_EXTRAWORK
    }

    virtual ~CSymbolEngine ( void )
    {
    }

/*----------------------------------------------------------------------
                  Public Helper Information Functions
----------------------------------------------------------------------*/
public      :

    BOOL CanDoSourceLines ( void )
    {
#ifdef SYMENG_EXTRAWORK
        return ( NULL != m_pfnSymGetLineFromAddr ) ;
#else
        return ( TRUE ) ;
#endif  // SYMENG_EXTRAWORK
    }

    // Returns the file version string of IMAGEHLP.DLL being used.
    // To convert the return values into something readable:
    // wsprintf ( szVer                  ,
    //            _T ( "%d.%02d.%d.%d" ) ,
    //            HIWORD ( dwMS )        ,
    //            LOWORD ( dwMS )        ,
    //            HIWORD ( dwLS )        ,
    //            LOWORD ( dwLS )         ) ;
    // szVer will contain a string like: 5.00.1878.1
    BOOL GetImageHlpVersion ( DWORD & dwMS , DWORD & dwLS )
    {
        return( GetInMemoryFileVersion ( "IMAGEHLP.DLL" , dwMS , dwLS));
    }

    // Does the same thing for the PDB reading DLLs.
    BOOL GetPDBReaderVersion ( DWORD & dwMS , DWORD & dwLS )
    {
        // First try MSDBI.DLL.
        if ( TRUE == GetInMemoryFileVersion ( "MSDBI.DLL" ,
                                              dwMS        ,
                                              dwLS          ) )
        {
            return ( TRUE ) ;
        }
        // MSDBI.DLL is not in memory so try MSPDB50.DLL.
        return ( GetInMemoryFileVersion ( "MSPDB50.DLL" , dwMS , dwLS));
    }

    // The worker function used by the previous two functions.
    BOOL GetInMemoryFileVersion ( LPCTSTR szFile ,
                                  DWORD & dwMS   ,
                                  DWORD & dwLS    )
    {
        HMODULE hInstIH = GetModuleHandle ( szFile ) ;

        // Get the full filename of the loaded version.
        TCHAR szImageHlp[ MAX_PATH ] ;
        GetModuleFileName ( hInstIH , szImageHlp , MAX_PATH ) ;

        dwMS = 0 ;
        dwLS = 0 ;

        // Get the version info size.
        DWORD dwVerInfoHandle ;
        DWORD dwVerSize       ;

        dwVerSize = GetFileVersionInfoSize ( szImageHlp       ,
                                             &dwVerInfoHandle  ) ;
        if ( 0 == dwVerSize )
        {
            return ( FALSE ) ;
        }

        // Got the version size, now get the version info.
        LPVOID lpData = (LPVOID)new TCHAR [ dwVerSize ] ;
        if ( FALSE == GetFileVersionInfo ( szImageHlp       ,
                                           dwVerInfoHandle  ,
                                           dwVerSize        ,
                                           lpData            ) )
        {
            delete [] lpData ;
            return ( FALSE ) ;
        }

        VS_FIXEDFILEINFO * lpVerInfo ;
        UINT uiLen ;
        BOOL bRet = VerQueryValue ( lpData              ,
                                    _T ( "\\" )         ,
                                    (LPVOID*)&lpVerInfo ,
                                    &uiLen               ) ;
        if ( TRUE == bRet )
        {
            dwMS = lpVerInfo->dwFileVersionMS ;
            dwLS = lpVerInfo->dwFileVersionLS ;
        }

        delete [] lpData ;

        return ( bRet ) ;
    }

/*----------------------------------------------------------------------
                   Public Initialization and Cleanup
----------------------------------------------------------------------*/
public      :

    BOOL SymInitialize ( IN HANDLE   hProcess       ,
                         IN LPSTR    UserSearchPath ,
                         IN BOOL     fInvadeProcess  )
    {
        m_hProcess = hProcess ;
        return ( ::SymInitialize ( hProcess       ,
                                   UserSearchPath ,
                                   fInvadeProcess  ) ) ;
    }

    BOOL SymCleanup ( void )
    {
        return ( ::SymCleanup ( m_hProcess ) ) ;
    }

/*----------------------------------------------------------------------
                       Public Module Manipulation
----------------------------------------------------------------------*/
public      :

    BOOL SymEnumerateModules ( IN PSYM_ENUMMODULES_CALLBACK
                                                    EnumModulesCallback,
                               IN PVOID             UserContext )
    {
        return ( ::SymEnumerateModules ( m_hProcess           ,
                                         EnumModulesCallback  ,
                                         UserContext           ) ) ;
    }

    BOOL SymLoadModule ( IN  HANDLE hFile       ,
                         IN  PSTR   ImageName   ,
                         IN  PSTR   ModuleName  ,
                         IN  DWORD  BaseOfDll   ,
                         IN  DWORD  SizeOfDll    )
    {
        return ( ::SymLoadModule ( m_hProcess   ,
                                   hFile        ,
                                   ImageName    ,
                                   ModuleName   ,
                                   BaseOfDll    ,
                                   SizeOfDll     ) ) ;
    }

    BOOL EnumerateLoadedModules ( IN PENUMLOADED_MODULES_CALLBACK
                                              EnumLoadedModulesCallback,
                                  IN PVOID         UserContext       )
    {
        return ( ::EnumerateLoadedModules ( m_hProcess                ,
                                            EnumLoadedModulesCallback ,
                                            UserContext              ));
    }

    BOOL SymUnloadModule ( IN  DWORD BaseOfDll )
    {
        return ( ::SymUnloadModule ( m_hProcess , BaseOfDll ) ) ;
    }

    BOOL SymGetModuleInfo ( IN  DWORD            dwAddr     ,
                            OUT PIMAGEHLP_MODULE ModuleInfo  )
    {
        return ( ::SymGetModuleInfo ( m_hProcess    ,
                                      dwAddr        ,
                                      ModuleInfo     ) ) ;
    }

    DWORD SymGetModuleBase ( IN DWORD dwAddr )
    {
        return ( ::SymGetModuleBase ( m_hProcess , dwAddr ) ) ;
    }

/*----------------------------------------------------------------------
                       Public Symbol Manipulation
----------------------------------------------------------------------*/
public      :

    BOOL SymEnumerateSymbols (IN DWORD                        BaseOfDll,
                              IN PSYM_ENUMSYMBOLS_CALLBACK
                                                    EnumSymbolsCallback,
                              IN PVOID                     UserContext )
    {
        return ( ::SymEnumerateSymbols ( m_hProcess          ,
                                         BaseOfDll           ,
                                         EnumSymbolsCallback ,
                                         UserContext          ) ) ;
    }

    BOOL SymGetSymFromAddr ( IN  DWORD               dwAddr          ,
                             OUT PDWORD              pdwDisplacement ,
                             OUT PIMAGEHLP_SYMBOL    Symbol           )
    {
        return ( ::SymGetSymFromAddr ( m_hProcess       ,
                                       dwAddr           ,
                                       pdwDisplacement  ,
                                       Symbol            ) ) ;
    }

    BOOL SymGetSymFromName ( IN  LPSTR            Name   ,
                             OUT PIMAGEHLP_SYMBOL Symbol  )
    {
        return ( ::SymGetSymFromName ( m_hProcess ,
                                       Name       ,
                                       Symbol      ) ) ;
    }

    BOOL SymGetSymNext ( IN OUT PIMAGEHLP_SYMBOL Symbol )
    {
        return ( ::SymGetSymNext ( m_hProcess , Symbol ) ) ;
    }

    BOOL SymGetSymPrev ( IN OUT PIMAGEHLP_SYMBOL Symbol )
    {
        return ( :: SymGetSymPrev ( m_hProcess , Symbol ) ) ;
    }

/*----------------------------------------------------------------------
                    Public Source Line Manipulation
----------------------------------------------------------------------*/
public      :

    BOOL SymGetLineFromAddr ( IN  DWORD          dwAddr          ,
                              OUT PDWORD         pdwDisplacement ,
                              OUT PIMAGEHLP_LINE Line             )
    {
        // Holds the function pointer.
        PFNSYMGETLINEFROMADDR pfnSGLFA ;

#ifdef SYMENG_EXTRAWORK
        if ( NULL == m_pfnSymGetLineFromAddr )
        {
            return ( FALSE ) ;
        }
        else
        {
            pfnSGLFA = m_pfnSymGetLineFromAddr ;
        }
#else
        // Just grab it directly.
        pfnSGLFA = ::SymGetLineFromAddr ;
#endif  // SYMENG_EXTRAWORK

#ifdef DO_NOT_WORK_AROUND_SRCLINE_BUG
        // Just return whatever the main function returned.
        return ( pfnSGLFA ( m_hProcess      ,
                            dwAddr          ,
                            pdwDisplacement ,
                            Line             ) ) ;

#else
        // The problem is that the symbol engine only finds those source
        //  line addresses (after the first lookup) that fall exactly on
        //  a zero displacement.  I will walk backwards 100 bytes to
        //  find the line and return the proper displacement.
        DWORD dwTempDis = 0 ;
        while ( FALSE == pfnSGLFA ( m_hProcess          ,
                                    dwAddr - dwTempDis  ,
                                    pdwDisplacement     ,
                                    Line                 ) )
        {
            dwTempDis += 1 ;
            if ( 100 == dwTempDis )
            {
                return ( FALSE ) ;
            }
        }
        // It was found and the source line information is correct so
        //  change the displacement if it was looked up multiple times.
        if ( 0 != dwTempDis )
        {
            *pdwDisplacement = dwTempDis ;
        }
        return ( TRUE ) ;
#endif // DO_NOT_WORK_AROUND_SRCLINE_BUG
    }

    BOOL SymGetLineFromName ( IN     LPSTR          ModuleName      ,
                              IN     LPSTR          FileName        ,
                              IN     DWORD          dwLineNumber    ,
                              OUT    PLONG          plDisplacement  ,
                              IN OUT PIMAGEHLP_LINE Line             )
    {
#ifdef SYMENG_EXTRAWORK
        if ( NULL == m_pfnSymGetLineFromName )
        {
            return ( FALSE ) ;
        }
        return ( m_pfnSymGetLineFromName ( m_hProcess       ,
                                           ModuleName       ,
                                           FileName         ,
                                           dwLineNumber     ,
                                           plDisplacement   ,
                                           Line              ) ) ;
#else
        return ( ::SymGetLineFromName ( m_hProcess       ,
                                        ModuleName       ,
                                        FileName         ,
                                        dwLineNumber     ,
                                        plDisplacement   ,
                                        Line              ) ) ;
#endif  // SYMENG_EXTRAWORK
    }

    BOOL SymGetLineNext ( IN OUT PIMAGEHLP_LINE Line )
    {
#ifdef SYMENG_EXTRAWORK
        if ( NULL == m_pfnSymGetLineNext )
        {
            return ( FALSE ) ;
        }
        return ( m_pfnSymGetLineNext ( m_hProcess , Line ) ) ;
#else
        return ( ::SymGetLineNext ( m_hProcess , Line ) ) ;
#endif  // SYMENG_EXTRAWORK
    }

    BOOL SymGetLinePrev ( IN OUT PIMAGEHLP_LINE Line )
    {
#ifdef SYMENG_EXTRAWORK
        if ( NULL == m_pfnSymGetLinePrev )
        {
            return ( FALSE ) ;
        }
        return ( m_pfnSymGetLinePrev ( m_hProcess , Line ) ) ;
#else
        return ( ::SymGetLinePrev ( m_hProcess , Line ) ) ;
#endif  // SYMENG_EXTRAWORK
    }

    // What is this?
    BOOL SymMatchFileName ( IN  LPSTR   FileName        ,
                            IN  LPSTR   Match           ,
                            OUT LPSTR * FileNameStop    ,
                            OUT LPSTR * MatchStop        )
    {
#ifdef SYMENG_EXTRAWORK
        if ( NULL == m_pfnSymMatchFileName )
        {
            return ( FALSE ) ;
        }
        return ( m_pfnSymMatchFileName ( FileName       ,
                                         Match          ,
                                         FileNameStop   ,
                                         MatchStop       ) ) ;
#else
        return ( ::SymMatchFileName ( FileName       ,
                                      Match          ,
                                      FileNameStop   ,
                                      MatchStop       ) ) ;
#endif  // SYMENG_EXTRAWORK
    }

/*----------------------------------------------------------------------
                          Public Misc Members
----------------------------------------------------------------------*/
public      :

    LPVOID SymFunctionTableAccess ( DWORD AddrBase )
    {
        return ( :: SymFunctionTableAccess ( m_hProcess , AddrBase ) ) ;
    }

    BOOL SymGetSearchPath ( OUT LPSTR SearchPath        ,
                            IN  DWORD SearchPathLength   )
    {
        return ( :: SymGetSearchPath ( m_hProcess       ,
                                       SearchPath       ,
                                       SearchPathLength  ) ) ;
    }

    BOOL SymSetSearchPath ( IN LPSTR SearchPath )
    {
        return ( :: SymSetSearchPath ( m_hProcess , SearchPath ) ) ;
    }

    BOOL SymRegisterCallback ( IN PSYMBOL_REGISTERED_CALLBACK
                                                       CallbackFunction,
                               IN PVOID                UserContext    )
    {
        return ( ::SymRegisterCallback ( m_hProcess         ,
                                         CallbackFunction   ,
                                         UserContext         ) ) ;
    }


/*----------------------------------------------------------------------
                         Protected Data Members
----------------------------------------------------------------------*/
protected   :
    // The unique value that will be used for this instance of the
    //  symbol engine.  Note that this does not have to be an actual
    //  process value, just a unique value.
    HANDLE      m_hProcess      ;

#ifdef SYMENG_EXTRAWORK
    // The function pointers to the new symbol engine source and line
    //  functions.
    PFNSYMGETLINEFROMADDR   m_pfnSymGetLineFromAddr ;
    PFNSYMGETLINEFROMNAME   m_pfnSymGetLineFromName ;
    PFNSYMGETLINENEXT       m_pfnSymGetLineNext     ;
    PFNSYMGETLINEPREV       m_pfnSymGetLinePrev     ;
    PFNSYMMATCHFILENAME     m_pfnSymMatchFileName   ;
#endif  // SYMENG_EXTRAWORK

} ;

#endif      // _SYMBOLENGINE_H


