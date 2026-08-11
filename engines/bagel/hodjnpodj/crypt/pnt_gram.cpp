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
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/crypt/globals.h"
#include "bagel/hodjnpodj/crypt/pnt_gram.h"
#include "bagel/hodjnpodj/crypt/crypt.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

/*****************************************************************
 *
 * CPaintGram
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
CPaintGram::CPaintGram(CDC *pDC)

	:   m_nGramLineSpacing(GRAM_LINE_SPACING),      // init const members
	    m_nGramLetterSpacing(GRAM_LETTER_SPACING),
	    m_nGramLeftMargin(GRAM_LEFT_MARGIN),
	    m_nGramRightMargin(GRAM_RIGHT_MARGIN),
	    m_nStandardSpaceLength(STANDARD_SPACE_LENGTH),
	    m_nStandardCharHeight(STANDARD_CHAR_HEIGHT),
	    m_nStandardCharWidth(STANDARD_CHAR_WIDTH) {
	CBitmap *pFontSource = nullptr,
	         *pFontCel = nullptr;
	bool    bSuccess;
	int     i;
	int     nExtraWidth = 0;
	int     nFontWidth[] =  { 11, 10, 5, 9, 11, 11, 11, 11, 11,             // 9 symbols
	                          11, 11, 11, 11, 11, 11, 11, 11, 11, 11
	                        };   // + 10 numbers

	m_cAlphaRegion.SetRect(         // init members
	    ALPHA_TOP_COL,
	    ALPHA_TOP_ROW,
	    ALPHA_BOT_COL,
	    ALPHA_BOT_ROW
	);
	m_cGramRegion.SetRect(
	    GRAM_TOP_COL,
	    GRAM_TOP_ROW,
	    GRAM_BOT_COL,
	    GRAM_BOT_ROW
	);

	pFontSource = FetchResourceBitmap(pDC, nullptr, ALPHA_RESOURCE_ID);         // get the font bitmap

	for (i = 0; i < SYMBOLS ; i++) {         // Initialize master alphabet list
		m_cAlphabet[i] = new CSprite();     // Initize the individual letter of the alphabet list
		(*m_cAlphabet[i]).SharePalette(pGamePalette);
		if ((i < ALPHABET)) {
			pFontCel = ExtractBitmap(pDC, pFontSource, pGamePalette,
			                         i * STANDARD_CHAR_WIDTH, ALPHA_ROW * STANDARD_CHAR_HEIGHT,
			                         STANDARD_CHAR_WIDTH, STANDARD_CHAR_HEIGHT);
		} else {                                                // it's a punctuation mark or number
			pFontCel = ExtractBitmap(pDC, pFontSource, pGamePalette,
			                         (ALPHABET * STANDARD_CHAR_WIDTH) + nExtraWidth,
			                         ALPHA_ROW * STANDARD_CHAR_HEIGHT,
			                         nFontWidth[i - ALPHABET], STANDARD_CHAR_HEIGHT);
			nExtraWidth += nFontWidth[i - ALPHABET];
		}
		bSuccess = (*m_cAlphabet[i]).LoadSprite(pFontCel, pGamePalette);
		ASSERT(bSuccess);

		(*m_cAlphabet[i]).SetMobile(true);
		(*m_cAlphabet[i]).SetMasked(true);
		(*m_cAlphabet[i]).SetTypeCode(IndexToChar(i));  // set user defined Type Code to corres to the letter
	}  // end for

	for (i = 0; i < USED_SYMBOLS ; i++) {        // Initialize master alphabet-used list
		m_cUsedAlphabet[i] = new CSprite();     // Initialize High-lighted chars
		(*m_cUsedAlphabet[i]).SharePalette(pGamePalette);
		pFontCel = ExtractBitmap(pDC, pFontSource, pGamePalette,
		                         i * STANDARD_CHAR_WIDTH, USED_ROW * STANDARD_CHAR_HEIGHT,
		                         STANDARD_CHAR_WIDTH, STANDARD_CHAR_HEIGHT);
		bSuccess = (*m_cUsedAlphabet[i]).LoadSprite(pFontCel, pGamePalette);
		ASSERT(bSuccess);

		(*m_cUsedAlphabet[i]).SetMobile(true);
		(*m_cUsedAlphabet[i]).SetMasked(true);
		(*m_cUsedAlphabet[i]).SetTypeCode(SetAlphaType(IndexToChar(i)));    // set user defined Type Code to corres to the letter
	}  // end for

	for (i = 0; i < REVEAL_SYMBOLS ; i++) {          // Initialize master alphabet-used list
		m_cRevealAlphabet[i] = new CSprite();       // Initialize High-lighted chars
		(*m_cRevealAlphabet[i]).SharePalette(pGamePalette);
		pFontCel = ExtractBitmap(pDC, pFontSource, pGamePalette,
		                         i * STANDARD_CHAR_WIDTH, REVEAL_ROW * STANDARD_CHAR_HEIGHT,
		                         STANDARD_CHAR_WIDTH, STANDARD_CHAR_HEIGHT);
		bSuccess = (*m_cRevealAlphabet[i]).LoadSprite(pFontCel, pGamePalette);
		ASSERT(bSuccess);

		(*m_cRevealAlphabet[i]).SetMobile(true);
		(*m_cRevealAlphabet[i]).SetMasked(true);
		(*m_cRevealAlphabet[i]).SetTypeCode(SetAlphaType(IndexToChar(i)));  // set user defined Type Code to corres to the letter
	}  // end for

	// Delete temporary font bitmaps
	if (pFontSource != nullptr) {
		pFontSource->DeleteObject();
		delete pFontSource;
		pFontSource = nullptr;
	}

	m_cHiLite = new CSprite();              // HiLite box area
	bSuccess = (*m_cHiLite).LoadResourceSprite(pDC, HILITE_RESOURCE_ID);
	ASSERT(bSuccess);

	(*m_cHiLite).SetMobile(true);
	(*m_cHiLite).SetMasked(true);
	(*m_cHiLite).SetTypeCode(SetHiLiteTypeOn(false));

}

/*****************************************************************
 *
 * ~CPaintGram
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
CPaintGram::~CPaintGram() {
	int i;

	CSprite::FlushSpriteChain();
	if (m_cHiLite != nullptr)
		delete m_cHiLite;
	for (i = 0; i < USED_SYMBOLS; i++)
		delete m_cRevealAlphabet[i];    // each letter in the alpha
	for (i = 0; i < REVEAL_SYMBOLS; i++)
		delete m_cUsedAlphabet[i];      // each letter in the alpha
	for (i = 0; i < SYMBOLS; i++)
		delete m_cAlphabet[i];          // each letter in the alpha
}

/*****************************************************************
 *
 * IsHiLiteType
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsHiLiteType(int nTypeCode) {
	if ((nTypeCode & HILITE_TYPE) != false) {    // True is anything > 0
		return true;
	} else {
		return false;
	}
}

/*****************************************************************
 *
 * IsAlphabetType
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsAlphabetType(int nTypeCode) {
	if ((nTypeCode & ALPHA_TYPE) != false) {     // True is anything > 0
		return true;
	} else {
		return false;
	}
}

/*****************************************************************
 *
 * IsGramType
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsGramType(int nTypeCode) {
	if (IsAlphabetType(nTypeCode) == false) {    // True is anything > 0
		return true;
	} else {
		return false;
	}
}

/*****************************************************************
 *
 * IsUsedType
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsUsedType(int nTypeCode) {
	if ((nTypeCode & USED_TYPE) != false) {      // True is anything > 0
		return true;
	} else {
		return false;
	}
}

bool CPaintGram::IsRevealType(int nTypeCode) {
	if ((nTypeCode & REVEAL_TYPE) != false) {    // True is anything > 0
		return true;
	} else {
		return false;
	}
}

/*****************************************************************
 *
 * IsAlphaChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsAlphaChar(char chChar) {
	if (chChar >= A && chChar <= Z) {
		return true;
	} else {
		return false;
	}
}

/*****************************************************************
 *
 * IsSymbolChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsSymbolChar(char chChar) {
	return SymbToIndex(chChar) != false;
}

/*****************************************************************
 *
 * IndexToSymp
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
char CPaintGram::IndexToSymb(int nIndex) {
	switch (nIndex) {

	case 26:
		return '.';
	case 27:
		return ',';
	case 28:
		return '\'';
	case 29:
		return '\"';
	case 30:
		return '-';
	case 31:
		return '!';
	case 32:
		return '?';
	case 33:
		return ':';
	case 34:
		return ';';
	case 35:
		return '0';
	case 36:
		return '1';
	case 37:
		return '2';
	case 38:
		return '3';
	case 39:
		return '4';
	case 40:
		return '5';
	case 41:
		return '6';
	case 42:
		return '7';
	case 43:
		return '8';
	case 44:
		return '9';
	default:
		return false;
	}
}

/*****************************************************************
 *
 * SymbToIndex
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
int CPaintGram::SymbToIndex(char chChar) {
	switch (chChar) {

	case '.':
		return 26;
	case ',':
		return 27;
	case '\'':
		return 28;
	case '\"':
		return 29;
	case '-':
		return 30;
	case '!':
		return 31;
	case '?':
		return 32;
	case ':':
		return 33;
	case ';':
		return 34;
	case '0':
		return 35;
	case '1':
		return 36;
	case '2':
		return 37;
	case '3':
		return 38;
	case '4':
		return 39;
	case '5':
		return 40;
	case '6':
		return 41;
	case '7':
		return 42;
	case '8':
		return 43;
	case '9':
		return 44;
	default:
		return false;
	}
}

/*****************************************************************
 *
 * IndexToChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
char CPaintGram::IndexToChar(int nIndex) {
	char chSym = IndexToSymb(nIndex);

	if (chSym != false) {
		return (chSym);
	} else {
		return (char)nIndex + ASCII_OFFSET;
	}
}

/*****************************************************************
 *
 * CharToIndex
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
int CPaintGram::CharToIndex(char chChar) {
	int nSym = SymbToIndex(chChar);

	if (nSym != false) {
		return nSym;
	} else {
		return (int)toupper(chChar) - ASCII_OFFSET;
	}
}

/*****************************************************************
 *
 * PaintAlphabet
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::PaintAlphabet(CDC *pDC) {
	int     i;
	CSize   cLetterDim;
	CSize   cLetterPos(0, 0);   // letter col

	for (i = 0; i < ALPHABET ; i++) {    // Initialize the displayed alphabet
		//  list on bottom of screen
		m_cDisplayLetters = (*m_cAlphabet[i]).DuplicateSprite(pDC);
		(*m_cDisplayLetters).SetTypeCode(SetAlphaType(IndexToChar(i)));
		cLetterDim = (*m_cDisplayLetters).GetSize();
		(*m_cDisplayLetters).LinkSprite();
		(*m_cDisplayLetters).PaintSprite(
		    pDC,
		    m_cAlphaRegion.TopLeft().x + cLetterPos.cx, // Advances col pos
		    (m_cAlphaRegion.TopLeft()).y +              // Advances row pos
		    ((m_cAlphaRegion.Height() - cLetterDim.cy) >> 1));

		cLetterPos.cx += cLetterDim.cx +            // advance to new starting
		                 m_nGramLetterSpacing;   // row for the next letter
	} // end for
}

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::PaintGram(CDC *pDC, const char *lpszCryptedGram) {
	int     i;
	int     nSpaceMarker;
	int     nStartPos;
	int     nIndexChar;
	int     nLineLength = m_nGramLeftMargin;

	/****************************************************************
	* Counts the words until its length exceeds line boundaries     *
	* set as length of left and right margin plus line width.       *
	* Once it finds a complete line within this boundary, it will   *
	* call 'PaintLine' member to draw the visual line.              *
	****************************************************************/
	for (i = 0, nSpaceMarker = 0, nStartPos = 0; true; i++) {
		if (lpszCryptedGram[i] == 0) {
			if ((nLineLength + m_nGramRightMargin) >= m_cGramRegion.Width()) {
				nSpaceMarker++;                     // Advance one past space
				PaintLine(pDC, lpszCryptedGram + nStartPos, m_cPos, nSpaceMarker - nStartPos);

				/************************
				* Reset line positions. *
				************************/
				nStartPos = nSpaceMarker;
				nLineLength = m_nGramLeftMargin;
				m_cPos.cx  = m_nGramLeftMargin + (m_cGramRegion.TopLeft()).x;
				m_cPos.cy += m_nGramLineSpacing + m_nStandardCharHeight;
			} // end if
			break;
		} // end if

		if (lpszCryptedGram[i] == ' ') {
			nLineLength += m_nStandardSpaceLength;

			if (lpszCryptedGram[i + 1] == ' ')   // Space could follow space
				continue;

			if ((nLineLength + m_nGramRightMargin) >= m_cGramRegion.Width()) {
				nSpaceMarker++;                     // Advance one past space
				PaintLine(pDC, lpszCryptedGram + nStartPos, m_cPos, nSpaceMarker - nStartPos);

				/************************
				* Reset line positions. *
				************************/
				nStartPos = nSpaceMarker;           // Must restart position from current spot
				i = nSpaceMarker;                   // ...and reset index to recalc char widths
				nLineLength = m_nGramLeftMargin;
				m_cPos.cx  = m_nGramLeftMargin + (m_cGramRegion.TopLeft()).x;
				m_cPos.cy += m_nGramLineSpacing + m_nStandardCharHeight;
				continue;
			} // end if
			nSpaceMarker = i;                       // Advance space marker to new pos
			continue;
		} // end if

		if (IsAlphaChar(lpszCryptedGram[i])) {   // 'A' thru 'Z'
			nLineLength += m_nStandardCharWidth + m_nGramLetterSpacing;
			continue;
			//((*m_cAlphabet[nChar]).GetSize()).cx + m_nGramLetterSpacing;
		} // end if

		if ((nIndexChar = SymbToIndex(lpszCryptedGram[i])) != false) {   // '.' thru ':'
			nLineLength += ((*m_cAlphabet[nIndexChar]).GetSize()).cx + m_nGramLetterSpacing;
		} // end if
	} // end for

	PaintLine(pDC, lpszCryptedGram + nStartPos, m_cPos, i - nStartPos);

	/*******************************************************
	* Advance to beginning column, and add two extra rows. *
	*******************************************************/
	m_cPos.cx = m_nGramLeftMargin + (m_cGramRegion.TopLeft()).x;
	m_cPos.cy += 2 * (m_nGramLineSpacing + m_nStandardCharHeight);
}

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::PaintLine(CDC *pDC, const char *lpszCryptedGram, CSize cPos, int i) {
	int     j;
	int     nChar;
	CSize   cLetterDim;

	for (j = 0; j < i; j++) {
		if (lpszCryptedGram[j] == 0) {
			break;
		}

		if (lpszCryptedGram[j] == ' ') {                     // space char?
			cPos.cx += m_nStandardSpaceLength;
			continue;
		} // end if


		if (IsAlphaChar(lpszCryptedGram[j]) == true) {   // A - Z char?
			nChar = CharToIndex(lpszCryptedGram[j]);
			m_cDisplayLetters = (*m_cAlphabet[nChar]).DuplicateSprite(pDC);
			//(*m_cDisplayLetters).SetTypeCode(SetGramType(IndexToAlpha(i)));
			//cLetterDim = (*m_cDisplayLetters).GetSize();
			(*m_cDisplayLetters).LinkSprite();
			(*m_cDisplayLetters).PaintSprite(pDC, cPos.cx, cPos.cy);
			cPos.cx += m_nStandardCharWidth + m_nGramLetterSpacing;
			continue;
			//cLetterDim.cx + m_nGramLetterSpacing;
		} // end if

		if (IsSymbolChar(lpszCryptedGram[j]) == true) {      // '.' thru ':'
			nChar = CharToIndex(lpszCryptedGram[j]);
			m_cDisplayLetters = (*m_cAlphabet[nChar]).DuplicateSprite(pDC);
			(*m_cDisplayLetters).LinkSprite();
			(*m_cDisplayLetters).PaintSprite(pDC, cPos.cx, cPos.cy);

			cLetterDim = (*m_cDisplayLetters).GetSize();
			cPos.cx += cLetterDim.cx + m_nGramLetterSpacing;
		} // end if
	}
}

