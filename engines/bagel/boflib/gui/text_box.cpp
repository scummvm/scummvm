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
#define DEFAULT_POINT_SIZE  FONT_12POINT
#define DEFAULT_COLOR       CTEXT_COLOR


CBofTextBox::CBofTextBox() {
	// Inits
	m_pDestWindow = nullptr;
	m_pDestBitmap = nullptr;
	m_pTextField = nullptr;
	m_nWeight = DEFAULT_WEIGHT;
	m_nPointSize = DEFAULT_POINT_SIZE;
	m_cTextColor = DEFAULT_COLOR;
	m_nTextFont = FONT_DEFAULT;
	m_nPageSize = DEFAULT_PAGE_SIZE;
	m_nCurrentLine = 0;
	m_nCurrentIndex = 0;
	m_nNumLines = 0;
}


CBofTextBox::CBofTextBox(CBofWindow *pWindow, const CBofRect *pRect, const CBofString &cText) {
	Assert(pWindow != nullptr);
	Assert(pRect != nullptr);

	// Inits
	m_pDestWindow = nullptr;
	m_pDestBitmap = nullptr;
	m_pTextField = nullptr;
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


CBofTextBox::CBofTextBox(CBofBitmap *pBitmap, const CBofRect *pRect, const CBofString &cText) {
	Assert(pBitmap != nullptr);
	Assert(pRect != nullptr);

	// Inits
	m_pDestWindow = nullptr;
	m_pDestBitmap = nullptr;
	m_pTextField = nullptr;
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

	if (m_pTextField != nullptr) {
		delete m_pTextField;
		m_pTextField = nullptr;
	}
	m_pDestWindow = nullptr;
	m_pDestBitmap = nullptr;
}


ErrorCode CBofTextBox::SetBox(const CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	// Remove previous text field (if any)
	if (m_pTextField != nullptr) {
		delete m_pTextField;
		m_pTextField = nullptr;
	}

	// Create a new text field the size of the box we want
	if ((m_pTextField = new CBofText(pRect, JUSTIFY_WRAP)) != nullptr) {

	} else {
		ReportError(ERR_MEMORY, "Could not allocate a CBofText");
	}

	return m_errCode;
}


void CBofTextBox::SetDisplay(CBofWindow *pWindow) {
	Assert(IsValidObject(this));
	Assert(pWindow != nullptr);

	m_pDestWindow = pWindow;
	m_pDestBitmap = nullptr;
}


void CBofTextBox::SetDisplay(CBofBitmap *pBitmap) {
	Assert(IsValidObject(this));
	Assert(pBitmap != nullptr);

	m_pDestBitmap = pBitmap;
	m_pDestWindow = nullptr;
}


void CBofTextBox::SetTextAttribs(const int nSize, const int nWeight, const RGBCOLOR cColor, const int nFont) {
	Assert(IsValidObject(this));

	m_nPointSize = nSize;
	m_nWeight = nWeight;
	m_cTextColor = cColor;
	m_nTextFont = nFont;
}


void CBofTextBox::SetText(const CBofString &cString) {
	m_cBuffer = cString;
	Assert(m_cBuffer.GetLength() != 0);

#if BOF_MAC || BOF_WINMAC
	m_cBuffer.ReplaceStr("\r\n", "\r");         // (\r\n instead of \n\r)
	m_cBuffer.ReplaceStr("\n", "\r");           // (\r instead of \n)
#endif

	m_nCurrentLine = 0;
	m_nCurrentIndex = 0;

#if BOF_MAC || BOF_WINMAC
	// Count number of carriage returns for mac scrolling
	m_nNumLines = m_cBuffer.FindNumOccurrences("\r");
#else
	m_nNumLines = m_cBuffer.FindNumOccurrences("\n");
#endif
}

int CBofTextBox::GetIndex(const int nLine) {
	Assert(nLine >= 0 && nLine <= m_nNumLines);

	const char *pszCur, *pszBuffer, *pszLast;
	int i, nChars;

	// Find the index into our buffer that represents the top left of the
	// buffer that is nLine from current the beginning of the buffer.
	pszLast = pszCur = pszBuffer = m_cBuffer;
	for (i = 0; i < nLine; i++) {
		pszLast = pszCur;
#if BOF_MAC || BOF_WINMAC
		pszCur = strstr(pszCur, "\r");      // look for cr's
#else
		pszCur = strstr(pszCur, "\n");
#endif
		// Make sure we don't go too far (nLines is invalid)
		Assert(pszCur != nullptr);

		pszCur++;
	}
	nChars = pszCur - pszBuffer;
	if (nLine == m_nNumLines) {
		nChars = pszLast - pszBuffer;
		m_nCurrentLine--;
	}

	return nChars;
}


ErrorCode CBofTextBox::ScrollUp(const int nLines) {
	int nNewLine;

	// Make scroll a no-op if all the lines in the box appear on one screen.
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
		//
		nNewLine = (m_nNumLines - m_nPageSize);
	}
	return ScrollTo(nNewLine);
}


ErrorCode CBofTextBox::ScrollTo(const int nLine) {
	Assert(IsValidObject(this));
	Assert(nLine >= 0 && nLine <= m_nNumLines);

	m_nCurrentIndex = GetIndex(nLine);
	m_nCurrentLine = nLine;

	// Show the text box
	Display();

	return m_errCode;
}


ErrorCode CBofTextBox::Display() {
	Assert(IsValidObject(this));
	Assert(m_nCurrentLine >= 0 && m_nCurrentLine <= m_nNumLines);
	Assert(m_nCurrentIndex >= 0 && m_nCurrentIndex < m_cBuffer.GetLength());

	// The actual text box must have been created before it can be displayed
	Assert(m_pTextField != nullptr);

	// If painting to a window
	if (m_pDestWindow != nullptr) {
		m_pTextField->Display(m_pDestWindow, m_cBuffer.Mid(m_nCurrentIndex), m_nPointSize, m_nWeight, m_cTextColor, m_nTextFont);

	} else {
		// Otherwise, must be painting to a bitmap
		Assert(m_pDestBitmap != nullptr);

		m_pTextField->Display(m_pDestBitmap, m_cBuffer.Mid(m_nCurrentIndex), m_nPointSize, m_nWeight, m_cTextColor, m_nTextFont);
	}

	return m_errCode;
}


ErrorCode CBofTextBox::Erase() {
	Assert(IsValidObject(this));

	// The actual text box must have been created before it can be displayed
	Assert(m_pTextField != nullptr);

	if (m_pDestWindow != nullptr) {
		m_errCode = m_pTextField->Erase(m_pDestWindow);

	} else {
		Assert(m_pDestBitmap != nullptr);
		m_errCode = m_pTextField->Erase(m_pDestBitmap);
	}

	return m_errCode;
}


void CBofTextBox::FlushBackground() {
	Assert(IsValidObject(this));

	if (m_pTextField != nullptr) {
		m_pTextField->FlushBackground();
	}
}

} // namespace Bagel
