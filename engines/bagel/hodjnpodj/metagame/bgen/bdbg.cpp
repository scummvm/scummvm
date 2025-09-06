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
// returns: void
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
	return FALSE;
}

//* CBdbgMgr::GetDebugInt -- get debugging integer
int CBdbgMgr::GetDebugInt(LPCSTR lpszOption, int iDefault)
// lpszOption -- option name string
// iDefault -- default value
// returns: debugging integer value
{
	return iDefault;
}



//* CBdbgMgr::GetDebugString --
BOOL CBdbgMgr::GetDebugString(LPCSTR lpszOption, LPSTR lpszTarget, int iTargetSize, LPCSTR lpszDefault)
// lpszOption -- option name string
// lpszDefault -- default value; if nullptr, then default is null string
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::TraceConstructor -- trace object constructor, if optioned
BOOL CBdbgMgr::TraceConstructor(LPCSTR lpszName, void *lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::TraceDestructor -- trace object destructor, if optioned
BOOL CBdbgMgr::TraceDestructor(LPCSTR lpszName, void *lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::DebugMessageBox --
BOOL CBdbgMgr::DebugMessageBox(LPCSTR lpszPrompt, UINT nType, UINT nIDPrompt)
// lpszPrompt -- message string, starting with '~'
// nType, nIDPrompt -- integer Common::sprintf_s substitutions
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::AddTraceObject -- add object to trace list
BOOL CBdbgMgr::AddTraceObject(LPCSTR lpszName, void *lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::TestTraceObject -- test
BOOL CBdbgMgr::TestTraceObject(LPCSTR lpszName, void *lpPtr, BOOL bMissing)
// lpszName -- name of object
// lpPtr -- pointer to object
// bMissing -- if TRUE, then correct if pointer is missing;
//		if FALSE, then correct if pointer is present -- in
//		case of array overflow, this test can't be made
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::RemoveTraceObject -- add object to trace list
BOOL CBdbgMgr::RemoveTraceObject(LPCSTR lpszName, void *lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

//* CBdbgMgr::ReportTraceObjects -- final report at program end
BOOL CBdbgMgr::ReportTraceObjects(void)
//// int FAR PASCAL CBdbgMgr::ReportTraceObjects(void)
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}


//* CBdbgMgr::OutputWithTime -- output debugging string with time
// lpszPattern -- Common::sprintf_s string containing %s for time substitution
// returns: TRUE if error, FALSE otherwise
BOOL CBdbgMgr::OutputWithTime(LPCSTR lpszPattern) {
	return FALSE;
}


//* CBdbgMgr::OutputWithWordWrap -- output debugging string with time
BOOL CBdbgMgr::OutputWithWordWrap(LPCSTR lpStr1, LPCSTR lpStr2, int iIndent)
// lpStr1, lpStr2 -- strings to be concatenated and outputted, with
//		the second one (only) processed for word wrap
// iIndent -- # blanks to indent after first line
// returns: TRUE if error, FALSE otherwise
{
	return FALSE;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
