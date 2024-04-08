
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

#include "common/util.h"

#include "bagel/boflib/object.h"
#include "bagel/boflib/string.h"

namespace Bagel {

class bof_ifstream : public CBofObject {
private:
	int m_nLineNumber;
	CBofString m_sLineString;

	char *m_pszBuf;
	char *m_pszCurr;
	char *m_pszEOF;
	int m_nLength;

public:
	bof_ifstream(char *pBuf, int nLength) {
		m_pszCurr = m_pszBuf = pBuf;
		m_pszEOF = pBuf + nLength;
		m_nLength = nLength;

		m_nLineNumber = 1;
		m_sLineString = "";
	}

	int Get() {
		int ch = get();

#if BOF_DEBUG
		m_sLineString += (char)ch;
#endif
		return ch;
	}

	int getline(char *pszBuf, int nLength) {
		return get(pszBuf, nLength, '\n');
	}

	int putback(char /*ch*/) {
		if (m_pszCurr > m_pszBuf)
			m_pszCurr--;

		return 0;
	}

	int eof() {
		if (m_pszCurr == m_pszEOF) {
			return -1;

		} else {
			return 0;
		}
	}

	int get() {
		if (m_pszCurr != m_pszEOF)
			return *m_pszCurr++;
		else
			return -1;
	}

	int get(char *pszBuf, int nCount, char chDelim = '\n') {
		if (!eof()) {
			char *p;
			nCount = MIN<int>(nCount, m_pszEOF - m_pszCurr);

			if ((p = (char *)memchr(m_pszCurr, chDelim, nCount)) != nullptr) {
				nCount = MIN<int>(nCount, p - m_pszCurr /* + 1*/);
			}
			memcpy(pszBuf, m_pszCurr, nCount);
			m_pszCurr += nCount;
			Assert(m_pszCurr <= m_pszEOF);

			return m_pszCurr == m_pszEOF ? -1 : 0;

		} else {
			return -1;
		}
	}

	int peek() {
		if (m_pszCurr != m_pszEOF)
			return *m_pszCurr;
		else
			return -1;
	}

	int Get(char *pch, int nCount, char delim = '\n') {
		get(pch, nCount, delim);

#if BOF_DEBUG
		if (delim == '\n') {
			m_nLineNumber++;
			m_sLineString = "";
		}
#endif
		return 0;
	}

	int EatWhite() {
		int ch;
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

	int GetLineNumber() const {
		return m_nLineNumber;
	}
	int GetSize() const {
		return m_nLength;
	}
	const CBofString &GetLineString() {
		return m_sLineString;
	}
};


} // namespace Bagel

#endif
