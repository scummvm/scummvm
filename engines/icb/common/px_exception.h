/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_PX_EXCEPTION_H
#define ICB_PX_EXCEPTION_H

#include "engines/icb/common/px_rccommon.h" // This should be included in every file before anything else
#include "engines/icb/common/px_string.h"

namespace ICB {

class pxException {
	pxString m_reason; // A text reason
	int m_errno;       // An integer error number

#ifdef _DEBUG
	pxString m_sourceFileName; // Store for the source name
	int m_sourceFileLine;      // Store for the source line
#endif

public:
	// Different constructors for _DEBUG and release

#ifdef _DEBUG
	pxException(const char *sname, int sline, const char *reason, int reverrno = 0);
	pxException(const char *sname, int sline, int reverrno = 0, const char *reason = "");

	cstr GetSourceName() { return (m_sourceFileName); }
	int GetLine() { return (m_sourceFileLine); }
#else
	pxException(const char *reason, int reverrno = 0);
	explicit pxException(int reverrno = 0, const char *reason = "");
#endif

	cstr GetError() { return (m_reason); }

	// Function for compatibility with CException
	void Delete() { delete this; }

private:
	pxException(const pxException &) {}
	void operator=(const pxException &) {}
};

#ifdef _DEBUG

inline pxException::pxException(const char *sname, int sline, const char *reason, int reverrno) {
	m_reason = reason;
	m_errno = reverrno;

	m_sourceFileName = sname;
	m_sourceFileLine = sline;
}

inline pxException::pxException(const char *sname, int sline, int reverrno, const char *reason) {
	m_reason = reason;
	m_errno = reverrno;

	m_sourceFileName = sname;
	m_sourceFileLine = sline;
}

#else

inline pxException::pxException(const char *reason, int reverrno) {
	m_reason = reason;
	m_errno = reverrno;
}

inline pxException::pxException(int reverrno, const char *reason) {
	m_reason = reason;
	m_errno = reverrno;
}

#endif

#ifdef _DEBUG
#define ThrowRevolutionException(x) (throw new pxException(__FILE__, __LINE__, x))
#else
#define ThrowRevolutionException(x) (throw new pxException(x))
#endif

} // End of namespace ICB

#endif //#ifndef _PXLIBRARY_pxException_H
