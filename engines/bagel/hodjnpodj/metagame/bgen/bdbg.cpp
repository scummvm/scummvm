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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/bgen/bdbg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

///DEFS bdbg.h

CBdbgMgr FAR *CBdbgMgr::lpBdbgMgr = nullptr ;

//* CBdbgMgr::CBdbgMgr -- constructor
CBdbgMgr::CBdbgMgr(void) {
	JXENTER(CBdbgMgr::CBdbgMgr) ;

	// zero out all data
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
	lpBdbgMgr = this ;

// cleanup:

	JXELEAVE(CBdbgMgr::CBdbgMgr) ;
	RETURN_VOID ;
}

//* CBdbgMgr::~CBdbgMgr -- destructor
CBdbgMgr::~CBdbgMgr(void)
// returns: VOID
{
	JXENTER(CBdbgMgr::~CBdbgMgr) ;

	if (m_lpTraceObjects)
		delete [] m_lpTraceObjects ;

	m_lpTraceObjects = nullptr ;

	lpBdbgMgr = nullptr ;

// cleanup:

	JXELEAVE(CBdbgMgr::~CBdbgMgr) ;
	RETURN_VOID ;
}


//* CBdbgMgr::DebugInit -- Initialize
BOOL CBdbgMgr::DebugInit(LPCSTR lpszIniFilename, LPCSTR lpszIniSectionname)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::DebugInit) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	int iK ;        // loop variable
	char szStr[100] ;

	strncpy(m_szIniFilename, lpszIniFilename,
	          sizeof(m_szIniFilename)) ;
	strncpy(m_szIniSectionname, lpszIniSectionname,
	          sizeof(m_szIniSectionname)) ;

	m_bDebug = GetDebugInt("debug") ;
	m_bDebugMessages = GetDebugInt("debugmessages") ;
	m_bTimeMessage = GetDebugInt("timemessage") ;
	m_iConstructorMsgLevel = GetDebugInt("constructormsglevel") ;
	m_bVerifyDc = GetDebugInt("verifydc") ;
	m_bTrack = GetDebugInt("track") ;
	m_bTrace = GetDebugInt("trace") ;
	m_bTraceError = GetDebugInt("traceerror") ;

	dbgtrk = m_bTrack ;
	dbgtrr = m_bTraceError ;
	dbgtrc = m_bTrace ;

	GetDebugString("debugoutput", dbgfile, DBGFILESIZE, nullptr) ;
	dbgreopen = GetDebugInt("reopen", MAXPOSINT) ;

	for (iK = 0 ; iK < DIMENSION(m_iDebugValues) ; ++iK) {
		Common::sprintf_s(szStr, sizeof(szStr) -1, "debug%d", iK) ;
		m_iDebugValues[iK] = GetDebugInt(szStr) ;
	}

	m_iTraceObjectCount = GetDebugInt("traceobjectcount") ;

	if (m_iTraceObjectCount) {
		if (m_iTraceObjectCount < 100)
			m_iTraceObjectCount = 100 ;
		if (!m_lpTraceObjects)
			m_lpTraceObjects = new LPVOID[m_iTraceObjectCount + 1] ;
	}

	if (m_bDebugMessages || m_bTimeMessage)
		OutputWithTime("\n\nDebug Output File: %s.\n\n") ;

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::DebugInit) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::GetDebugInt -- get debugging integer
int CBdbgMgr::GetDebugInt(LPCSTR lpszOption, int iDefault)
// lpszOption -- option name string
// iDefault -- default value
// returns: debugging integer value
{
	JXENTER(CBdbgMgr::GetDebugInt) ;
	int iRetval = iDefault;
	#ifdef JX_DEBUG
	iRetval = GetPrivateProfileInt(m_szIniSectionname, lpszOption, iDefault, m_szIniFilename) ;

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::GetDebugInt) ;
	RETURN(iRetval) ;
}



