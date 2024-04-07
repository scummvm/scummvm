/*----------------------------------------------------------------------
   John Robbins - Microsoft Systems Journal Bugslayer Column - Feb 99
----------------------------------------------------------------------*/

#include "xglobal.h"
#include "pch.h"
#include "BugslayerUtil.h"
#include "DiagAssert.h"

// The project internal header file.
#include "Internal.h"

#pragma warning ( disable : 4710 )

// More STL weirdo errors.

#pragma warning(disable: 4073)
#pragma init_seg(lib)
const int size = 10000;
static XBuffer buf4AssertsBuffer(size);
XBuffer& assertsBuffer()
{
	if(buf4AssertsBuffer.tell() > size*0.8){
		buf4AssertsBuffer.init();
		buf4AssertsBuffer < "assertBuffer overflow\r\n";
	}
	return buf4AssertsBuffer;
}

/*//////////////////////////////////////////////////////////////////////
                          File Scope Typedefs
//////////////////////////////////////////////////////////////////////*/
// The size of buffer that DiagAssert will use.  If you want to see more
//  stack trace, make this a bigger number.
#define DIAGASSERT_BUFFSIZE 4096

/*//////////////////////////////////////////////////////////////////////
                           File Scope Globals
//////////////////////////////////////////////////////////////////////*/
// The HMODULE vector.
static const int g_HMODVectorSizeMax = 200;
static int g_HMODVectorSize = 0;
static HINSTANCE g_HMODVector[g_HMODVectorSizeMax];

// The DiagAssert display options.
static DWORD g_DiagAssertOptions = DA_SHOWMSGBOX | DA_SHOWODS ;

// The symbol engine.
static CSymbolEngine g_cSym ;
// If TRUE, the symbol engine has been initialized.
static BOOL g_bSymIsInit = FALSE ;

/*//////////////////////////////////////////////////////////////////////
                         File Scope Prototypes
//////////////////////////////////////////////////////////////////////*/
// Handles doing the stack trace for DiagAssert.
static void DoStackTrace ( LPTSTR szString  ,
                           DWORD  dwSize    ,
                           DWORD  dwNumSkip  ) ;

/*//////////////////////////////////////////////////////////////////////
                            CODE STARTS HERE
//////////////////////////////////////////////////////////////////////*/

DWORD BUGSUTIL_DLLINTERFACE __stdcall
    SetDiagAssertOptions ( DWORD dwOpts )
{
    if ( DA_USEDEFAULTS == dwOpts )
    {
        return ( DA_USEDEFAULTS ) ;
    }
    DWORD dwOld = g_DiagAssertOptions ;
    g_DiagAssertOptions = dwOpts ;
    return ( dwOld ) ;
}

BOOL BUGSUTIL_DLLINTERFACE __stdcall AddDiagAssertModule(HMODULE hMod)
{
	g_HMODVector[g_HMODVectorSize] = hMod;
	if(++g_HMODVectorSize == g_HMODVectorSizeMax)
		g_HMODVectorSize = g_HMODVectorSizeMax - 1;
	return ( TRUE ) ;
}

static void (*assert_restore_graphics_function)() = 0;

void SetAssertRestoreGraphicsFunction(void(*func)())
{
	assert_restore_graphics_function = func;
}

