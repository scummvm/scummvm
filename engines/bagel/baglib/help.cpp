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
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define RULES_DIR "$SBARDIR\\GENERAL\\RULES\\"

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
	setFlags(0);
}

ErrorCode CBagHelp::attach() {
	Assert(IsValidObject(this));

	CBofPalette *pPal;

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->getPalette();

	// Insert ours
	pPal = _pBackdrop->getPalette();
	CBofApp::GetApp()->setPalette(pPal);

	if ((m_pOKButton = new CBofBmpButton) != nullptr) {
		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = loadBitmap(BuildHelpDir(HELP_OK_UP), pPal);
		pDown = loadBitmap(BuildHelpDir(HELP_OK_DOWN), pPal);
		pFocus = loadBitmap(BuildHelpDir(HELP_OK_FOCUS), pPal);
		pDis = loadBitmap(BuildHelpDir(HELP_OK_DISABLED), pPal);

		m_pOKButton->loadBitmaps(pUp, pDown, pFocus, pDis);

		m_pOKButton->create("OK", HELP_OK_X, HELP_OK_Y, HELP_OK_CX, HELP_OK_CY, this, HELP_OK_ID);
		m_pOKButton->show();
	} else {
		ReportError(ERR_MEMORY);
	}

	if ((m_pPageUp = new CBofBmpButton) != nullptr) {

		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = loadBitmap(BuildHelpDir(HELP_PU_UP), pPal);
		pDown = loadBitmap(BuildHelpDir(HELP_PU_DOWN), pPal);
		pFocus = loadBitmap(BuildHelpDir(HELP_PU_FOCUS), pPal);
		pDis = loadBitmap(BuildHelpDir(HELP_PU_DISABLED), pPal);

		m_pPageUp->loadBitmaps(pUp, pDown, pFocus, pDis);

		m_pPageUp->create("PageUp", HELP_PU_X, HELP_PU_Y, HELP_PU_CX, HELP_PU_CY, this, HELP_PU_ID);
		m_pPageUp->show();
	} else {
		ReportError(ERR_MEMORY);
	}

	if ((m_pPageDown = new CBofBmpButton) != nullptr) {

		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = loadBitmap(BuildHelpDir(HELP_PD_UP), pPal);
		pDown = loadBitmap(BuildHelpDir(HELP_PD_DOWN), pPal);
		pFocus = loadBitmap(BuildHelpDir(HELP_PD_FOCUS), pPal);
		pDis = loadBitmap(BuildHelpDir(HELP_PD_DISABLED), pPal);

		m_pPageDown->loadBitmaps(pUp, pDown, pFocus, pDis);

		m_pPageDown->create("PageDown", HELP_PD_X, HELP_PD_Y, HELP_PD_CX, HELP_PD_CY, this, HELP_PD_ID);
		m_pPageDown->show();
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
			m_pTextBox->setPageLength(18);

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate a CBofTextBox");
		}

		BofFree(pszBuf);

	} else {
		ReportError(ERR_MEMORY, "Unable to allocate %d bytes to read %s.", lSize, m_cTextFile.GetBuffer());
	}

	CBagCursor::showSystemCursor();

	return _errCode;
}


ErrorCode CBagHelp::Detach() {
	Assert(IsValidObject(this));

	CBagCursor::hideSystemCursor();

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

	CBofApp::GetApp()->setPalette(m_pSavePalette);

	return _errCode;
}

ErrorCode CBagHelp::SetHelpFile(const char *pszTextFile) {
	Assert(IsValidObject(this));
	Assert(pszTextFile != nullptr);

	m_cTextFile = pszTextFile;

	return _errCode;
}


void CBagHelp::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	paintBackdrop(pRect);

	if (m_pTextBox != nullptr) {
		m_pTextBox->display();
	}

	validateAnscestors();
}


void CBagHelp::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_UP:
		if (m_pTextBox != nullptr) {
			m_pTextBox->lineUp();
		}
		break;

	case BKEY_DOWN:
		if (m_pTextBox != nullptr) {
			m_pTextBox->lineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (m_pTextBox != nullptr) {
			m_pTextBox->pageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (m_pTextBox != nullptr) {
			m_pTextBox->pageDown();
		}
		break;

	case BKEY_ESC:
	case BKEY_ENTER:
		close();

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}


void CBagHelp::onBofButton(CBofObject *pObject, int nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	switch (pButton->getControlID()) {
	case HELP_OK_ID:
		close();
		break;

	case HELP_PU_ID:
		if (m_pTextBox != nullptr) {
			m_pTextBox->pageUp();
		}
		break;

	case HELP_PD_ID:
		if (m_pTextBox != nullptr) {
			m_pTextBox->pageDown();
		}
		break;

	default:
		LogWarning("Unknown button");
		break;
	}
}

const char *BuildHelpDir(const char *pszFile) {
	return formPath(RULES_DIR, pszFile);
}

void CBagHelp::onInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::onInitDialog();

	attach();
}

} // namespace Bagel
