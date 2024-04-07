/*----------------------------------------------------------------------
  John Robbins - Microsoft Systems Journal Bugslayer Column - Feb '99
----------------------------------------------------------------------*/

#ifndef _DIAGASSERT_H
#define _DIAGASSERT_H

#ifndef __cplusplus
extern "C" {
#endif  //__cplusplus

/*//////////////////////////////////////////////////////////////////////
                                Defines
//////////////////////////////////////////////////////////////////////*/
// Keep the core stuff availible in both release and debug builds.
// Uses the global assert flags.
#define DA_USEDEFAULTS      0x0000
// Turns on showing the assert in a messagebox.  This is the default.
#define DA_SHOWMSGBOX       0x0001
// Turns on showing the assert as through OutputDebugString.  This is
//  the default.
#define DA_SHOWODS          0x0002
// Shows a stack trace in the assert.  This is off by default with the
//  ASSERT macro, and on in the SUPERASSERT macro.
#define DA_SHOWSTACKTRACE   0x0004

/*----------------------------------------------------------------------
FUNCTION        :   SetDiagAssertOptions
DISCUSSION      :
    Sets the global options for normal ASSERT macros.
PARAMETERS      :
    dwOpts - The new options flags.
RETURNS         :
    The previous options.
----------------------------------------------------------------------*/
DWORD BUGSUTIL_DLLINTERFACE __stdcall
    SetDiagAssertOptions ( DWORD dwOpts ) ;

/*----------------------------------------------------------------------
FUNCTION        :   AddDiagAssertModule
DISCUSSION      :
    Adds the specified module to the list of modules that error strings
will be pulled from.
PARAMETERS      :
    hMod - The module to add.
RETURNS         :
    TRUE  - The module was added.
    FALSE - The internal table is full.
----------------------------------------------------------------------*/
BOOL BUGSUTIL_DLLINTERFACE __stdcall
    AddDiagAssertModule ( HMODULE hMod ) ;

/*----------------------------------------------------------------------
FUNCTION        :   DiagAssert
DISCUSSION      :
    The actual assert function itself.
PARAMETERS      :
    dwOverrideOpts - The DA_* options to override the global defaults
                     for this call into DiagAssert.
    szMsg          - The message to show.
    szFile         - The file that showed the assert.
    dwLine         - The line that had the assert.
RETURNS         :
    FALSE - Ignore the assert.
    TRUE  - Trigger the DebugBreak.
----------------------------------------------------------------------*/
int DiagAssert (unsigned long dwOverrideOpts, const char* szMsg, const char* szFile, unsigned long dwLine ) ;

/*----------------------------------------------------------------------
FUNCTION        :   DiagOutput
DISCUSSION      :
    Provides a tracing routine to send strings through
OutputDebugString.
PARAMETERS      :
    szFmt - The format string.
    ...   - Parameters that will be expanded into szFmt.
RETURNS         :
    None.
----------------------------------------------------------------------*/
void BUGSUTIL_DLLINTERFACE __stdcall
    DiagOutput ( LPCTSTR szFmt , ... ) ;

/*//////////////////////////////////////////////////////////////////////
                           _DEBUG Is Defined
//////////////////////////////////////////////////////////////////////*/
#ifdef _DEBUG

/*//////////////////////////////////////////////////////////////////////
                                Defines
//////////////////////////////////////////////////////////////////////*/
// The different options that can be set with SetDiagAssertOptions to
//  set the global options.  If any of these are passed to DiagAssert in
//  the first parameter, then that value will override whatever the
//  global settings are.

// Undefine the things I want to take over.
#ifdef ASSERT
#undef ASSERT
#endif

#ifdef assert
#undef assert
#endif

#ifdef VERIFY
#undef VERIFY
#endif

// The assert macro used by ASSERT and SUPERASSERT
#define ASSERTMACRO(a,x)                                            \
    do                                                              \
    {                                                               \
        if ( !(x)                                               &&  \
             DiagAssert ( a , _T ( #x ) , __FILE__  , __LINE__)    )\
        {                                                           \
                DebugBreak ( ) ;                                    \
        }                                                           \
    } while (0)

// The normal assert.  It just uses whatever the module defaults.
#define ASSERT(x) ASSERTMACRO(DA_USEDEFAULTS,x)

// Do the lowercase one.
#define assert ASSERT

// Trust, but verify.
#define VERIFY(x)   ASSERT(x)

// Full blow assert with all the trimmings.
#define SUPERASSERT(x) ASSERTMACRO ( DA_SHOWSTACKTRACE |    \
                                        DA_SHOWMSGBOX  |    \
                                        DA_SHOWODS      ,   \
                                     x                  , )

// The options macro.
#define SETDIAGASSERTOPTIONS(x) SetDiagAssertOptions(x)

// The add module macro.
#define ADDDIAGASSERTMODULE(x) AddDiagAssertModule(x)

#ifdef TRACE
#undef TRACE
#endif

#ifdef TRACE0
#undef TRACE0
#endif
#ifdef TRACE1
#undef TRACE1
#endif
#ifdef TRACE2
#undef TRACE2
#endif
#ifdef TRACE3
#undef TRACE3
#endif

// The TRACE macros.
#ifdef __cplusplus
#define TRACE   ::DiagOutput
#endif

#define TRACE0(sz)              ::DiagOutput(_T("%s"), _T(sz))
#define TRACE1(sz, p1)          ::DiagOutput(_T(sz), p1)
#define TRACE2(sz, p1, p2)      ::DiagOutput(_T(sz), p1, p2)
#define TRACE3(sz, p1, p2, p3)  ::DiagOutput(_T(sz), p1, p2, p3)

#else   // !_DEBUG
/*//////////////////////////////////////////////////////////////////////
                       _DEBUG Is !!NOT!! Defined
//////////////////////////////////////////////////////////////////////*/

#define ASSERTMACRO(a,x)
#define ASSERT(x)
#define VERIFY(x)   ((void)(x))
#define SUPERASSERT(x)
#define SETDIAGASSERTOPTIONS(x)
#define ADDDIAGASSERTMODULE(x)

#ifdef __cplusplus
//inline void TraceOutput(LPCTSTR, ...) { }
#if _MSC_VER >= 1310
#define TRACE __noop
#else
#define TRACE   (void)0
#endif

#endif

#define TRACE0(fmt)
#define TRACE1(fmt,arg1)
#define TRACE2(fmt,arg1,arg2)
#define TRACE3(fmt,arg1,arg2,arg3)

#endif  // _DEBUG


#ifndef __cplusplus
}
#endif  //__cplusplus

#endif  // _DIAGASSERT_H


