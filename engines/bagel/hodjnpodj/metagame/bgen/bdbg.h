/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_METAGAME_BGEN_BDBG_H
#define BAGEL_METAGAME_BGEN_BDBG_H

#include "bagel/hodjnpodj/metagame/bgen/bgen.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define DBGFILESIZE 50

#ifdef JX_DEBUG

#define TRACECONSTRUCTOR(name) \
	AfxGetApp()->DoMessageBox("~~C" #name, \
	                          LOWORD((LPVOID)this), HIWORD((LPVOID)this)) ;

#define TRACEDESTRUCTOR(name) \
	AfxGetApp()->DoMessageBox("~~D" #name, \
	                          LOWORD((LPVOID)this), HIWORD((LPVOID)this)) ;
#else

#define TRACECONSTRUCTOR(name) ;

#define TRACEDESTRUCTOR(name) ;

#endif // JX_DEBUG

// CBdbgMgr -- boffo games debugging manager
class CBdbgMgr {
public:
	static CBdbgMgr FAR *lpBdbgMgr ;    // pointer to this block
	char m_cStartData ;
	char m_szIniFilename[100] ; // .INI file name
	char m_szIniSectionname[15] ;   // .INI file section name
	BOOL m_bDebug ;     // flag -- debugging mode
	BOOL m_bDebugMessages ; // debugging messages
	BOOL m_bTimeMessage ;   // time stamp message
	BOOL m_bTrack, m_bTrace ;   // debugging -- track/trace flags
	BOOL m_bTraceError ;    // debugging -- trace error exits
	int  m_iConstructorMsgLevel ; // level of constructor messages
	BOOL m_bVerifyDc ;      // verify device context is ok
	int  m_iDebugValues[100] ;  // misc debugging values
	int  m_iTraceObjectCount ;  // max # objects to trace
	LPVOID *m_lpTraceObjects ;  // trace object array
	int  m_iTraceObjectCurrent ;    // # of array elements in use
	int  m_iErrorCount ;    // number of errors encountered
	char m_cEndData ;

// methods
	static CBdbgMgr FAR *GetPointer(void) {
		return lpBdbgMgr;
	}

	CBdbgMgr(void) ;
	~CBdbgMgr(void) ;
	BOOL DebugInit(LPCSTR lpszIniFilename,
	               LPCSTR lpszIniSectionname) ;
	int GetDebugInt(LPCSTR lpszOption,
	                int iDefault PDFT(0)) ;
	BOOL GetDebugString(LPCSTR lpszOption,
	                    LPSTR lpszTarget, int iTargetSize,
	                    LPCSTR lpszDefault PDFT(nullptr)) ;
	BOOL TraceConstructor(LPCSTR lpszName, LPVOID lpLoc) ;
	BOOL TraceDestructor(LPCSTR lpszName, LPVOID lpLoc) ;
	BOOL DebugMessageBox(LPCSTR lpszPrompt,
	                     UINT nType, UINT nIDPrompt) ;
	BOOL AddTraceObject(LPCSTR lpszName, LPVOID lpPtr) ;
	BOOL TestTraceObject(LPCSTR lpszName, LPVOID lpPtr,
	                     BOOL bMissing PDFT(FALSE)) ;
	BOOL RemoveTraceObject(LPCSTR lpszName, LPVOID lpPtr) ;
	BOOL ReportTraceObjects(void) ;
	STATIC BOOL OutputWithTime(LPCSTR lpszPattern) ;
	STATIC BOOL OutputWithWordWrap(LPCSTR lpStr1,
	                               LPCSTR lpStr2, int iIndent) ;
} ;

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

	#define JXOutputDebugString ifcwst

	extern LPVOID dbgptr ;
	extern int dbgtrk ;
	extern int dbgtrc ;
	extern int dbgtrr ;

	extern int dbgreopen ;
	extern char dbgfile[DBGFILESIZE] ;

	/* bdbgc.c -- debugging routines */

	//- dbgarg -- handle debugging arguments
	void dbgarg(int argc, char * argv[]) ;
	//- dbgntr -- enter a subroutine
	void dbgntr(const char * name) ;
	//- dbgxit -- exit a subroutine
	void dbgxit(const char * name, int iError) ;
	//- dbgtyp -- type status line
	void dbgtyp(const char * ps, BOOL bReset) ;
	//- dbgntu -- enter a subroutine -- unconditional message
	void dbgntu(const char * name) ;
	//- dbgxiu -- exit a subroutine -- unconditional message
	void dbgxiu(const char * name) ;
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
	//- ifcspf -- Common::sprintf_s replacement
	int ifcspf(char * ds, char * cs, ...) ;
	//- ifcfmt -- formatting routine
	char *ifcfmt(char * cs, char * ds, void * pparg, int * flen) ;
	//- ifcwst -- write character string to screen
	void ifcwst(LPCSTR xpStr) ;
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

#else
	#define JXOutputDebugString TRACE

#endif /* JX_DEBUG */

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
