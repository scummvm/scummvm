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

#ifndef HODJNPODJ_CRYPT_CRYPT_H
#define HODJNPODJ_CRYPT_CRYPT_H

#include "bagel/hodjnpodj/crypt/rec.h"
#include "bagel/hodjnpodj/crypt/pnt_gram.h"
#include "bagel/hodjnpodj/crypt/stats.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

#define SPLASHSPEC      ".\\art\\crypt.BMP" // bitmap file for the splash screen

#define NOT_USED        -1
#define MAP             2
#define DECRYPT_MAP     0
#define ENCRYPT_MAP     1

class CCryptogram {
private:
	CCryptRecord    *m_cRecordGram = nullptr;
	int             m_nCryptMap[MAP][ALPHABET];

public:
	CCryptogram(CDC *pDC);
	~CCryptogram();

	BOOL        DrawGram(CDC *pDC);
	BOOL        HandleUserUpdate(CDC *pDC, CPoint cpointClicked);
	BOOL        HandleUserUpdate(CDC *pDC, unsigned int nChar);
	void        Encrypt();
	void        CreateCryptMap(int nLettersSolved);
	int         UpdateCryptMap(int nOldType, int nNewType);
	BOOL        IsSolved();
	int         LettersSolved();
	void        MarkSolvedLetters(CDC *pDC);
	void        SolveCryptogram(CDC *pDC);
	void        DrawSource(CDC *pDC);

	char        m_chEncryptGram[MAX_GRAM_LEN];
	CPaintGram  *m_cPaintGram = nullptr;
	CStats      *m_cStats = nullptr;
	BOOL        bIsGameOver = FALSE;
};

// Globals!
extern CPalette     *pGamePalette;

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel

#endif
