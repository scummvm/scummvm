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

#include "bagel/boflib/gui/text_box.h"

namespace Bagel {

#define DEFAULT_PAGE_SIZE   10
#define DEFAULT_WEIGHT      TEXT_NORMAL
#define DEFAULT_POINT_SIZE  FONT_12POINT        // jwl 1.6.97
#define DEFAULT_COLOR       CTEXT_COLOR


CBofTextBox::CBofTextBox(VOID) {
	// Inits
	//
	m_pDestWindow = NULL;
	m_pDestBitmap = NULL;
	m_pTextField = NULL;
	m_nWeight = DEFAULT_WEIGHT;
	m_nPointSize = DEFAULT_POINT_SIZE;
	m_cTextColor = DEFAULT_COLOR;
	m_nTextFont = FONT_DEFAULT;
	m_nPageSize = DEFAULT_PAGE_SIZE;
	m_nCurrentLine = 0;
	m_nCurrentIndex = 0;
	m_nNumLines = 0;
}


CBofTextBox::CBofTextBox(CBofWindow *pWindow, CBofRect *pRect, const CBofString &cText) {
	Assert(pWindow != NULL);
	Assert(pRect != NULL);

	// Inits
	//
	m_pDestWindow = NULL;
	m_pDestBitmap = NULL;
	m_pTextField = NULL;
	m_nWeight = DEFAULT_WEIGHT;
	m_nPointSize = DEFAULT_POINT_SIZE;
	m_cTextColor = DEFAULT_COLOR;
	m_nPageSize = DEFAULT_PAGE_SIZE;
	m_nCurrentLine = 0;
	m_nCurrentIndex = 0;
	m_nNumLines = 0;

	SetText(cText);
	SetBox(pRect);
	SetDisplay(pWindow);
}


CBofTextBox::CBofTextBox(CBofBitmap *pBitmap, CBofRect *pRect, const CBofString &cText) {
	Assert(pBitmap != NULL);
	Assert(pRect != NULL);

	// Inits
	//
	m_pDestWindow = NULL;
	m_pDestBitmap = NULL;
	m_pTextField = NULL;
	m_nWeight = DEFAULT_WEIGHT;
	m_nPointSize = DEFAULT_POINT_SIZE;
	m_cTextColor = DEFAULT_COLOR;
	m_nPageSize = DEFAULT_PAGE_SIZE;
	m_nCurrentLine = 0;
	m_nCurrentIndex = 0;
	m_nNumLines = 0;

	SetText(cText);
	SetBox(pRect);
	SetDisplay(pBitmap);
}


CBofTextBox::~CBofTextBox() {
	Assert(IsValidObject(this));

	if (m_pTextField != NULL) {
		delete m_pTextField;
		m_pTextField = NULL;
	}
	m_pDestWindow = NULL;
	m_pDestBitmap = NULL;
}


ERROR_CODE CBofTextBox::SetBox(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != NULL);

	// remove previous text field (if any)
	//
	if (m_pTextField != NULL) {
		delete m_pTextField;
		m_pTextField = NULL;
	}

	// create a new text field the size of the box we want
	//
	if ((m_pTextField = new CBofText(pRect, JUSTIFY_WRAP)) != NULL) {

	} else {
		ReportError(ERR_MEMORY, "Could not allocate a CBofText");
	}

	return (m_errCode);
}


VOID CBofTextBox::SetDisplay(CBofWindow *pWindow) {
	Assert(IsValidObject(this));
	Assert(pWindow != NULL);

	m_pDestWindow = pWindow;
	m_pDestBitmap = NULL;
}


VOID CBofTextBox::SetDisplay(CBofBitmap *pBitmap) {
	Assert(IsValidObject(this));
	Assert(pBitmap != NULL);

	m_pDestBitmap = pBitmap;
	m_pDestWindow = NULL;
}


VOID CBofTextBox::SetTextAttribs(const INT nSize, const INT nWeight, const RGBCOLOR cColor, const INT nFont) {
	Assert(IsValidObject(this));

	m_nPointSize = nSize;
	m_nWeight = nWeight;
	m_cTextColor = cColor;
	m_nTextFont = nFont;
}


