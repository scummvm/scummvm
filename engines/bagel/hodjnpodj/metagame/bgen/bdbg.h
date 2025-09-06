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

#define TRACECONSTRUCTOR(name) ;

#define TRACEDESTRUCTOR(name) ;

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
	void **m_lpTraceObjects ;  // trace object array
	int  m_iTraceObjectCurrent ;    // # of array elements in use
	int  m_iErrorCount ;    // number of errors encountered
	char m_cEndData ;

// methods
	static CBdbgMgr FAR *GetPointer(void) {
		return lpBdbgMgr;
	}

	CBdbgMgr(void) ;
	~CBdbgMgr(void) ;
	BOOL DebugInit(const char *lpszIniFilename,
	               const char *lpszIniSectionname) ;
	int GetDebugInt(const char *lpszOption,
	                int iDefault PDFT(0)) ;
	BOOL GetDebugString(const char *lpszOption,
	                    char *lpszTarget, int iTargetSize,
	                    const char *lpszDefault PDFT(nullptr)) ;
	BOOL TraceConstructor(const char *lpszName, void *lpLoc) ;
	BOOL TraceDestructor(const char *lpszName, void *lpLoc) ;
	BOOL DebugMessageBox(const char *lpszPrompt,
	                     unsigned int nType, unsigned int nIDPrompt) ;
	BOOL AddTraceObject(const char *lpszName, void *lpPtr) ;
	BOOL TestTraceObject(const char *lpszName, void *lpPtr,
	                     BOOL bMissing PDFT(FALSE)) ;
	BOOL RemoveTraceObject(const char *lpszName, void *lpPtr) ;
	BOOL ReportTraceObjects(void) ;
	STATIC BOOL OutputWithTime(const char *lpszPattern) ;
	STATIC BOOL OutputWithWordWrap(const char *lpStr1,
	                               const char *lpStr2, int iIndent) ;
} ;

#ifndef JXENTER
		#define JXENTER(name)
		#define JXLEAVE(name)
		#define JXELEAVE(name)
#endif /* ifndef JXENTER */

#define RETURN(x) return(x)
#define RETURN_VOID return

#define JXOutputDebugString TRACE

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