void CPaintGram::InitGramPosition(CCryptRecord *pRec) {
	/************************************
	* initial starting letter position. *
	************************************/
	m_cPos.cx = m_nGramLeftMargin + (m_cGramRegion.TopLeft()).x;
	m_cPos.cy = (m_cGramRegion.TopLeft()).y + CenterGramVert(pRec);
}


int CPaintGram::CenterGramVert(CCryptRecord *pRec) {
	const char    *lpszGram;
	const char    *lpszSource;
	int     nSpaceMarker;
	int     nLineLength;
	int     nEstimate;
	int     i;

	lpszGram        = (*pRec).GetGram();
	lpszSource      = (*pRec).GetSource();
	nEstimate       = 0;

	/****************************************************************
	* Counts the words until its length exceeds line boundaries     *
	* set as length of left and right margin plus line width.       *
	* Once it finds a complete line within this boundary, it        *
	* increments nEstimate.                                         *
	****************************************************************/
	for (
	    i = 0,
	    nSpaceMarker = 0,
	    nLineLength = m_nGramLeftMargin; true ; i++
	) {             // Estimate for Gram
		if (lpszGram[i] == 0) {
			if ((nLineLength + m_nGramRightMargin) >= m_cGramRegion.Width()) {
				nEstimate++;
			} // end if
			nEstimate++;
			break;
		} // end if

		if (lpszGram[i] == ' ') {
			nLineLength += m_nStandardSpaceLength;

			if (lpszGram[i + 1] == ' ')          // Space could follow space
				continue;

			if ((nLineLength + m_nGramRightMargin) >= m_cGramRegion.Width()) {
				nEstimate++;

				nSpaceMarker++;                 // Advance one past space
				i = nSpaceMarker;               // reset index to recalc char widths
				nLineLength = m_nGramLeftMargin;
				continue;
			} // end if
			nSpaceMarker = i;                   // Advance space marker to new pos
			continue;
		} // end if

		if (IsAlphaChar(lpszGram[i])) {
			nLineLength += m_nStandardCharWidth + m_nGramLetterSpacing;
			continue;
			//((*m_cAlphabet[nChar]).GetSize()).cx + m_nGramLetterSpacing;
		} // end if

		if (IsSymbolChar(lpszGram[i])) {
			nLineLength += (*m_cAlphabet[SymbToIndex(lpszGram[i])]).GetSize().cx + m_nGramLetterSpacing;
			//((*m_cAlphabet[nChar]).GetSize()).cx + m_nGramLetterSpacing;
		} // end if
	} // end for

	nEstimate += 2;                             // Want two spaces between cryptogram and source

	for (
	    i = 0,
	    nSpaceMarker = 0,
	    nLineLength = m_nGramLeftMargin; true ; i++
	) {             // Estimate for Gram
		if (lpszSource[i] == 0) {
			if ((nLineLength + m_nGramRightMargin) >= m_cGramRegion.Width()) {
				nEstimate++;
			} // end if
			nEstimate++;
			break;
		} // end if

		if (lpszSource[i] == ' ') {
			nLineLength += m_nStandardSpaceLength;

			if (lpszSource[i + 1] == ' ')        // Space could follow space
				continue;

			if ((nLineLength + m_nGramRightMargin) >= m_cGramRegion.Width()) {
				nEstimate++;

				nSpaceMarker++;                 // Advance one past space
				i = nSpaceMarker;               // reset index to recalc char widths
				nLineLength = m_nGramLeftMargin;
				continue;
			} // end if
			nSpaceMarker = i;                   // Advance space marker to new pos
			continue;
		} // end if

		if (IsAlphaChar(lpszSource[i])) {
			nLineLength += m_nStandardCharWidth + m_nGramLetterSpacing;
			continue;
			//((*m_cAlphabet[nChar]).GetSize()).cx + m_nGramLetterSpacing;
		} // end if

		if (IsSymbolChar(lpszSource[i])) {
			nLineLength += (*m_cAlphabet[SymbToIndex(lpszSource[i])]).GetSize().cx + m_nGramLetterSpacing;
			//((*m_cAlphabet[nChar]).GetSize()).cx + m_nGramLetterSpacing;
		} // end if
	} // end for

	nEstimate *= m_nGramLineSpacing + m_nStandardCharHeight;

	return (int)(m_cGramRegion.Height() - nEstimate) >> 1;
}

