/*----------------------------------------------------------------------
       John Robbins - Microsoft Systems Journal Bugslayer Column

The TOOLHELP32.DLL access functions.  These are used by Windows95,
Windows98, and NT5.  NT4.0 is covered in NT4ProcessInfo.cpp.
----------------------------------------------------------------------*/

#include "xglobal.h"
#include "PCH.h"
#include "BugslayerUtil.h"
#include "TLHELP32.h"

/*//////////////////////////////////////////////////////////////////////
                                Typedefs
//////////////////////////////////////////////////////////////////////*/
// The typedefs for the TOOLHELP32.DLL functions used by this module.
// Type definitions for pointers to call tool help functions.
typedef BOOL (WINAPI *MODULEWALK) ( HANDLE          hSnapshot ,
                                    LPMODULEENTRY32 lpme       ) ;
typedef BOOL (WINAPI *THREADWALK) ( HANDLE          hSnapshot ,
                                    LPTHREADENTRY32 lpte       ) ;
typedef BOOL (WINAPI *PROCESSWALK) ( HANDLE           hSnapshot ,
                                     LPPROCESSENTRY32 lppe       ) ;
typedef HANDLE (WINAPI *CREATESNAPSHOT) ( DWORD dwFlags       ,
                                          DWORD th32ProcessID  ) ;


/*//////////////////////////////////////////////////////////////////////
                            File Static Data
//////////////////////////////////////////////////////////////////////*/
// Has the function stuff here been initialized?  This is only to be
//  used by the InitPSAPI function and nothing else.
static BOOL           g_bInitialized              = FALSE ;
static CREATESNAPSHOT g_pCreateToolhelp32Snapshot = NULL  ;
static MODULEWALK     g_pModule32First            = NULL  ;
static MODULEWALK     g_pModule32Next             = NULL  ;
static PROCESSWALK    g_pProcess32First           = NULL  ;
static PROCESSWALK    g_pProcess32Next            = NULL  ;
static THREADWALK     g_pThread32First            = NULL  ;
static THREADWALK     g_pThread32Next             = NULL  ;

/*----------------------------------------------------------------------
FUNCTION        :   InitTOOLHELP32
DISCUSSION      :
    Retrieves the function pointers needed to access the tool help
routines.  Since this code is supposed to work on NT4, it cannot link
to the non-existant addresses in KERNEL32.DLL.
    This is pretty much lifter right from the MSDN.
PARAMETERS      :
    None.
RETURNS         :
    TRUE  - Everything initialized properly.
    FALSE - There was a problem.
----------------------------------------------------------------------*/
static BOOL InitTOOLHELP32 ( void )
{
    if ( TRUE == g_bInitialized )
    {
        return ( TRUE ) ;
    }

    BOOL      bRet    = FALSE ;
    HINSTANCE hKernel = NULL  ;

    // Obtain the module handle of the kernel to retrieve addresses of
    //  the tool helper functions.
    hKernel = GetModuleHandleA ( "KERNEL32.DLL" ) ;
    ASSERT ( NULL != hKernel ) ;

    if ( NULL != hKernel )
    {
        g_pCreateToolhelp32Snapshot =
           (CREATESNAPSHOT)GetProcAddress ( hKernel ,
                                            "CreateToolhelp32Snapshot");
        ASSERT ( NULL != g_pCreateToolhelp32Snapshot ) ;

        g_pModule32First = (MODULEWALK)GetProcAddress (hKernel ,
                                                       "Module32First");
        ASSERT ( NULL != g_pModule32First ) ;

        g_pModule32Next = (MODULEWALK)GetProcAddress (hKernel        ,
                                                      "Module32Next"  );
        ASSERT ( NULL != g_pModule32Next ) ;

        g_pProcess32First =
                (PROCESSWALK)GetProcAddress ( hKernel          ,
                                              "Process32First"  ) ;
        ASSERT ( NULL != g_pProcess32First ) ;

        g_pProcess32Next =
                (PROCESSWALK)GetProcAddress ( hKernel         ,
                                              "Process32Next" ) ;
        ASSERT ( NULL != g_pProcess32Next ) ;

        g_pThread32First =
                (THREADWALK)GetProcAddress ( hKernel         ,
                                             "Thread32First"  ) ;
        ASSERT ( NULL != g_pThread32First ) ;

        g_pThread32Next =
                (THREADWALK)GetProcAddress ( hKernel        ,
                                             "Thread32Next"  ) ;
        ASSERT ( NULL != g_pThread32Next ) ;

        // All addresses must be non-NULL to be successful.  If one of
        //  these addresses is NULL, one of the needed lists cannot be
        //  walked.

        bRet =  g_pModule32First            &&
                g_pModule32Next             &&
                g_pProcess32First           &&
                g_pProcess32Next            &&
                g_pThread32First            &&
                g_pThread32Next             &&
                g_pCreateToolhelp32Snapshot    ;
    }
    else
    {
        // Could not get the module handle of kernel.
        SetLastErrorEx ( ERROR_DLL_INIT_FAILED , SLE_ERROR ) ;
        bRet = FALSE ;
    }

    ASSERT ( TRUE == bRet ) ;

    if ( TRUE == bRet )
    {
        // All OK, Jumpmaster!
        g_bInitialized = TRUE ;
    }
    return ( bRet ) ;
}