//* CBdbgMgr::GetDebugString --
BOOL CBdbgMgr::GetDebugString(LPCSTR lpszOption, LPSTR lpszTarget, int iTargetSize, LPCSTR lpszDefault)
// lpszOption -- option name string
// lpszDefault -- default value; if nullptr, then default is null string
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::GetDebugString) ;
	int iError = 0 ;        // error code

	#ifdef JX_DEBUG
	if (!lpszDefault)
		lpszDefault = "";

	GetPrivateProfileString(m_szIniSectionname,
	    lpszOption, lpszDefault, lpszTarget, iTargetSize,
	    m_szIniFilename) ;

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::GetDebugString) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::TraceConstructor -- trace object constructor, if optioned
BOOL CBdbgMgr::TraceConstructor(LPCSTR lpszName, LPVOID lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::TraceConstructor) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	char szStr[100] ;

	if (m_iConstructorMsgLevel) {
		Common::sprintf_s(szStr, sizeof(szStr) - 1, "Constructing %Fs at %#04x:%#04x.\n",
		          lpszName, HIWORD(lpLoc), LOWORD(lpLoc)) ;
		JXOutputDebugString(szStr) ;
	}

	iError = AddTraceObject(lpszName, lpLoc) ;

// cleanup:
	#endif

	JXELEAVE(CBdbgMgr::TraceConstructor) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::TraceDestructor -- trace object destructor, if optioned
BOOL CBdbgMgr::TraceDestructor(LPCSTR lpszName, LPVOID lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::TraceDestructor) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	char szStr[100] ;

	if (m_iConstructorMsgLevel) {
		Common::sprintf_s(szStr, sizeof(szStr) -1, "Destructing %Fs at %#04x:%#04x.\n",
		          lpszName, HIWORD(lpLoc), LOWORD(lpLoc)) ;
		JXOutputDebugString(szStr) ;
	}

	iError = RemoveTraceObject(lpszName, lpLoc) ;

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::TraceDestructor) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::DebugMessageBox --
BOOL CBdbgMgr::DebugMessageBox(LPCSTR lpszPrompt, UINT nType, UINT nIDPrompt)
// lpszPrompt -- message string, starting with '~'
// nType, nIDPrompt -- integer Common::sprintf_s substitutions
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::DebugMessageBox) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	BOOL bDone = FALSE ;
	char szStr[100] ;

	if (lpszPrompt[1] == '~')
		switch (lpszPrompt[2]) {
		case 'C':
			TraceConstructor(lpszPrompt + 3, (LPVOID)MAKELONG(nType, nIDPrompt));
			bDone = TRUE ;
			break ;

		case 'D':
			TraceDestructor(lpszPrompt + 3, (LPVOID)MAKELONG(nType, nIDPrompt)) ;
			bDone = TRUE ;
			break ;

		default:
			break ;
		}


	if (!bDone) {
		Common::sprintf_s(szStr, sizeof(szStr) -1, lpszPrompt + 1, nType, nIDPrompt);
		JXOutputDebugString(szStr) ;
	}

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::DebugMessageBox) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::AddTraceObject -- add object to trace list
BOOL CBdbgMgr::AddTraceObject(LPCSTR lpszName, LPVOID lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::AddTraceObject) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	int iK ;        // loop variable
	int iPosition = -1 ;    // free position in array
	char szStr[100] ;

	if (!lpPtr) {
		Common::sprintf_s(szStr, sizeof(szStr) - 1,
		          "AddTraceObject error: %Fs %#04x:%#04x "
		          "invalid pointer.\n", lpszName,
		          HIWORD(lpPtr), LOWORD(lpPtr)) ;
		JXOutputDebugString(szStr) ;
		++m_iErrorCount ;
		iError = 100 ;
		goto cleanup ;
	}

	if (lpPtr && m_lpTraceObjects) {
		for (iK = 0 ; iK < m_iTraceObjectCurrent ; ++iK)
			if (!m_lpTraceObjects[iK])
				iPosition = iK ;

			else if (m_lpTraceObjects[iK] == lpPtr) {
				Common::sprintf_s(szStr, sizeof(szStr) - 1,
				          "AddTraceObject error: %Fs %#04x:%#04x "
				          "already in array.\n", lpszName,
				          HIWORD(lpPtr), LOWORD(lpPtr)) ;
				JXOutputDebugString(szStr) ;
				++m_iErrorCount ;
				iError = 101 ;
				goto cleanup ;
			}

		if (iPosition < 0 && m_iTraceObjectCurrent >= m_iTraceObjectCount - 1) {
			JXOutputDebugString("AddTraceObject error: Array overflow.\n") ;
			++m_iErrorCount ;
			iError = 102 ;
			goto cleanup ;
		}

		if (iPosition < 0)
			iPosition = m_iTraceObjectCurrent++ ;

		m_lpTraceObjects[iPosition] = lpPtr ;
	}

