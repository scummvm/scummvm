// bdbg.h -- Boffo Debugging Header
// Written by John J. Xenakis, 1994, for Boffo Games Inc.

#ifndef __bdbg_H__
#define __bdbg_H__

#include "bgen.h"
#include "hhead.h"

#ifdef __cplusplus

#define TRACECONSTRUCTOR(name) \
	AfxGetApp()->DoMessageBox("~~C" #name, \
		LOWORD((LPVOID)this), HIWORD((LPVOID)this)) ;

#define TRACEDESTRUCTOR(name) \
	AfxGetApp()->DoMessageBox("~~D" #name, \
		LOWORD((LPVOID)this), HIWORD((LPVOID)this)) ;

// CBdbgMgr -- boffo games debugging manager
class CBdbgMgr {
public:
    static CBdbgMgr FAR * lpBdbgMgr ;	// pointer to this block
    char m_cStartData ;
    char m_szIniFilename[100] ;	// .INI file name
    char m_szIniSectionname[15] ;	// .INI file section name
    BOOL m_bDebug ;		// flag -- debugging mode
    BOOL m_bDebugMessages ;	// debugging messages
    BOOL m_bTimeMessage ;	// time stamp message
    BOOL m_bTrack, m_bTrace ;	// debugging -- track/trace flags
    BOOL m_bTraceError ;	// debugging -- trace error exits
    int  m_iConstructorMsgLevel ; // level of constructor messages
    BOOL m_bVerifyDc ;		// verify device context is ok
    int  m_iDebugValues[100] ;	// misc debugging values
    int  m_iTraceObjectCount ;	// max # objects to trace
    LPVOID * m_lpTraceObjects ;	// trace object array
    int  m_iTraceObjectCurrent ;	// # of array elements in use
    int  m_iErrorCount ;	// number of errors encountered
    char m_cEndData ;

// methods
    static CBdbgMgr FAR * GetPointer(void) { return(lpBdbgMgr) ; }



// bdbg.cpp -- Boffo debugging for meta game

//- CBdbgMgr::CBdbgMgr -- constructor
PUBLIC CBdbgMgr::CBdbgMgr(void) ;
//- CBdbgMgr::~CBdbgMgr -- destructor
PUBLIC CBdbgMgr::~CBdbgMgr(void) ;
//- CBdbgMgr::DebugInit -- Initialize
PUBLIC BOOL CBdbgMgr::DebugInit(LPSTR lpszIniFilename,
			LPSTR lpszIniSectionname) ;
//- CBdbgMgr::GetDebugInt -- get debugging integer
PUBLIC int CBdbgMgr::GetDebugInt(LPSTR lpszOption,
			int iDefault PDFT(0)) ;
//- CBdbgMgr::GetDebugString -- 
PUBLIC BOOL CBdbgMgr::GetDebugString(LPCSTR lpszOption,
    	LPSTR lpszTarget, int iTargetSize,
		LPSTR lpszDefault PDFT(NULL)) ;
//- CBdbgMgr::TraceConstructor -- trace object constructor, if optioned
PUBLIC BOOL CBdbgMgr::TraceConstructor(LPCSTR lpszName, LPVOID lpLoc) ;
//- CBdbgMgr::TraceDestructor -- trace object destructor, if optioned
PUBLIC BOOL CBdbgMgr::TraceDestructor(LPCSTR lpszName, LPVOID lpLoc) ;
//- CBdbgMgr::DebugMessageBox -- 
PUBLIC BOOL CBdbgMgr::DebugMessageBox(LPCSTR lpszPrompt,
	UINT nType, UINT nIDPrompt) ;
//- CBdbgMgr::AddTraceObject -- add object to trace list
PUBLIC BOOL CBdbgMgr::AddTraceObject(LPCSTR lpszName, LPVOID lpPtr) ;
//- CBdbgMgr::TestTraceObject -- test 
PUBLIC BOOL CBdbgMgr::TestTraceObject(LPCSTR lpszName, LPVOID lpPtr,
    			BOOL bMissing PDFT(FALSE)) ;
//- CBdbgMgr::RemoveTraceObject -- add object to trace list
PUBLIC BOOL CBdbgMgr::RemoveTraceObject(LPCSTR lpszName, LPVOID lpPtr) ;
//- CBdbgMgr::ReportTraceObjects -- final report at program end
PUBLIC BOOL CBdbgMgr::ReportTraceObjects(void) ;
//- CBdbgMgr::OutputWithTime -- output debugging string with time
PUBLIC STATIC BOOL CBdbgMgr::OutputWithTime(LPSTR lpszPattern) ;
//- CBdbgMgr::OutputWithWordWrap -- output debugging string with time
PUBLIC STATIC BOOL CBdbgMgr::OutputWithWordWrap(LPSTR lpStr1,
			LPSTR lpStr2, int iIndent) ;






} ;
#endif	/* __cplusplus */


