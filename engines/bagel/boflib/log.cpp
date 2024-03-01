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

#include "common/str.h"
#include "common/debug.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

static const CHAR *const g_pszLogTypes[4] = {
	"Fatal Error: ",
	"Error: ",
	"Warning: ",
	""
};

#define MAX_LOG_LINE_LEN 400

CBofLog::CBofLog(const CHAR *pszFileName, ULONG lOptions) {
	// Inits
	//
	m_szFileName[0] = '\0';
	m_lOptions = lOptions;

	if (pszFileName != nullptr) {
		SetLogFile(pszFileName);
	}
}

#if BOF_DEBUG
CBofLog::~CBofLog() {
	Assert(IsValidObject(this));

	WriteMessage(LOG_INFO, "Log Session Closed");
}
#endif

VOID CBofLog::SetLogFile(const CHAR *pszFileName) {
	Assert(IsValidObject(this));

	Assert(pszFileName != nullptr);
	Assert(*pszFileName != '\0');
	Assert(strlen(pszFileName) < MAX_FNAME);

	Common::strcpy_s(m_szFileName, pszFileName);

#if BOF_DEBUG
	CHAR szTimeBuf[12], szDateBuf[12];

	WriteMessage(LOG_INFO, BuildString("Log Session Opened at %s on %s", _strtime(szTimeBuf), _strdate(szDateBuf)));
#endif
}

VOID CBofLog::GetLogFile(CHAR *pszFileName) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);

	Common::strcpy_s(pszFileName, MAX_FNAME, m_szFileName);
}

INT CBofLog::GetTypeIndex(ULONG nLogType) {
	Assert(IsValidObject(this));
	Assert((nLogType == LOG_INFO) || (nLogType == LOG_WARN) || (nLogType == LOG_ERROR) || (nLogType == LOG_FATAL));

	INT i;

	nLogType >>= 16;

	i = 0;
	while (nLogType > 1) {
		nLogType >>= 1;
		i++;
	}

	return i;
}

VOID CBofLog::WriteMessage(ULONG nLogType, const CHAR *pszMessage, USHORT /*nUserFilter*/, const CHAR *pszSourceFile, INT nLine) {
	static BOOL bAlready = FALSE;

	if (!bAlready) {

		// stop recursion
		bAlready = TRUE;

		Assert(IsValidObject(this));

		// validate input
		Assert((nLogType == LOG_INFO) || (nLogType == LOG_WARN) || (nLogType == LOG_ERROR) || (nLogType == LOG_FATAL));
		Assert(pszMessage != nullptr);

		Common::String buf, extraBuf;

		if ((m_lOptions & nLogType) /*&& (m_lOptions & nUserFilter)*/) {
			buf = Common::String::format("%s%s", g_pszLogTypes[GetTypeIndex(nLogType)], pszMessage);

			// include source file and line # info?
			//
			if ((m_lOptions & LOG_SOURCE) && (pszSourceFile != nullptr)) {
				extraBuf = Common::String::format(" in %s(%d)", pszSourceFile, nLine);
				buf += extraBuf;
			}

			// make sure we don't overrun this pointer
			Assert(buf.size() < MAX_LOG_LINE_LEN);

			// if writing to an output file
			if ((m_lOptions & LOG_FILE) && (m_szFileName[0] != '\0')) {
#if 0
				FILE *pFile;

				if ((pFile = fopen(m_szFileName, "at")) != nullptr) {
					fprintf(pFile, "\n%s", szBuf);
					fclose(pFile);
				}
#endif
			}

			// if writing to the debug output window
			if (m_lOptions & LOG_WINDOW) {
				debug("%s", buf.c_str());
			}
		}

		bAlready = FALSE;
	}
}

const CHAR *BuildString(const CHAR *pszFormat, ...) {
	static CHAR szBuf[MAX_LOG_LINE_LEN];
	va_list argptr;

	Assert(pszFormat != nullptr);

	if (pszFormat != nullptr) {
		// Parse the variable arguement list
		va_start(argptr, pszFormat);
		Common::vsprintf_s(szBuf, pszFormat, argptr);
		va_end(argptr);

		// make sure we don't blow the stack
		Assert(strlen(szBuf) < MAX_LOG_LINE_LEN);

		return (const CHAR *)&szBuf[0];
	}

	return nullptr;
}


} // namespace Bagel