/*****************************************************************
 *
 *  IsHiLiteOn
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Looks for a HiLite
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CPaintGram::IsHiLiteOn() {
	m_cDisplayLetters = CSprite::GetSpriteChain();
	while (m_cDisplayLetters != nullptr) {
		if (IsHiLiteType((*m_cDisplayLetters).GetTypeCode()) == true) {
			return true;
		}
		m_cDisplayLetters = (*m_cDisplayLetters).GetNextSprite();
	}

	return false;
}

/*****************************************************************
 *
 * GetHiLiteType
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Assumes something is highlighted.
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
int CPaintGram::GetHiLiteType(CDC *pDC) {
	CRect   crectSprite;
	int     nTypeCode;

	m_cDisplayLetters = CSprite::GetSpriteChain();
	while (m_cDisplayLetters != nullptr) {
		if (IsHiLiteType((*m_cDisplayLetters).GetTypeCode()) == true) {
			crectSprite = (*m_cDisplayLetters).GetRect();
			m_cDisplayLetters = (*m_cDisplayLetters).Interception(&crectSprite);
			nTypeCode = (*m_cDisplayLetters).GetTypeCode();

			if (IsHiLiteType(nTypeCode) == true) {
				return (*m_cDisplayLetters).Interception(&crectSprite, m_cDisplayLetters)->GetTypeCode();
			} else {
				return nTypeCode;
			}
		}
		m_cDisplayLetters = (*m_cDisplayLetters).GetNextSprite();
	}

	return false;
}

/*****************************************************************
 *
 * HiLiteOff
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Turns highlighted visual characters off, if any.  Uses
 *  knowledge of sprite's user defined TypeCode to distinguish
 *  between Hilite, Alphabetic region, Cryptogram region, and
 *  character letter type.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - handle to device context window
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  None
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  m_cDisplayLetters - replaces hilited sprite bitmaps with
 *      normal sprite bitmaps.
 *
 * RETURN VALUE:
 *
 *  None
 *
 ****************************************************************/
