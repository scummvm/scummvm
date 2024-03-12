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

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/edit_text.h"

namespace Bagel {


CBofEditText::CBofEditText(const CHAR *pszName, INT x, INT y, INT nWidth, INT nHeight, CBofWindow *pParent)
	: CBofWindow(pszName, x, y, nWidth, nHeight, pParent) {
#if BOF_MAC
	m_hTE = nullptr;
#endif

	Create(pszName, x, y, nWidth, nHeight, pParent);
}


ERROR_CODE CBofEditText::Create(const CHAR *pszName, CBofRect *pRect, CBofWindow *pParent, UINT nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// remember who our parent is
	_parent = pParent;

	INT x, y, nWidth, nHeight;

	x = y = 0;
	nWidth = nHeight = USE_DEFAULT;
	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->Width();
		nHeight = pRect->Height();
	}

	return Create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}


ERROR_CODE CBofEditText::Create(const CHAR *pszName, INT x, INT y, INT nWidth, INT nHeight, CBofWindow *pParent, UINT nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// remember who our parent is
	_parent = pParent;
	m_nID = nControlID;

	// remember the name of this window
	strncpy(m_szTitle, pszName, MAX_TITLE);

	// retain screen coordinates for this window
	m_cWindowRect.SetRect(x, y, x + nWidth - 1, y + nHeight - 1);

#if BOF_WINDOWS

	DWORD dwStyle;
	HWND hParent;

	hParent = nullptr;
	dwStyle = WS_POPUP;
	if (pParent != nullptr) {
		hParent = pParent->GetHandle();
		dwStyle = WS_CHILD;
	}
	dwStyle |= ES_AUTOHSCROLL | ES_LEFT;

	if ((m_hWnd = ::CreateWindow("EDIT", pszName, dwStyle, x, y, nWidth, nHeight, hParent, nullptr, CBofApp::GetInstanceHandle(), nullptr)) != nullptr) {

		RECT rect;
		::GetWindowRect(m_hWnd, &rect);

		// reset screen coordinates (in case we are using the defaults set up
		// by CreateWindow).
		//
		m_cWindowRect = rect;

	} else {
		ReportError(ERR_UNKNOWN, "Unable to CreateWindow(%s)", pszName);
	}

#elif BOF_MAC
	UCHAR szBuf[256];
	Rect stRect = {y, x, y + nHeight, x + nWidth};

	strcpy((CHAR *)szBuf, m_szTitle);
	StrCToPascal((CHAR *)szBuf);

	if ((m_pWindow = NewCWindow(nullptr, &stRect, szBuf, FALSE, 2, WindowPtr(-1), FALSE, 0)) != nullptr) {

		SetPort(m_pWindow);

		if (m_hTE != nullptr) {
			TEDelete(m_hTE);
		}

		if ((m_hTE = TEStyleNew(&m_pWindow->portRect, &m_pWindow->portRect)) != nullptr) {
			TEAutoView(TRUE, m_hTE);
			TECalText(m_hTE);
		} else {
			ReportError(ERR_MEMORY, "Could not allocate a new TextEdit control");
		}

	} else {
		ReportError(ERR_UNKNOWN, "Unable to NewCWindow(%s)", pszName);
	}

#endif

	if (!ErrorOccurred()) {

		CBofPalette *pPalette;
		if ((pPalette = CBofApp::GetApp()->GetPalette()) != nullptr) {
			SelectPalette(pPalette);
		}

		// retain local coordinates (based on own window)
		m_cRect.SetRect(0, 0, m_cWindowRect.Width() - 1, m_cWindowRect.Height() - 1);
	}

	return m_errCode;
}


CBofString CBofEditText::GetText() {
	Assert(IsValidObject(this));
	Assert(IsCreated());

	CBofString cString;

#if BOF_WINDOWS
	CHAR *pBuf;
	INT nLength;

	if ((nLength = GetWindowTextLength(m_hWnd)) > 0) {
		nLength++;

		if ((pBuf = new CHAR[nLength]) != nullptr) {

			GetWindowText(m_hWnd, pBuf, nLength);

			cString = pBuf;

			delete pBuf;

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate %d bytes for text in a CBofEditText control", nLength);
		}
	}

#elif BOF_MAC
	CharsHandle hChar;
	CHAR *p;
	CHAR pHold;

	//  make sure we lock down the buffer before
	//  calling the copy constructor, also, this buffer is not
	//  null terminated, so fake it out.

	hChar = TEGetText(m_hTE);           // get the handle to the buff
	HLock((Handle) hChar);              // lock down the string
	p = *hChar;                         // get a pointer to the string
	pHold = p[(*m_hTE)->teLength];      // preserve the last char
	p[(*m_hTE)->teLength] = 0;          // null terminate the actual string
	cString = p;                        // call copy constructor
	p[(*m_hTE)->teLength] = pHold;      // replace the lost byte
	HUnlock((Handle) hChar);            // return the string to it's unlocked state

#endif

	return cString;
}


VOID CBofEditText::SetText(const CHAR *pszString) {
	Assert(IsValidObject(this));
	Assert(IsCreated());
	Assert(pszString != nullptr);

#if BOF_WINDOWS
	SetWindowText(m_hWnd, pszString);

#elif BOF_MAC
	TESetText(pszString, strlen(pszString), m_hTE);

	// invalidate to force an update, make sure
	// our port is current

	GrafPtr savePort;
	GetPort(&savePort);
	SetPort(m_pWindow);

	//::EraseRect (&(*m_hTE)->destRect);
	::InvalRect(&m_pWindow->portRect);

	SetPort(savePort);
#endif
}


#if BOF_MAC
// jKey just enters a key into the text edit buffer.

VOID CBofEditText::Key(const CHAR key) {
	Assert(IsValidObject(this));
	Assert(IsCreated());

	SetPort(m_pWindow);
	::TECut(m_hTE);         // trash selection range if there is one
	::TEKey(key, m_hTE);
	::TEUpdate(&m_pWindow->portRect, m_hTE);
}


VOID CBofEditText::Destroy() {
	if (m_hTE != nullptr) {
		TEDelete(m_hTE);
		m_hTE = nullptr;
	}
	CBofWindow::Destroy();
}


VOID CBofEditText::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	if (m_hTE != nullptr) {
		TextStyle stStyle;
		Rect stRect = *pRect;

		stStyle.tsColor.red = GetRed(m_cFgColor);
		stStyle.tsColor.green = GetGreen(m_cFgColor);
		stStyle.tsColor.blue = GetBlue(m_cFgColor);

		TESetStyle(doColor, &stStyle, FALSE, m_hTE);
		TEUpdate(&stRect, m_hTE);
	}
}

VOID CBofEditText::OnActivate() {
	Assert(IsValidObject(this));

	if (m_hTE != nullptr) {
		TEActivate(m_hTE);
	}
}

VOID CBofEditText::OnDeActivate() {
	Assert(IsValidObject(this));

	if (m_hTE != nullptr) {
		TEDeactivate(m_hTE);
	}
}

VOID CBofEditText::OnSelect() {
	//  select the entire range...

	if (m_hTE) {
		::TESetSelect(0, (*m_hTE)->teLength, m_hTE);
	}
}

VOID CBofEditText::OnLButtonDown(UINT /*nFlags*/, CBofPoint *xPoint) {
	if (m_hTE) {
		::TESetSelect(0, 0, m_hTE);
	}
}
#endif

} // namespace Bagel