#ifndef JXENTER
#ifdef JX_DEBUG
#define JXENTER(name) dbgntr(#name)
#define JXLEAVE(name) dbgxit(#name,0)
#define JXELEAVE(name) dbgxit(#name, iError)
#else
#define JXENTER(name)
#define JXLEAVE(name)
#define JXELEAVE(name)
#endif /* ifdef JX_DEBUG */
#endif /* ifndef JXENTER */
#define RETURN(x) return(x)
#define RETURN_VOID return

#ifdef JX_DEBUG

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

#define JXOutputDebugString ifcwst

extern LPVOID dbgptr ;
extern int dbgtrk ;
extern int dbgtrc ;
extern int dbgtrr ;
#define DBGFILESIZE 50
extern int dbgreopen ;
extern char dbgfile[DBGFILESIZE] ;

/* bdbgc.c -- debugging routines */

//- dbgarg -- handle debugging arguments
void dbgarg(int argc, char * argv[]) ;
//- dbgntr -- enter a subroutine
void dbgntr(char * name) ;
//- dbgxit -- exit a subroutine
void dbgxit(char * name, int iError) ;
//- dbgtyp -- type status line
void dbgtyp(char * ps, BOOL bReset) ;
//- dbgntu -- enter a subroutine -- unconditional message
void dbgntu(char * name) ;
//- dbgxiu -- exit a subroutine -- unconditional message
void dbgxiu(char * name) ;
//- dbgttg -- keyboard trace toggle hit
void dbgttg(void) ;
//- dbggsd -- get current stack depth
int dbggsd(void) ;
//- dbgssd -- set stack depth
void dbgssd(int depth) ;
//- dbgcnp -- check null pointer storage area clobbered
void dbgcnp(char * caller, char * name) ;
//- kybupq -- input chars, update keyboard input queue
void kybupq(void) ;
//- kybrd -- keyboard read -- wait if no char available
int kybrd(void) ;
//- kybrdz -- keyboard read, return zero if no char
int kybrdz(void) ;
//- kybord -- o/s keyboard read -- returns char or scan code + 1000
int kybord(void) ;
//- kybtst -- test for keyboard input, return char, leave in queue
// Quick test for keyboard input -- returns next char, but
//		doesn't remove it from queue
int kybtst(void) ;
//- kybnlt -- new line test -- test if there's a newline in queue
int kybnlt(void) ;
//- ifcprf -- printf replacement
void ifcprf(char * cs, ...) ;
//- ifcspf -- sprintf replacement
int ifcspf(char * ds, char * cs, ...) ;
//- ifcfmt -- formatting routine
char * ifcfmt(char * cs, char * ds, void * pparg, int * flen) ;
//- ifcwst -- write character string to screen
void ifcwst(XPSTR xpStr) ;
//- ifcwtt -- write character to screen
void ifcwtt(char ch) ;
//- ifcwnl -- write newline
void ifcwnl(void) ;
//- ifcbep -- "display" a beep
void ifcbep(void) ;
//- ifcwwf -- write character to windows file
void ifcwwf(int iChar) ;
//- dbgWindowDump -- dump current window tree
VOID FAR PASCAL dbgWindowDump(HWND hStartWnd) ;
//- dbgDumpEnumProc - enumeration function to find child windows
BOOL FAR PASCAL dbgDumpEnumProc(HWND hWnd, DWORD dwlpWddb) ;
//- dbgDumpWindowInfo - dump info on one window
VOID dbgDumpWindowInfo(HWND hWnd, DWORD dwlpWddb) ;



#ifdef __cplusplus
}  /* extern "C" */
#endif	/* __cplusplus */


#endif /* JX_DEBUG */



#include "htail.h"

#endif // __bdbg_H__