void CPaintGram::HiLiteOff(CDC *pDC) {
	CSprite *pSprite;       // for faster sprite search
	CPoint  cpointSprite;   // hilited sprite position
	int     nTypeCode;      // holds char info

	m_cDisplayLetters = CSprite::GetSpriteChain();
	while (m_cDisplayLetters != nullptr) {
		nTypeCode = (*m_cDisplayLetters).GetTypeCode();         // Get sprite's Hilite Code

		if (IsHiLiteType(nTypeCode) == true) {               // Is this sprite hi-lited?
			pSprite = (*m_cDisplayLetters).GetNextSprite();     // Yes - save current pos in sprite chain
			(*m_cDisplayLetters).RefreshBackground(pDC);        // ...repaint background
			(*m_cDisplayLetters).UnlinkSprite();                // ...unlink it from chain
			delete m_cDisplayLetters;

			m_cDisplayLetters = pSprite;                        // faster than GetNextSprite call
		} else {
			m_cDisplayLetters = (*m_cDisplayLetters).GetNextSprite();
		}
	}
}

/*****************************************************************
 *
 * HiLiteOn
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Hilites visual letter specified by nTypeCode.
 *  If visual letter is inside the Cryptogram region,
 *  all matching letters are hilited.
 *  If visual letter is inside the Alphabet region, only one char
 *  is hilited.
 *  Handles case of nTypeCode when it is not a letter.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - handle to device context window
 *  nTypeCode - contains knowledge of sprite's current hilite state,
 *      alphabetic region, cryptogram region, and letter type.
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  None
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::HiLiteOn(CDC *pDC, int nTypeCode) {
	CSprite *pSprite;       // for faster sprite search
	CPoint  cpointSprite;   // hilited sprite position

	m_cDisplayLetters = CSprite::GetSpriteChain();  // Get entire sprite chain
	while (m_cDisplayLetters != nullptr) {              // Hilite all matching chars
		if (nTypeCode == (*m_cDisplayLetters).GetTypeCode()) {   // Is this a char to hilite?
			cpointSprite = (*m_cDisplayLetters).GetPosition();      // Yes - get old sprite's position
			pSprite = (*m_cDisplayLetters).GetNextSprite();         // save current pos in sprite chain

			m_cDisplayLetters = (*m_cHiLite).DuplicateSprite(pDC);  // setup hilited sprite
			(*m_cDisplayLetters).LinkSprite();
			(*m_cDisplayLetters).PaintSprite(pDC, cpointSprite);

			if (IsAlphabetType(nTypeCode) == true)               // Hilited char in alpha region?
				break;                                              // Yes - break out

			m_cDisplayLetters = pSprite;                            // faster than GetNextSprite call
		} else {
			m_cDisplayLetters = (*m_cDisplayLetters).GetNextSprite();
		} // end if
	} // end while
}

/*****************************************************************
 *
 * ReplaceLetter
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Thumbs thru sprites looking for any characters matching
 *  nOldType, and replaces it with nNewType.  These char's
 *  could be in either alpha region or gram region.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - pointer to window device that gets update visually
 *  nOldType - Type of letter that will be replaced
 *  nNewType - Type of letter that will replace
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  None
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  m_cDisplayLetters - modified using nNewType for every match
 *      of nOldType.
 *
 * RETURN VALUE:
 *
 *  None
 *
 ****************************************************************/
