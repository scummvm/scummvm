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

#include "bagel/dialogs/next_cd_dialog.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/cursor.h"
#include "bagel/baglib/buttons.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

#define OK_BTN 5

CBagNextCDDialog::CBagNextCDDialog() {
	// Inits
	_nReturnValue = -1;
	m_pButton = nullptr;
	_lFlags = 0;
}

void CBagNextCDDialog::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();

	SetReturnValue(-1);

	CBofPalette *pPal;

	Assert(m_pBackdrop != nullptr);
	pPal = m_pBackdrop->GetPalette();
	SelectPalette(pPal);

	// Build all our buttons
	if ((m_pButton = new CBofBmpButton) != nullptr) {
		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = LoadBitmap(BuildSysDir("CDOKUP.BMP"), pPal);
		pDown = LoadBitmap(BuildSysDir("CDOKDN.BMP"), pPal);
		pFocus = LoadBitmap(BuildSysDir("CDOKUP.BMP"), pPal);
		pDis = LoadBitmap(BuildSysDir("CDOKUP.BMP"), pPal);

		m_pButton->LoadBitmaps(pUp, pDown, pFocus, pDis);

		m_pButton->Create("NextCD", 77, 127, 60, 30, this, OK_BTN);
		m_pButton->Show();

	} else {
		ReportError(ERR_MEMORY);
	}

	// Show System cursor
	CBagCursor::ShowSystemCursor();
}

void CBagNextCDDialog::OnClose() {
	Assert(IsValidObject(this));

	CBagCursor::HideSystemCursor();

	// Destroy my buttons
	if (m_pButton != nullptr) {
		delete m_pButton;
		m_pButton = nullptr;
	}

	CBofDialog::OnClose();
}

void CBagNextCDDialog::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	_bFirstTime = false;

	// Paint the dialog
	if (m_pBackdrop != nullptr) {
		m_pBackdrop->Paint(this, pRect, pRect, 1);
	}

	// Paint to stop it showing up behind backdrop
#if BOF_MAC
	m_pButton->Paint();
#endif

	_bHavePainted = true;

	ValidateAnscestors();
}

void CBagNextCDDialog::OnKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {

	// Cancel
	case BKEY_ENTER:
	case BKEY_ESC:
		Close();
		break;

	default:
		CBofDialog::OnKeyHit(lKey, nRepCount);
		break;
	}
}

void CBagNextCDDialog::OnBofButton(CBofObject * /*pObject*/, int nFlags) {
	Assert(IsValidObject(this));

	if (nFlags == BUTTON_CLICKED) {
		Close();
	}
}

} // namespace Bagel