/*----------------------------------------------------------------------
FUNCTION        :   TLHELPGetLoadedModules
DISCUSSION      :
    The TOOLHELP32 specific version of GetLoadedModules.  This function
assumes that GetLoadedModules does the work to validate the parameters.
PARAMETERS      :
    dwPID        - The process ID to look into.
    uiCount      - The number of slots in the paModArray buffer.  If
                   this value is 0, then the return value will be TRUE
                   and puiRealCount will hold the number of items
                   needed.
    paModArray   - The array to place the HMODULES into.  If this buffer
                   is too small to hold the result and uiCount is not
                   zero, then FALSE is returned, but puiRealCount will
                   be the real number of items needed.
    puiRealCount - The count of items needed in paModArray, if uiCount
                   is zero, or the real number of items in paModArray.
RETURNS         :
    FALSE - There was a problem, check GetLastError.
    TRUE  - The function succeeded.  See the parameter discussion for
            the output parameters.
----------------------------------------------------------------------*/
BOOL TLHELPGetLoadedModules ( DWORD     dwPID        ,
                              UINT      uiCount      ,
                              HMODULE * paModArray   ,
                              LPUINT    puiRealCount   )
{

    // Always set puiRealCount to a know value before anything else.
    *puiRealCount = 0 ;

    if ( FALSE == InitTOOLHELP32 ( ) )
    {
        ASSERT ( FALSE ) ;
        SetLastErrorEx ( ERROR_DLL_INIT_FAILED , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // The snapshot handle.
    HANDLE        hModSnap     = NULL ;
    // The module structure.
    MODULEENTRY32 stME32              ;
    // The return value for the function.
    BOOL          bRet         = TRUE ;
    // A flag kept to report if the buffer was too small.
    BOOL          bBuffToSmall = FALSE ;


    // Get the snapshot for the specified process.
    hModSnap = g_pCreateToolhelp32Snapshot ( TH32CS_SNAPMODULE ,
                                             dwPID              ) ;
    ASSERT ( INVALID_HANDLE_VALUE != hModSnap ) ;
    if ( INVALID_HANDLE_VALUE == hModSnap )
    {
        TRACE1 ( "Unable to get module snapshot for %08X\n" , dwPID ) ;
        return ( FALSE ) ;
    }

    memset ( &stME32 , NULL , sizeof ( MODULEENTRY32 ) ) ;
    stME32.dwSize = sizeof ( MODULEENTRY32 ) ;

    // Start getting the module values.
    if ( TRUE == g_pModule32First ( hModSnap , &stME32 ) )
    {
        do
        {
            // If uiCount is not zero, copy values.
            if ( 0 != uiCount )
            {
                // If the passed in buffer is to small, set the flag.
                //  This is so we match the functionality of the NT4
                //  version of this function which will return the
                //  correct total needed.
                if ( ( TRUE == bBuffToSmall     ) ||
                     ( *puiRealCount == uiCount )   )
                {
                    bBuffToSmall = TRUE ;
                    break ;
                }
                else
                {
                    // Copy this value in.
                    paModArray[ *puiRealCount ] =
                                         (HINSTANCE)stME32.modBaseAddr ;
                }
            }
            // Bump up the real total count.
            *puiRealCount += 1 ;
        }
        while ( ( TRUE == g_pModule32Next ( hModSnap , &stME32 ) ) ) ;
    }
    else
    {
        ASSERT ( FALSE ) ;
        TRACE0 ( "Failed to get first module!\n" ) ;
        bRet = FALSE ;
    }

    // Close the snapshot handle.
    VERIFY ( CloseHandle ( hModSnap ) ) ;

    // Check if the buffer was too small.
    if ( TRUE == bBuffToSmall )
    {
        ASSERT ( FALSE ) ;
        TRACE0 ( "Buffer to small in TLHELPGetLoadedModules\n" ) ;
        SetLastErrorEx ( ERROR_INSUFFICIENT_BUFFER , SLE_ERROR ) ;
        bRet = FALSE ;
    }

    // All OK, Jumpmaster!
    SetLastError ( ERROR_SUCCESS ) ;
    return ( bRet ) ;
}