int DiagAssert(unsigned long dwOverrideOpts, const char* szMsg, const char* szFile, unsigned long dwLine )
{
	static bool flag_recursion=check_command_line("noassert");
	if(flag_recursion)
		return 0;
	flag_recursion=1;
	ErrH.flag_errorOrAssertHandling=true;
    // The buffer used for the final message text.
    static TCHAR  szBuff [ DIAGASSERT_BUFFSIZE ] ;
    // The current position in szBuff ;
    LPTSTR pCurrPos = szBuff ;
    // The module name.
    TCHAR  szModName[ MAX_PATH + 1 ] ;
    // The decoded message from FormatMessage
    LPTSTR szFmtMsg = NULL ;
    // The options.
    DWORD  dwOpts = dwOverrideOpts ;
    // The last error value.  (Which is preserved across the call).
    DWORD  dwLastErr = GetLastError ( ) ;

    if(assert_restore_graphics_function)
		assert_restore_graphics_function();

    if(DA_USEDEFAULTS == dwOverrideOpts)
        dwOpts = g_DiagAssertOptions ;

    // Look in any specified modules for the code.
	for(HINSTANCE* loop = g_HMODVector; loop != g_HMODVector + g_HMODVectorSize; loop++){
        if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE      ,
          *loop, dwLastErr, 0, (LPTSTR)&szFmtMsg, 0, NULL))
            break;
    }

    // If the message was not translated, just look in the system.
    if(NULL == szFmtMsg){
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER    |
                          FORMAT_MESSAGE_IGNORE_INSERTS   |
                          FORMAT_MESSAGE_FROM_SYSTEM        ,
                        NULL                                ,
                        dwLastErr                           ,
                        0                                   ,
                        (LPTSTR)&szFmtMsg                   ,
                        0                                   ,
                        NULL                                 ) ;
    }

    // Make sure the message got translated into something.
    LPTSTR szRealLastErr ;
    if( NULL != szFmtMsg )
        szRealLastErr = szFmtMsg ;
    else
        szRealLastErr = _T ( "**Last error code does not exist!!!!" ) ;

    // Get the module name.
    if ( 0 == GetModuleFileName ( NULL , szModName , MAX_PATH ) )
        _tcscpy ( szModName , _T ( "<unknown application>" ) ) ;

    // Build the message.
    pCurrPos += (wsprintf ( szBuff                                 ,
                            _T ( "Debug Assertion Failed!\n\n"  \
                                 "Program : %s\n"               \
                                 "File : %s\n"                  \
                                 "Line : %d\n"                  \
                                 "Expression : %s\n\n"            ),
                                 //"Last Error (0x%08X) : %s\n\n"   ) ,
                            szModName                               ,
                            szFile                                  ,
                            dwLine                                  ,
                            szMsg                                   ,
                            dwLastErr                               )
                            //szRealLastErr                            )
                 * sizeof ( TCHAR ) ) ;

    // Get rid of the allocated memory from FormatMessage.
    if ( NULL != szFmtMsg )
    {
        LocalFree ( (LPVOID)szFmtMsg ) ;
    }

    // Am I supposed to show the stack trace too?
    if ( DA_SHOWSTACKTRACE == ( DA_SHOWSTACKTRACE & dwOpts ) )
    {
        DoStackTrace ( pCurrPos ,
                       sizeof ( szBuff ) -
                                       ((DWORD)pCurrPos-(DWORD)szBuff),
                       2         ) ;
    }

    // Is this supposed to go to ODS?
    if ( DA_SHOWODS == ( DA_SHOWODS & dwOpts ) )
    {
        OutputDebugString ( szBuff ) ;
    }

	assertsBuffer() < szBuff < "\r\n";

    // By default, treat the return as an IGNORE.  This works best in
    //  the case the user does not want the MessageBox.
    int iRet = IDIGNORE ;
    if ( DA_SHOWMSGBOX == ( DA_SHOWMSGBOX & dwOpts ) ) {
        HWND hWndParent = GetActiveWindow ( ) ;
        if ( NULL != hWndParent ) {
            hWndParent = GetLastActivePopup ( hWndParent ) ;
        }
        iRet = MessageBox ( hWndParent                      ,
                            szBuff                          ,
                            _T ( "ASSERTION FAILURE..." )   ,
                            MB_TASKMODAL            |
                                MB_SETFOREGROUND    |
                                MB_ABORTRETRYIGNORE |
                                MB_ICONERROR                 ) ;
    }

    // Put the incoming last error back.
    SetLastError ( dwLastErr ) ;

	flag_recursion=0;
	ErrH.flag_errorOrAssertHandling=false;

    // Figure out what to do on the return.
    if ( IDIGNORE == iRet ) {
        //HWND hWndParent = GetActiveWindow ( ) ;
        //if ( NULL != hWndParent ) {
        //    hWndParent = GetLastActivePopup ( hWndParent ) ;
        //}
        //return IDYES == MessageBox ( hWndParent    ,
        //                    "Permanently ignore this assertion?" ,
        //                    _T ( "ASSERTION FAILURE..." )   ,
        //                    MB_TASKMODAL            |
        //                        MB_SETFOREGROUND    |
        //                        MB_YESNO |
        //                        MB_ICONERROR ) ? 1 : 0;
        return 1;//Ignore - ignore all
    }
    else if ( IDRETRY == iRet ) {
        // This will trigger DebugBreak!!
		//SetUnhandledExceptionFilter(NULL);
        //return 2;
		return 0;//Retry - ignore 1
    }
	else 
		return 2; // int3 //Abort

    //// The return has to be Abort....
    ////ExitProcess ( (UINT)-1 ) ;
}

void BUGSUTIL_DLLINTERFACE __stdcall
    DiagOutput ( LPCTSTR szFmt , ... )
{
    // Never corrupt the last error value.
    DWORD dwLastError = GetLastError ( ) ;

    static TCHAR szOutBuff [ 1024 ] ;

    va_list  args ;

    va_start ( args , szFmt ) ;
    _vstprintf ( szOutBuff , szFmt , args ) ;
    OutputDebugString ( szOutBuff ) ;
    va_end ( args ) ;

    SetLastError ( dwLastError ) ;
}

