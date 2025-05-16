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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/crypt/rec.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

CCryptRecord::CCryptRecord() {
}

CCryptRecord::~CCryptRecord() {
}


BOOL CCryptRecord::GetRecord(int nID) {
	#ifdef TODO
	OFSTRUCT    ofstFileStat;
	char        chBuf;
	int         i;

	if ((m_hfCryptFile = LZOpenFile(CRYPT_TXT_FILE, &ofstFileStat, OF_READ)) == -1)
		return FALSE;

	m_nID = 0;
	for (m_nID = 0; m_nID < nID; m_nID++) {              // Locate record
		LZSeek(
		    m_hfCryptFile,
		    RECORD_LEN * sizeof(char),
		    1
		);                                          // Advance past initial quote mark
	}

	LZSeek(m_hfCryptFile, sizeof(char), 1);             // Advance past initial quote mark

	for (i = 0; ; i++) {                                 // Grab cryptogram
		if (i >= MAX_GRAM_LEN)
			return FALSE;

		LZRead(m_hfCryptFile, &chBuf, sizeof(char));
		if (chBuf == '\\') {
			LZRead(m_hfCryptFile, &chBuf, sizeof(char));

			m_lpszGram[i] = chBuf;
			i++;
		} else if (chBuf == '\"') {
			m_lpszGram[i] = '\0';
			break;
		}

		m_lpszGram[i] = chBuf;
	}

	_fstrupr(m_lpszGram);
	LZSeek(m_hfCryptFile, 2 * sizeof(char), 1);         // Advance past dilimiting comma and initial quote mark

	for (i = 0; ; i++) {             // Grab source
		if (i >= MAX_SOURCE_LEN)
			return FALSE;

		LZRead(m_hfCryptFile, &chBuf, sizeof(char));
		if (chBuf == '\\') {
			LZRead(m_hfCryptFile, &chBuf, sizeof(char));
			m_lpszSource[i] = chBuf;
		} else if (chBuf == '\"') {
			m_lpszSource[i] = '\0';
			break;
		}

		m_lpszSource[i] = chBuf;
	}

	_fstrupr(m_lpszSource);
	LZClose(m_hfCryptFile);
	return TRUE;
	#else
	error("TODO: CCryptRecord::GetRecord");
	#endif
}

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel
