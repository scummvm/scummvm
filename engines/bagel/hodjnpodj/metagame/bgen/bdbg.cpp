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
CBdbgMgr::CBdbgMgr() {
	JXENTER(CBdbgMgr::CBdbgMgr) ;

	// zero out all data
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
	lpBdbgMgr = this ;

// cleanup:

	JXELEAVE(CBdbgMgr::CBdbgMgr) ;
	RETURN_VOID ;
}

//* CBdbgMgr::~CBdbgMgr -- destructor
CBdbgMgr::~CBdbgMgr()
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
bool CBdbgMgr::DebugInit(const char *lpszIniFilename, const char *lpszIniSectionname)
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::GetDebugInt -- get debugging integer
int CBdbgMgr::GetDebugInt(const char *lpszOption, int iDefault)
// lpszOption -- option name string
// iDefault -- default value
// returns: debugging integer value
{
	return iDefault;
}



//* CBdbgMgr::GetDebugString --
bool CBdbgMgr::GetDebugString(const char *lpszOption, char *lpszTarget, int iTargetSize, const char *lpszDefault)
// lpszOption -- option name string
// lpszDefault -- default value; if nullptr, then default is null string
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::TraceConstructor -- trace object constructor, if optioned
bool CBdbgMgr::TraceConstructor(const char *lpszName, void *lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::TraceDestructor -- trace object destructor, if optioned
bool CBdbgMgr::TraceDestructor(const char *lpszName, void *lpLoc)
// lpszName -- name of object
// lpLoc -- location of object
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::DebugMessageBox --
bool CBdbgMgr::DebugMessageBox(const char *lpszPrompt, unsigned int nType, unsigned int nIDPrompt)
// lpszPrompt -- message string, starting with '~'
// nType, nIDPrompt -- integer Common::sprintf_s substitutions
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::AddTraceObject -- add object to trace list
bool CBdbgMgr::AddTraceObject(const char *lpszName, void *lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::TestTraceObject -- test
bool CBdbgMgr::TestTraceObject(const char *lpszName, void *lpPtr, bool bMissing)
// lpszName -- name of object
// lpPtr -- pointer to object
// bMissing -- if true, then correct if pointer is missing;
//		if false, then correct if pointer is present -- in
//		case of array overflow, this test can't be made
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::RemoveTraceObject -- add object to trace list
bool CBdbgMgr::RemoveTraceObject(const char *lpszName, void *lpPtr)
// lpszName -- name of object
// lpPtr -- pointer to object
// returns: true if error, false otherwise
{
	return false;
}

//* CBdbgMgr::ReportTraceObjects -- final report at program end
bool CBdbgMgr::ReportTraceObjects()
//// int FAR PASCAL CBdbgMgr::ReportTraceObjects()
// returns: true if error, false otherwise
{
	return false;
}


//* CBdbgMgr::OutputWithTime -- output debugging string with time
// lpszPattern -- Common::sprintf_s string containing %s for time substitution
// returns: true if error, false otherwise
bool CBdbgMgr::OutputWithTime(const char *lpszPattern) {
	return false;
}


//* CBdbgMgr::OutputWithWordWrap -- output debugging string with time
bool CBdbgMgr::OutputWithWordWrap(const char *lpStr1, const char *lpStr2, int iIndent)
// lpStr1, lpStr2 -- strings to be concatenated and outputted, with
//		the second one (only) processed for word wrap
// iIndent -- # blanks to indent after first line
// returns: true if error, false otherwise
{
	return false;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