void CPaintGram::ReplaceLetter(CDC *pDC, int nOldType, int nNewType) {
	int     nTypeCode;
	int     nIndexCode;
	CPoint  cpointSprite;
	CSprite *pSprite;

	m_cDisplayLetters = CSprite::GetSpriteChain();

	while (m_cDisplayLetters != nullptr) {
		nTypeCode = (*m_cDisplayLetters).GetTypeCode();
		if (nOldType == nTypeCode) {                                 // Is this a char to change?

			cpointSprite = (*m_cDisplayLetters).GetPosition();      // Yes - get old sprite's position
			pSprite = (*m_cDisplayLetters).GetNextSprite();         // save current pos in sprite chain

			(*m_cDisplayLetters).RefreshBackground(pDC);            // ...repaint background
			(*m_cDisplayLetters).UnlinkSprite();                    // ...unlink it from chain
			delete m_cDisplayLetters;

			nIndexCode = CharToIndex(nNewType);
			assert(nIndexCode < REVEAL_SYMBOLS);

			if (IsRevealType(nNewType) == true) {
				m_cDisplayLetters = (*m_cRevealAlphabet[nIndexCode]).DuplicateSprite(pDC);  // setup new sprite
			} else if (IsUsedType(nNewType) == true) {
				m_cDisplayLetters = (*m_cUsedAlphabet[nIndexCode]).DuplicateSprite(pDC);    // setup new sprite
			} else {
				m_cDisplayLetters = (*m_cAlphabet[nIndexCode]).DuplicateSprite(pDC);        // setup new sprite
				(*m_cDisplayLetters).SetTypeCode(nNewType);                                 // save type code
			}
			(*m_cDisplayLetters).LinkSprite();
			(*m_cDisplayLetters).PaintSprite(pDC, cpointSprite);

			if ((IsAlphabetType(nNewType) == true) ||
			        (IsAlphabetType(nOldType) == true))                 // Replacing letter in the Alphabet region?
				return;                                             // Yes - only on in alphabet region to replace, so quit

			m_cDisplayLetters = pSprite;                            // faster than GetNextSprite call
		} else {
			m_cDisplayLetters = (*m_cDisplayLetters).GetNextSprite();
		} // end if
	} // end while
}


/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::UsedOff(CDC *pDC, int nTypeCode) {
	ReplaceLetter(
	    pDC,
	    SetAlphaType(GetCharType(nTypeCode)),
	    SetAlphaType(SetUsedTypeOff(GetCharType(nTypeCode))));
}

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::UsedOn(CDC *pDC, int nTypeCode) {
	ReplaceLetter(
	    pDC,
	    SetUsedTypeOff(SetAlphaType(GetCharType(nTypeCode))),
	    SetUsedTypeOn(SetAlphaType(GetCharType(nTypeCode))));
}

void CPaintGram::RevealOn(CDC *pDC, int nTypeCode) {
	ReplaceLetter(
	    pDC,
	    SetRevealTypeOff(SetAlphaType(GetCharType(nTypeCode))),
	    SetRevealTypeOn(SetAlphaType(GetCharType(nTypeCode))));
}

/*****************************************************************
 *
 * ClearGram
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CPaintGram::ClearGram(CDC *pDC) {
	CSprite::EraseSprites(pDC);
	CSprite::FlushSpriteChain();        // flush any existing sprites
	m_cDisplayLetters = nullptr;
}

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel
