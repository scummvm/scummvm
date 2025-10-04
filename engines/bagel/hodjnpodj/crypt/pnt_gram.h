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

#ifndef HODJNPODJ_CRYPT_PNT_GRAM_H
#define HODJNPODJ_CRYPT_PNT_GRAM_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/crypt/globals.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

#define A                       65  // ascii value of 'A'
#define Z                       90  // ascii value of 'Z'
#define ALPHABET_BASE           0   // first index to char 'A' of bitmap array
#define ASCII_OFFSET            65  // ascii value of 'A'

//#define ALPHA_RESOURCE_ID     100 // corresp to crypt.rc alphabet id's
//#define USED_RESOURCE_ID      200 // corresp to crypt.rc alphabet id's
#define ALPHA_ROW               0
#define USED_ROW                1
#define REVEAL_ROW              2
#define ALPHA_RESOURCE_ID       100 // corresp to crypt.rc celtest.bmp id
#define HILITE_RESOURCE_ID      300 // corresp to crypt.rc alphabet id's

#define STANDARD_SPACE_LENGTH   10  // lenth of ' ' space char
#define STANDARD_CHAR_WIDTH     11  // width of larges char art bitmap
#define STANDARD_CHAR_HEIGHT    9   // height of typical char art bitmap
#define PUNCTUATION             9   // Number of punctuation symbols of non-standard width

#define ALPHA_TOP_COL           185 // alphabet pos coord
#define ALPHA_TOP_ROW           225
#define ALPHA_BOT_COL           472
#define ALPHA_BOT_ROW           237

#define GRAM_TOP_COL            143 // cryptogram pos coord
#define GRAM_TOP_ROW            253
#define GRAM_BOT_COL            505
#define GRAM_BOT_ROW            462

#define GRAM_LINE_SPACING       2   // space betw each line
#define GRAM_LETTER_SPACING     0   // betw each letter
#define GRAM_LEFT_MARGIN        15  // pixels indention
#define GRAM_RIGHT_MARGIN       0

#define CHAR_TYPE               0x00FF  // selects character
#define GRAM_TYPE               0x0000  // part of cryptogram screen section
#define ALPHA_TYPE              0x0100  // alphabet section
#define HILITE_TYPE             0x0200  // high-light section
#define USED_TYPE               0x0400  // indicates alphabet letter was used
#define LIMBO_TYPE              0x0800  // indicates char is in purgatory
#define REVEAL_TYPE             0x1000  // indicates an originally revealed letter

class CCryptRecord;

class CPaintGram {
private:    // functions
	inline  int  SetHiLiteTypeOn(int nTypeCode) {
		return nTypeCode | HILITE_TYPE;
	}
	inline  int  SetHiLiteTypeOff(int nTypeCode) {
		return nTypeCode  & ~HILITE_TYPE;
	}
	inline  int  SetAlphaType(int nTypeCode) {
		return nTypeCode | ALPHA_TYPE;
	}
	inline  int  SetGramType(int nTypeCode) {
		return nTypeCode | GRAM_TYPE;
	}
	inline  int  SetUsedTypeOn(int nTypeCode) {
		return nTypeCode | USED_TYPE;
	}
	inline  int  SetUsedTypeOff(int nTypeCode) {
		return nTypeCode &  ~USED_TYPE;
	}
	inline  int  SetRevealTypeOn(int nTypeCode) {
		return nTypeCode | REVEAL_TYPE;
	}
	inline  int  SetRevealTypeOff(int nTypeCode) {
		return nTypeCode &  ~REVEAL_TYPE;
	}

private:    // vars
	CSprite *m_cAlphabet[SYMBOLS] = {};          // Crypt construct objects
	CSprite *m_cUsedAlphabet[USED_SYMBOLS] = {};
	CSprite *m_cRevealAlphabet[REVEAL_SYMBOLS] = {};
	CSprite *m_cHiLite = nullptr;

	CSize   m_cPos;

	CRect           m_cGramRegion;          // Cryptogram formatting params
	CRect           m_cAlphaRegion;
	const int       m_nGramLineSpacing;
	const int       m_nGramLetterSpacing;
	const int       m_nGramLeftMargin;
	const int       m_nGramRightMargin;
	const int       m_nStandardSpaceLength;
	const int       m_nStandardCharHeight;
	const int       m_nStandardCharWidth;

public:
	CPaintGram(CDC *pDC);
	~CPaintGram();

	void    InitGramPosition(CCryptRecord *pRec);
	int     CenterGramVert(CCryptRecord *pRec);
	void    PaintAlphabet(CDC *pDC);
	void    PaintGram(CDC *pDC, const char *lpszCryptedGram);
	void    PaintLine(CDC *pDC, const char *lpszCryptedGram, CSize cPos, int i);
	void    ReplaceLetter(CDC *pDC, int nOldType, int nNewType);
	bool    IsHiLiteOn();
	int     GetHiLiteType(CDC *pDC);
	void    HiLiteOff(CDC *pDC);
	void    HiLiteOn(CDC *pDC, int nTypeCode);
	void    UsedOff(CDC *pDC, int nTypeCode);
	void    UsedOn(CDC *pDC, int nTypeCode);
	void    RevealOn(CDC *pDC, int nTypeCode);
	void    ClearGram(CDC *pDC);

	bool IsHiLiteType(int nTypeCode);
	bool IsAlphabetType(int nTypeCode);
	bool IsGramType(int nTypeCode);
	bool IsUsedType(int nTypeCode);
	bool IsRevealType(int nTypeCode);
	bool IsAlphaChar(char chChar);
	bool IsSymbolChar(char chChar);


	char IndexToSymb(int nIndex);
	int  SymbToIndex(char chChar);
	char IndexToChar(int nIndex);
	int  CharToIndex(char chChar);

	inline  char GetCharType(int nTypeCode) {
		return (char)(nTypeCode & CHAR_TYPE);
	}
	inline  int  SetLimboTypeOn(int nTypeCode) {
		return nTypeCode | LIMBO_TYPE;
	}
	inline  int  SetLimboTypeOff(int nTypeCode) {
		return nTypeCode &  ~LIMBO_TYPE;
	}

	CSprite *m_cDisplayLetters = nullptr;
};

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel

#endif
