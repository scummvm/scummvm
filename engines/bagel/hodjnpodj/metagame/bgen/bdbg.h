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
	static CBdbgMgr FAR *lpBdbgMgr;			// pointer to this block
	char m_cStartData = 0;
	char m_szIniFilename[100] = {};			// .INI file name
	char m_szIniSectionname[15] = {};		// .INI file section name
	bool m_bDebug = false;					// flag -- debugging mode
	bool m_bDebugMessages = false;			// debugging messages
	bool m_bTimeMessage = false;			// time stamp message
	bool m_bTrack = false, m_bTrace = false; // debugging -- track/trace flags
	bool m_bTraceError = false;				// debugging -- trace error exits
	int  m_iConstructorMsgLevel = 0;		// level of constructor messages
	bool m_bVerifyDc = false;				// verify device context is ok
	int  m_iDebugValues[100] = {};			// misc debugging values
	int  m_iTraceObjectCount = 0;			// max # objects to trace
	void **m_lpTraceObjects = nullptr;		// trace object array
	int  m_iTraceObjectCurrent = 0;			// # of array elements in use
	int  m_iErrorCount = 0;					// number of errors encountered

	// methods
	static CBdbgMgr FAR *GetPointer() {
		return lpBdbgMgr;
	}

	CBdbgMgr() ;
	~CBdbgMgr() ;
	bool DebugInit(const char *lpszIniFilename,
	               const char *lpszIniSectionname) ;
	int GetDebugInt(const char *lpszOption,
	                int iDefault PDFT(0)) ;
	bool GetDebugString(const char *lpszOption,
	                    char *lpszTarget, int iTargetSize,
	                    const char *lpszDefault PDFT(nullptr)) ;
	bool TraceConstructor(const char *lpszName, void *lpLoc) ;
	bool TraceDestructor(const char *lpszName, void *lpLoc) ;
	bool DebugMessageBox(const char *lpszPrompt,
	                     unsigned int nType, unsigned int nIDPrompt) ;
	bool AddTraceObject(const char *lpszName, void *lpPtr) ;
	bool TestTraceObject(const char *lpszName, void *lpPtr,
	                     bool bMissing PDFT(false)) ;
	bool RemoveTraceObject(const char *lpszName, void *lpPtr) ;
	bool ReportTraceObjects() ;
	STATIC bool OutputWithTime(const char *lpszPattern) ;
	STATIC bool OutputWithWordWrap(const char *lpStr1,
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
