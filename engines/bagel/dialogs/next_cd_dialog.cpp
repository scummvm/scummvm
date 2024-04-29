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
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define OK_BTN 5

CBagNextCDDialog::CBagNextCDDialog() {
	// Inits
	_nReturnValue = -1;
	m_pButton = nullptr;
	_lFlags = 0;
}

void CBagNextCDDialog::onInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::onInitDialog();

	setReturnValue(-1);

	CBofPalette *pPal;

	Assert(_pBackdrop != nullptr);
	pPal = _pBackdrop->getPalette();
	selectPalette(pPal);

	// Build all our buttons
	if ((m_pButton = new CBofBmpButton) != nullptr) {
		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = loadBitmap(BuildSysDir("CDOKUP.BMP"), pPal);
		pDown = loadBitmap(BuildSysDir("CDOKDN.BMP"), pPal);
		pFocus = loadBitmap(BuildSysDir("CDOKUP.BMP"), pPal);
		pDis = loadBitmap(BuildSysDir("CDOKUP.BMP"), pPal);

		m_pButton->loadBitmaps(pUp, pDown, pFocus, pDis);

		m_pButton->create("NextCD", 77, 127, 60, 30, this, OK_BTN);
		m_pButton->show();

	} else {
		ReportError(ERR_MEMORY);
	}

	// Show System cursor
	CBagCursor::showSystemCursor();
}

void CBagNextCDDialog::onClose() {
	Assert(IsValidObject(this));

	CBagCursor::hideSystemCursor();

	// Destroy my buttons
	if (m_pButton != nullptr) {
		delete m_pButton;
		m_pButton = nullptr;
	}

	CBofDialog::onClose();
}

void CBagNextCDDialog::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	_bFirstTime = false;

	// Paint the dialog
	if (_pBackdrop != nullptr) {
		_pBackdrop->paint(this, pRect, pRect, 1);
	}

	_bHavePainted = true;

	validateAnscestors();
}

void CBagNextCDDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	// Cancel
	case BKEY_ENTER:
	case BKEY_ESC:
		close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}

void CBagNextCDDialog::onBofButton(CBofObject * /*pObject*/, int nFlags) {
	Assert(IsValidObject(this));

	if (nFlags == BUTTON_CLICKED) {
		close();
	}
}

} // namespace Bagel
