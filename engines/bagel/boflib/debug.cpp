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

#include "common/textconsole.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define DEFAULT_DEBUG_LEVEL 4

CBofDebugOptions *g_pDebugOptions = NULL;
CBofLog *g_pDebugLog = NULL;

CBofDebugOptions::CBofDebugOptions(const CHAR *pszFileName) : CBofOptions(pszFileName) {
	// Add programmer definable debug options here
	ReadSetting("DebugOptions", "AbortsOn", &m_bAbortsOn, TRUE);
	ReadSetting("DebugOptions", "MessageBoxOn", &m_bMessageBoxOn, TRUE);
	ReadSetting("DebugOptions", "RandomOn", &m_bRandomOn, TRUE);
	ReadSetting("DebugOptions", "DebugLevel", &m_nDebugLevel, DEFAULT_DEBUG_LEVEL);
	ReadSetting("DebugOptions", "ShowIO", &m_bShowIO, FALSE);
	ReadSetting("DebugOptions", "MessageSpy", &m_bShowMessages, FALSE);
}

VOID BofAssert(BOOL bExpression, INT nLine, const CHAR *pszSourceFile, const CHAR *pszTimeStamp) {
	static CHAR szBuf[200];
	static BOOL bAlready = FALSE;

	/* Assert fails when expression is false
	 */
	if (!bExpression) {
		if (!bAlready) {
			bAlready = TRUE;

			/* if this compiler supports the __TIMESTAMP__ macro, then show that also
			 */
			Common::sprintf_s(szBuf, "Internal error: File %s at line %d\n", pszSourceFile, nLine);
			if (pszTimeStamp != NULL) {
				Common::sprintf_s(szBuf, "Internal error: File %s at line %d (FileDate: %s)\n", pszSourceFile, nLine, pszTimeStamp);
			}

			/*
			 * write this error to the log file
			 */
			if (g_pDebugLog != NULL) {
				g_pDebugLog->WriteMessage(LOG_ERROR, szBuf, 0, NULL);
			}

			bAlready = FALSE;
		}

		error("%s", szBuf);
	}
}

VOID BofAbort(const CHAR *pszInfo, const CHAR *pszFile, INT nLine) {
	CHAR szBuf[200];

	Common::strcpy_s(szBuf, "Unknown reason for Abort");
	if (pszInfo != NULL) {
		Common::strcpy_s(szBuf, pszInfo);
	}

	// log this message to DEBUG.LOG and to output window
	//
	if (g_pDebugLog != NULL) {
		g_pDebugLog->WriteMessage(LOG_FATAL, szBuf, 0, pszFile, nLine);
	}

	// display message box saying why we are aborting
	//
	if (g_pDebugOptions->m_bMessageBoxOn) {
		warning("%s", szBuf);
	}

	if (g_pDebugOptions->m_bAbortsOn) {
		if (g_pDebugLog != NULL) {
			g_pDebugLog->WriteMessage(LOG_FATAL, "Aborting!");
		}

		error("Aborted");
	}
}

} // namespace Bagel