static DWORD __stdcall GetModBase ( HANDLE hProcess , DWORD dwAddr )
{
    // Check in the symbol engine first.
    IMAGEHLP_MODULE stIHM ;

    // This is what the MFC stack trace routines forgot to do so their
    //  code will not get the info out of the symbol engine.
    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE ) ;

    if ( g_cSym.SymGetModuleInfo ( dwAddr , &stIHM ) )
    {
        return ( stIHM.BaseOfImage ) ;
    }
    else
    {
        // Let's go fishing.
        MEMORY_BASIC_INFORMATION stMBI ;

        if ( 0 != VirtualQueryEx ( hProcess         ,
                                   (LPCVOID)dwAddr  ,
                                   &stMBI           ,
                                   sizeof ( stMBI )  ) )
        {
            // Try and load it.
            DWORD dwNameLen = 0 ;
            TCHAR szFile[ MAX_PATH ] ;

            dwNameLen = GetModuleFileName ( (HINSTANCE)
                                                stMBI.AllocationBase ,
                                            szFile                   ,
                                            MAX_PATH                  );

            HANDLE hFile = NULL ;

            if ( 0 != dwNameLen )
            {
                hFile = CreateFile ( szFile       ,
                                     GENERIC_READ    ,
                                     FILE_SHARE_READ ,
                                     NULL            ,
                                     OPEN_EXISTING   ,
                                     0               ,
                                     0                ) ;
            }
#ifdef _DEBUG
            DWORD dwRet =
#endif
            g_cSym.SymLoadModule ( hFile                            ,
                                   ( dwNameLen ? szFile : NULL )    ,
                                   NULL                             ,
                                   (DWORD)stMBI.AllocationBase      ,
                                   0                                 ) ;
#ifdef _DEBUG
            if ( 0 == dwRet )
            {
                TRACE ( "SymLoadModule failed : 0x%08X\n" ,
                        GetLastError ( )                   ) ;
            }
#endif  // _DEBUG
            return ( (DWORD)stMBI.AllocationBase ) ;
        }
    }
    return ( 0 ) ;
}

static DWORD ConvertAddress ( DWORD dwAddr , LPTSTR szOutBuff )
{
    char szTemp [ MAX_PATH + sizeof ( IMAGEHLP_SYMBOL ) ] ;

    PIMAGEHLP_SYMBOL pIHS = (PIMAGEHLP_SYMBOL)&szTemp ;

    IMAGEHLP_MODULE stIHM ;

    LPTSTR pCurrPos = szOutBuff ;

    ZeroMemory ( pIHS , MAX_PATH + sizeof ( IMAGEHLP_SYMBOL ) ) ;
    ZeroMemory ( &stIHM , sizeof ( IMAGEHLP_MODULE ) ) ;

    pIHS->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL ) ;
    pIHS->Address = dwAddr ;
    pIHS->MaxNameLength = MAX_PATH ;

    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE ) ;

    // Always stick the address in first.
    pCurrPos += wsprintf ( pCurrPos , _T ( "0x%08X " ) , dwAddr ) ;

    // Get the module name.
    if ( 0 != g_cSym.SymGetModuleInfo ( dwAddr , &stIHM ) )
    {
        // Strip off the path.
        LPTSTR szName = _tcsrchr ( stIHM.ImageName , _T ( '\\' ) ) ;
        if ( NULL != szName )
        {
            szName++ ;
        }
        else
        {
            szName = stIHM.ImageName ;
        }
        pCurrPos += wsprintf ( pCurrPos , _T ( "%s: " ) , szName ) ;
    }
    else
    {
        pCurrPos += wsprintf ( pCurrPos , _T ( "<unknown module>: " ) );
    }

    // Get the function.
    DWORD dwDisp ;
    if ( 0 != g_cSym.SymGetSymFromAddr ( dwAddr , &dwDisp , pIHS ) )
    {
        if ( 0 == dwDisp )
        {
            pCurrPos += wsprintf ( pCurrPos , _T ( "%s" ) , pIHS->Name);
        }
        else
        {
            pCurrPos += wsprintf ( pCurrPos               ,
                                   _T ( "%s + %d bytes" ) ,
                                   pIHS->Name             ,
                                   dwDisp                  ) ;
        }

        // If I got a symbol, give the source and line a whirl.
        IMAGEHLP_LINE stIHL ;

        ZeroMemory ( &stIHL , sizeof ( IMAGEHLP_LINE ) ) ;

        stIHL.SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;

        if ( 0 != g_cSym.SymGetLineFromAddr ( dwAddr  ,
                                              &dwDisp ,
                                              &stIHL   ) )
        {
            // Put this on the next line and indented a bit.
            pCurrPos += wsprintf ( pCurrPos                  ,
                                  _T ( "\n\t\t%s, Line %d" ) ,
                                  stIHL.FileName             ,
                                  stIHL.LineNumber            ) ;
            if ( 0 != dwDisp )
            {
                pCurrPos += wsprintf ( pCurrPos             ,
                                       _T ( " + %d bytes" ) ,
                                       dwDisp                ) ;
            }
        }
    }
    else
    {
        pCurrPos += wsprintf ( pCurrPos , _T ( "<unknown symbol>" ) ) ;
    }

    // Tack on a CRLF.
    pCurrPos += wsprintf ( pCurrPos , _T ( "\n" ) ) ;

    return ( pCurrPos - szOutBuff ) ;
}

