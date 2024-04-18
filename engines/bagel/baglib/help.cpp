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

#include "bagel/baglib/help.h"
#include "bagel/boflib/file.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

#define TEST 1

#define HELP_OK_X  257
#define HELP_OK_Y  377
#define HELP_OK_CX 125
#define HELP_OK_CY 30
#define HELP_OK_UP         "DONE1.BMP"
#define HELP_OK_DOWN       "DONE2.BMP"
#define HELP_OK_FOCUS      "DONE3.BMP"
#define HELP_OK_DISABLED   "DONE4.BMP"
#define HELP_OK_ID 1

#define HELP_PU_X  81
#define HELP_PU_Y  377
#define HELP_PU_CX 125
#define HELP_PU_CY 30
#define HELP_PU_UP         "PAGEUP1.BMP"
#define HELP_PU_DOWN       "PAGEUP2.BMP"
#define HELP_PU_FOCUS      "PAGEUP3.BMP"
#define HELP_PU_DISABLED   "PAGEUP4.BMP"
#define HELP_PU_ID 2

#define HELP_PD_X  435
#define HELP_PD_Y  377
#define HELP_PD_CX 125
#define HELP_PD_CY 30
#define HELP_PD_UP         "PAGEDN1.BMP"
#define HELP_PD_DOWN       "PAGEDN2.BMP"
#define HELP_PD_FOCUS      "PAGEDN3.BMP"
#define HELP_PD_DISABLED   "PAGEDN4.BMP"
#define HELP_PD_ID 3

// Local functions
const char *BuildHelpDir(const char *pszFile);


CBagHelp::CBagHelp() {
	m_pTextBox = nullptr;
	m_pOKButton = nullptr;
	m_pPageUp = nullptr;
	m_pPageDown = nullptr;
	m_pSavePalette = nullptr;
	SetFlags(0);
}

ErrorCode CBagHelp::Attach() {
	Assert(IsValidObject(this));

	CBofPalette *pPal;

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

#if !TEST
	CBagStorageDevWnd::Attach();
#endif

	if ((m_pOKButton = new CBofBmpButton) != nullptr) {

		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = LoadBitmap(BuildHelpDir(HELP_OK_UP), pPal);
		pDown = LoadBitmap(BuildHelpDir(HELP_OK_DOWN), pPal);
		pFocus = LoadBitmap(BuildHelpDir(HELP_OK_FOCUS), pPal);
		pDis = LoadBitmap(BuildHelpDir(HELP_OK_DISABLED), pPal);

		m_pOKButton->LoadBitmaps(pUp, pDown, pFocus, pDis);

		m_pOKButton->Create("OK", HELP_OK_X, HELP_OK_Y, HELP_OK_CX, HELP_OK_CY, this, HELP_OK_ID);
		m_pOKButton->Show();
	} else {
		ReportError(ERR_MEMORY);
	}

	if ((m_pPageUp = new CBofBmpButton) != nullptr) {

		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = LoadBitmap(BuildHelpDir(HELP_PU_UP), pPal);
		pDown = LoadBitmap(BuildHelpDir(HELP_PU_DOWN), pPal);
		pFocus = LoadBitmap(BuildHelpDir(HELP_PU_FOCUS), pPal);
		pDis = LoadBitmap(BuildHelpDir(HELP_PU_DISABLED), pPal);

		m_pPageUp->LoadBitmaps(pUp, pDown, pFocus, pDis);

		m_pPageUp->Create("PageUp", HELP_PU_X, HELP_PU_Y, HELP_PU_CX, HELP_PU_CY, this, HELP_PU_ID);
		m_pPageUp->Show();
	} else {
		ReportError(ERR_MEMORY);
	}

	if ((m_pPageDown = new CBofBmpButton) != nullptr) {

		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = LoadBitmap(BuildHelpDir(HELP_PD_UP), pPal);
		pDown = LoadBitmap(BuildHelpDir(HELP_PD_DOWN), pPal);
		pFocus = LoadBitmap(BuildHelpDir(HELP_PD_FOCUS), pPal);
		pDis = LoadBitmap(BuildHelpDir(HELP_PD_DISABLED), pPal);

		m_pPageDown->LoadBitmaps(pUp, pDown, pFocus, pDis);

		m_pPageDown->Create("PageDown", HELP_PD_X, HELP_PD_Y, HELP_PD_CX, HELP_PD_CY, this, HELP_PD_ID);
		m_pPageDown->Show();
	} else {
		ReportError(ERR_MEMORY);
	}

	CBofFile cFile(m_cTextFile, CBF_BINARY | CBF_READONLY);
	CBofRect cRect;
	char *pszBuf;
	uint32 lSize;

	lSize = cFile.GetLength();
	if ((pszBuf = (char *)BofCAlloc(lSize + 1, 1)) != nullptr) {

		cFile.Read(pszBuf, lSize);

		cRect.SetRect(120, 100, 550, 348);
		if ((m_pTextBox = new CBofTextBox(this, &cRect, pszBuf)) != nullptr) {
			m_pTextBox->SetPageLength(18);

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate a CBofTextBox");
		}

		BofFree(pszBuf);

	} else {
		ReportError(ERR_MEMORY, "Unable to allocate %d bytes to read %s.", lSize, m_cTextFile.GetBuffer());
	}

	CBofCursor::Show();

	return m_errCode;
}