VOID CBofTextBox::SetText(const CBofString &cString) {
	m_cBuffer = cString;
	Assert(m_cBuffer.GetLength() != 0);         // jwl 06.28.96

#if BOF_MAC || BOF_WINMAC
	m_cBuffer.ReplaceStr("\r\n", "\r");         // jwl 06.28.96 (\r\n instead of \n\r)
	m_cBuffer.ReplaceStr("\n", "\r");           // jwl 06.28.96 (\r instead of \n)
#endif

	m_nCurrentLine = 0;
	m_nCurrentIndex = 0;

	//  jwl 06.28.96 count number of carriage returns for mac
	//  scrolling

#if BOF_MAC || BOF_WINMAC
	m_nNumLines = m_cBuffer.FindNumOccurrences("\r");
#else
	m_nNumLines = m_cBuffer.FindNumOccurrences("\n");
#endif
}

INT CBofTextBox::GetIndex(const INT nLine) {
	Assert(nLine >= 0 && nLine <= m_nNumLines);

	const CHAR *pszCur, *pszBuffer, *pszLast;
	INT i, nChars;

	// Find the index into our buffer that represents the top left of the
	// buffer that is nLine from current the begining of the buffer.
	//
	pszLast = pszCur = pszBuffer = m_cBuffer;
	for (i = 0; i < nLine; i++) {
		pszLast = pszCur;
#if BOF_MAC || BOF_WINMAC
		pszCur = strstr(pszCur, "\r");      // jwl 06.28.96 look for cr's
#else
		pszCur = strstr(pszCur, "\n");
#endif
		// make sure we don't go too far (nLines is invalid)
		Assert(pszCur != NULL);

		pszCur++;
	}
	nChars = pszCur - pszBuffer;
	if (nLine == m_nNumLines) {
		nChars = pszLast - pszBuffer;
		m_nCurrentLine--;
	}

	return (nChars);
}


ERROR_CODE CBofTextBox::ScrollUp(const INT nLines) {
	INT nNewLine;

	// Make scroll a no-op if all the lines in the box appear on
	// one screen.  GJJ 11-8-96.
	//
	if (m_nNumLines <= m_nPageSize) {
		return ScrollTo(m_nCurrentLine);
	}

	nNewLine = m_nCurrentLine - nLines;

	if (nNewLine < 0) {
		nNewLine = 0;
	} else if (nNewLine > (m_nNumLines - m_nPageSize)) {
		// If the line requested to be the top of the page
		// would cause fewer than m_nPageSize lines to be displayed,
		// snap nNewLine to be the top of the last full page.
		// GJJ 11-8-96
		//
		nNewLine = (m_nNumLines - m_nPageSize);
	}
	return (ScrollTo(nNewLine));
}


ERROR_CODE CBofTextBox::ScrollTo(const INT nLine) {
	Assert(IsValidObject(this));
	Assert(nLine >= 0 && nLine <= m_nNumLines);

	m_nCurrentIndex = GetIndex(nLine);
	m_nCurrentLine = nLine;

	// Show the text box
	Display();

	return (m_errCode);
}


ERROR_CODE CBofTextBox::Display(VOID) {
	Assert(IsValidObject(this));
	Assert(m_nCurrentLine >= 0 && m_nCurrentLine <= m_nNumLines);
	Assert(m_nCurrentIndex >= 0 && m_nCurrentIndex < m_cBuffer.GetLength());

	// The actual text box must have been created before it can be displayed
	Assert(m_pTextField != NULL);

	// if painting to a window
	//
	if (m_pDestWindow != NULL) {

		m_pTextField->Display(m_pDestWindow, m_cBuffer.Mid(m_nCurrentIndex), m_nPointSize, m_nWeight, m_cTextColor, m_nTextFont);

		// otherwise, must be painting to a bitmap
		//
	} else {
		Assert(m_pDestBitmap != NULL);

		m_pTextField->Display(m_pDestBitmap, m_cBuffer.Mid(m_nCurrentIndex), m_nPointSize, m_nWeight, m_cTextColor, m_nTextFont);
	}

	return (m_errCode);
}


ERROR_CODE CBofTextBox::Erase(VOID) {
	Assert(IsValidObject(this));

	// The actual text box must have been created before it can be displayed
	Assert(m_pTextField != NULL);

	if (m_pDestWindow != NULL) {
		m_errCode = m_pTextField->Erase(m_pDestWindow);

	} else {
		Assert(m_pDestBitmap != NULL);
		m_errCode = m_pTextField->Erase(m_pDestBitmap);
	}

	return (m_errCode);
}


VOID CBofTextBox::FlushBackground(VOID) {
	Assert(IsValidObject(this));

	if (m_pTextField != NULL) {
		m_pTextField->FlushBackground();
	}
}

} // namespace Bagel