static void DoStackTrace ( LPTSTR szString  ,
                           DWORD  dwSize    ,
                           DWORD  dwNumSkip  )
{
    HANDLE hProcess = GetCurrentProcess ( ) ;

    // If the symbol engine is not initialized, do it now.
    if ( FALSE == g_bSymIsInit )
    {
        DWORD dwOpts = SymGetOptions ( ) ;

        // Turn on load lines.
        SymSetOptions ( dwOpts                |
                        SYMOPT_LOAD_LINES      ) ;

        if ( FALSE == g_cSym.SymInitialize ( hProcess ,
                                             NULL     ,
                                             FALSE     ) )
        {
            TRACE ( "DiagAssert : Unable to initialize the "
                    "symbol engine!!!\n" ) ;
#ifdef _DEBUG
            DebugBreak ( ) ;
#endif
        }
        else
        {
            g_bSymIsInit = TRUE ;
        }
    }

    // The symbol engine is initialized so do the stack walk.

    // The array of addresses.
	
	const int vAddrsSizeMax = 512;
	int vAddrsSize = 0;
    unsigned long vAddrs[vAddrsSizeMax];

    // The thread information.
    CONTEXT    stCtx  ;

    stCtx.ContextFlags = CONTEXT_FULL ;

    if ( GetThreadContext ( GetCurrentThread ( ) , &stCtx ) )
    {
        STACKFRAME stFrame ;
        DWORD      dwMachine ;

        ZeroMemory ( &stFrame , sizeof ( STACKFRAME ) ) ;

        stFrame.AddrPC.Mode = AddrModeFlat ;

#if defined (_M_IX86)
        dwMachine                = IMAGE_FILE_MACHINE_I386 ;
        stFrame.AddrPC.Offset    = stCtx.Eip    ;
        stFrame.AddrStack.Offset = stCtx.Esp    ;
        stFrame.AddrStack.Mode   = AddrModeFlat ;
        stFrame.AddrFrame.Offset = stCtx.Ebp    ;
        stFrame.AddrFrame.Mode   = AddrModeFlat ;

#elif defined (_M_ALPHA)
        dwMachine                = IMAGE_FILE_MACHINE_ALPHA ;
        stFrame.AddrPC.Offset    = (unsigned long)stCtx.Fir ;
#else
#error ( "Unknown machine!" )
#endif

        // Loop for the first 512 stack elements.
        for ( DWORD i = 0 ; i < 512; i++ ){
            if ( FALSE == StackWalk ( dwMachine              ,
                                      hProcess               ,
                                      hProcess               ,
                                      &stFrame               ,
                                      &stCtx                 ,
                                      NULL                   ,
                                      SymFunctionTableAccess ,
                                      GetModBase             ,
                                      NULL                    ) ){
                break ;
            }
            if ( i > dwNumSkip ){
                // Also check that the address is not zero.  Sometimes
                //  StackWalk returns TRUE with a frame of zero.
                if(stFrame.AddrPC.Offset)
                    vAddrs[vAddrsSize++] = stFrame.AddrPC.Offset;
            }
        }

        // Now start converting the addresses.
        DWORD dwSizeLeft = dwSize ;
        DWORD dwSymSize ;

        TCHAR szSym [ MAX_PATH * 2 ] ;
        LPTSTR szCurrPos = szString ;

        for(unsigned long* loop = vAddrs; loop != vAddrs + vAddrsSize; loop++){
            dwSymSize = ConvertAddress ( *loop , szSym ) ;
            if ( dwSizeLeft < dwSymSize )
                break ;
            _tcscpy ( szCurrPos , szSym ) ;
            szCurrPos += dwSymSize ;
            dwSizeLeft -= dwSymSize ;
        }
    }
}