ErrorCode CBagHelp::Detach() {
	Assert(IsValidObject(this));

	CBofCursor::Hide();

	if (m_pTextBox != nullptr) {
		delete m_pTextBox;
		m_pTextBox = nullptr;
	}
	if (m_pPageDown != nullptr) {
		delete m_pPageDown;
		m_pPageDown = nullptr;
	}
	if (m_pPageUp != nullptr) {
		delete m_pPageUp;
		m_pPageUp = nullptr;
	}
	if (m_pOKButton != nullptr) {
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

#if !TEST
	CBagStorageDevWnd::Detach();
#endif

	CBofApp::GetApp()->SetPalette(m_pSavePalette);

	return m_errCode;
}

ErrorCode CBagHelp::SetHelpFile(const char *pszTextFile) {
	Assert(IsValidObject(this));
	Assert(pszTextFile != nullptr);

	m_cTextFile = pszTextFile;

	return m_errCode;
}


void CBagHelp::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

#if !TEST
	CBagStorageDevWnd::OnPaint(pRect);
#else
	//CBofDialog::OnPaint(pRect);
	PaintBackdrop(pRect);
#endif

	if (m_pTextBox != nullptr) {
		m_pTextBox->Display();
	}
	ValidateAnscestors();
}


void CBagHelp::OnKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_UP:
		if (m_pTextBox != nullptr) {
			m_pTextBox->LineUp();
		}
		break;

	case BKEY_DOWN:
		if (m_pTextBox != nullptr) {
			m_pTextBox->LineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (m_pTextBox != nullptr) {
			m_pTextBox->PageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (m_pTextBox != nullptr) {
			m_pTextBox->PageDown();
		}
		break;

	case BKEY_ESC:
	case BKEY_ENTER:
		Close();

	default:
#if !TEST
		CBagStorageDevWnd::OnKeyHit(lKey, nRepCount);
#else
		CBofDialog::OnKeyHit(lKey, nRepCount);
#endif
		break;
	}
}


void CBagHelp::OnBofButton(CBofObject *pObject, int nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	CBofBmpButton *pButton;

	pButton = (CBofBmpButton *)pObject;

	if (nFlags == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case HELP_OK_ID:
			Close();
			break;

		case HELP_PU_ID:
			if (m_pTextBox != nullptr) {
				m_pTextBox->PageUp();
			}
			break;

		case HELP_PD_ID:
			if (m_pTextBox != nullptr) {
				m_pTextBox->PageDown();
			}
			break;

		default:
			LogWarning("Unknown button");
			break;
		}
	}
}

const char *BuildHelpDir(const char *pszFile) {
	Assert(pszFile != nullptr);

	static char szBuf[MAX_DIRPATH];

	Common::sprintf_s(szBuf, "$SBARDIR%sGENERAL%sRULES%s%s", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER, pszFile);

	CBofString cTemp(szBuf, MAX_DIRPATH);
	MACROREPLACE(cTemp);

	return &szBuf[0];
}


#if TEST
void CBagHelp::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();

	Attach();
}
#endif

} // namespace Bagel