cleanup:
	#endif
	JXELEAVE(CBdbgMgr::AddTraceObject) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::TestTraceObject -- test
BOOL CBdbgMgr::TestTraceObject(LPCSTR lpszName, LPVOID lpPtr, BOOL bMissing)
// lpszName -- name of object
// lpPtr -- pointer to object
// bMissing -- if TRUE, then correct if pointer is missing;
//		if FALSE, then correct if pointer is present -- in
//		case of array overflow, this test can't be made
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::TestTraceObject) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	int iK ;        // loop variable
	int iPosition = -1 ;
	char szStr[100] ;

	if (!lpPtr) {
		Common::sprintf_s(szStr, sizeof(szStr) - 1,
		          "TestTraceObject error: %Fs %#04x:%#04x "
		          "invalid pointer.\n", lpszName,
		          HIWORD(lpPtr), LOWORD(lpPtr)) ;
		JXOutputDebugString(szStr) ;
		++m_iErrorCount ;
		iError = 100 ;
		goto cleanup ;
	}

	if (lpPtr && m_lpTraceObjects) {
		for (iK = 0 ; iPosition < 0
		        && iK < m_iTraceObjectCurrent ; ++iK)
			if (m_lpTraceObjects[iK] == lpPtr)
				iPosition = iK ;

		if (bMissing && iPosition >= 0) {
			Common::sprintf_s(szStr, sizeof(szStr) - 1,
			          "TestTraceObject error: %Fs %#04x:%#04x "
			          "already in array.\n", lpszName,
			          HIWORD(lpPtr), LOWORD(lpPtr)) ;
			JXOutputDebugString(szStr) ;
			++m_iErrorCount ;
			iError = 101 ;
			goto cleanup ;
		}

		if (!bMissing && iPosition < 0
		        && m_iTraceObjectCurrent < m_iTraceObjectCount - 1) {
			Common::sprintf_s(szStr, sizeof(szStr) - 1,
			          "TestTraceObject error: %Fs %#04x:%#04x "
			          "is not in array.\n", lpszName,
			          HIWORD(lpPtr), LOWORD(lpPtr)) ;
			JXOutputDebugString(szStr) ;
			++m_iErrorCount ;
			iError = 102 ;
			goto cleanup ;
		}
	}

cleanup:
	#endif
	JXELEAVE(CBdbgMgr::TestTraceObject) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::RemoveTraceObject -- add object to trace list
BOOL CBdbgMgr::RemoveTraceObject(LPCSTR lpszName, LPVOID lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::RemoveTraceObject) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	int iK ;        // loop variable
	int iPosition = -1 ;
	char szStr[100] ;

	if (!lpPtr) {
		Common::sprintf_s(szStr, sizeof(szStr) - 1,
		          "RemoveTraceObject error: %Fs %#04x:%#04x "
		          "invalid pointer.\n", lpszName,
		          HIWORD(lpPtr), LOWORD(lpPtr)) ;
		JXOutputDebugString(szStr) ;
		++m_iErrorCount ;
		iError = 100 ;
		goto cleanup ;
	}

	if (lpPtr && m_lpTraceObjects) {
		for (iK = 0 ; iPosition < 0
		        && iK < m_iTraceObjectCurrent ; ++iK)
			if (m_lpTraceObjects[iK] == lpPtr)
				iPosition = iK ;

		if (iPosition >= 0)
			m_lpTraceObjects[iPosition] = nullptr ;

		else if (m_iTraceObjectCurrent < m_iTraceObjectCount - 1) {
			Common::sprintf_s(szStr, sizeof(szStr) - 1,
			          "RemoveTraceObject error: %Fs %#04x:%#04x "
			          "is not in array.\n", lpszName,
			          HIWORD(lpPtr), LOWORD(lpPtr)) ;
			JXOutputDebugString(szStr) ;
			++m_iErrorCount ;
			iError = 101 ;
			goto cleanup ;
		}
	}

cleanup:

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::RemoveTraceObject) ;
	RETURN(iError != 0) ;
}

//* CBdbgMgr::ReportTraceObjects -- final report at program end
BOOL CBdbgMgr::ReportTraceObjects(void)
//// int FAR PASCAL CBdbgMgr::ReportTraceObjects(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::ReportTraceObjects) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	int iUnfreed = 0 ;      // number of unfreed objects
	LPVOID lpPtr ;
	int iK ;        // loop variable
	char szStr[100] ;

	if (m_lpTraceObjects) {
		JXOutputDebugString("\nCBdbgMgr final report:\n") ;

		if (m_iErrorCount) {
			Common::sprintf_s(szStr, sizeof(szStr) - 1, "   Previous errors: %d\n", m_iErrorCount) ;
			JXOutputDebugString(szStr) ;
		}

		for (iK = 0 ; iK < m_iTraceObjectCurrent ; ++iK)
			if ((lpPtr = m_lpTraceObjects[iK]) != nullptr) {
				Common::sprintf_s(szStr, sizeof(szStr) - 1, "    Unfreed object: %#04x:%#04x.\n",
				          HIWORD(lpPtr), LOWORD(lpPtr)) ;
				JXOutputDebugString(szStr) ;
				++iUnfreed ;
			}

		if (iUnfreed) {
			Common::sprintf_s(szStr, sizeof(szStr) - 1, "    Total unfreed objects: %d.\n", iUnfreed);
			JXOutputDebugString(szStr) ;
		}

		if (!m_iErrorCount && !iUnfreed)
			JXOutputDebugString("    No errors encountered.\n") ;
	}

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::ReportTraceObjects) ;
	RETURN(iError != 0) ;
}


//* CBdbgMgr::OutputWithTime -- output debugging string with time
// lpszPattern -- Common::sprintf_s string containing %s for time substitution
// returns: TRUE if error, FALSE otherwise
BOOL CBdbgMgr::OutputWithTime(LPCSTR lpszPattern) {
	JXENTER(CBdbgMgr::OutputWithTime) ;
	int iError = 0 ;        // error code
	#if 0
	#ifdef JX_DEBUG
	char szMsg[100] ;

	TimeDat
	struct tm *newtime;
	time_t aclock;

	time(&aclock);                   /* Get time in seconds */
	newtime = localtime(&aclock);

	Common::sprintf_s(szMsg, sizeof(szMsg) -1, lpszPattern, asctime(newtime)) ;
	JXOutputDebugString(szMsg) ;

// cleanup:
	#endif
	#endif
	JXELEAVE(CBdbgMgr::OutputWithTime) ;
	RETURN(iError != 0) ;
}


//* CBdbgMgr::OutputWithWordWrap -- output debugging string with time
BOOL CBdbgMgr::OutputWithWordWrap(LPCSTR lpStr1, LPCSTR lpStr2, int iIndent)
// lpStr1, lpStr2 -- strings to be concatenated and outputted, with
//		the second one (only) processed for word wrap
// iIndent -- # blanks to indent after first line
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBdbgMgr::OutputWithWordWrap) ;
	int iError = 0 ;        // error code
	#ifdef JX_DEBUG
	char szMsg[100] ;
	BOOL bSecond = FALSE ;
	int iK = 0, iBlank = 0;        // index variables
	LPCSTR lpStr = lpStr1, lpBlank = nullptr;
	char cChar ;

	while (((cChar = *lpStr++) != '\0') || !bSecond) {

		if (!cChar)
			lpStr = lpStr2, lpBlank = nullptr, bSecond = TRUE ;

		else {
			szMsg[iK++] = cChar ;

			if (cChar == ' ')
				lpBlank = lpStr, iBlank = iK ;

			if (iK >= 80) {
				if (lpBlank)
					lpStr = lpBlank, iK = iBlank - 1;
				szMsg[iK++] = '\n', szMsg[iK] = 0 ;

				JXOutputDebugString(szMsg) ;
				for (iK = 0 ; iK < iIndent ;)
					szMsg[iK++] = ' ' ;
			}
		}
	}

	if (iK > 0) {
		szMsg[iK] = 0 ;
		JXOutputDebugString(szMsg) ;
	}

// cleanup:
	#endif
	JXELEAVE(CBdbgMgr::OutputWithWordWrap) ;
	RETURN(iError != 0) ;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
