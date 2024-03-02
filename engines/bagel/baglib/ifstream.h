
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

#ifndef BAGEL_BAGLIB_IFSTREAM_H
#define BAGEL_BAGLIB_IFSTREAM_H

#include "bagel/boflib/fobject.h"
#include "bagel/boflib/string.h"

namespace Bagel {

class bof_ifstream : public CBofObject {
private:
	INT m_nLineNumber;
	CBofString m_sLineString;

	CHAR *m_pszBuf;
	CHAR *m_pszCurr;
	CHAR *m_pszEOF;
	INT m_nLength;

public:
	bof_ifstream(char *pBuf, INT nLength) {
		m_pszCurr = m_pszBuf = pBuf;
		m_pszEOF = pBuf + nLength;
		m_nLength = nLength;

		m_nLineNumber = 1;
		m_sLineString = "";
	}

	INT Get() {
		INT ch = get();

#if BOF_DEBUG
		m_sLineString += (CHAR)ch;
#endif
		return ch;
	}

	INT getline(CHAR *pszBuf, INT nLength) {
		return (get(pszBuf, nLength, '\n'));
	}

	INT putback(CHAR /*ch*/) {
		if (m_pszCurr > m_pszBuf)
			m_pszCurr--;

		return (0);
	}

	INT eof() {
		if (m_pszCurr == m_pszEOF) {
			return (-1);

		} else {
			return (0);
		}
	}

	INT get() {
		if (m_pszCurr != m_pszEOF)
			return (*m_pszCurr++);
		else
			return (-1);
	}

	INT get(char *pszBuf, INT nCount, CHAR chDelim = '\n') {
		if (!eof()) {
			CHAR *p;
			nCount = min(nCount, m_pszEOF - m_pszCurr);

			if ((p = (CHAR *)memchr(m_pszCurr, chDelim, nCount)) != nullptr) {
				nCount = min(nCount, p - m_pszCurr /* + 1*/);
			}
			memcpy(pszBuf, m_pszCurr, nCount);
			m_pszCurr += nCount;
			Assert(m_pszCurr <= m_pszEOF);

			return (m_pszCurr == m_pszEOF ? -1 : 0);

		} else {
			return (-1);
		}
	}

	INT peek() {
		if (m_pszCurr != m_pszEOF)
			return (*m_pszCurr);
		else
			return (-1);
	}

	INT Get(char *pch, int nCount, char delim = '\n') {
		get(pch, nCount, delim);

#if BOF_DEBUG
		if (delim == '\n') {
			m_nLineNumber++;
			m_sLineString = "";
		}
#endif
		return 0;
	}

	INT EatWhite() {
		INT ch;
		while (((ch = peek()) == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n')) {

			if (ch == '\n') {
				m_nLineNumber++;
				m_sLineString = "";
			}
			Get();
		}
		if (ch == -1)
			return ch;
		else
			return 0;
	}

	INT GetLineNumber() { return m_nLineNumber; }
	const CBofString &GetLineString() { return m_sLineString; }
};


} // namespace Bagel

#endif
