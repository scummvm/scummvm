
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

#ifndef BAGEL_DIALOGS_SAVE_DIALOG_H
#define BAGEL_DIALOGS_SAVE_DIALOG_H

#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/scroll_bar.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/edit_text.h"

#include "engines/savestate.h"

namespace Bagel {

#define NUM_BUTTONS 6

class CBagSaveDialog : public CBofDialog {
public:
	CBagSaveDialog();

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	byte *getSaveGameBuffer(int &nLength) {
		nLength = _nBufSize;
		return _pSaveBuf;
	}

	void setSaveGameBuffer(byte *pBuf, int nLength) {
		_pSaveBuf = pBuf;
		_nBufSize = nLength;
	}

	void onInitDialog() override;

protected:
	void onPaint(CBofRect *pRect) override;
	void onBofButton(CBofObject *pObject, int nFlags) override;
	void onBofListBox(CBofObject *pObject, int nItemIndex) override;
	void onKeyHit(uint32 lKey, uint32 nRepCount) override;

	void saveAndClose();

	CBofBmpButton *_pButtons[NUM_BUTTONS];
	CBofScrollBar *_pScrollBar;

	CBofEditText *_pEditText;
	CBofListBox *_pListBox;
	int _nSelectedItem;
	byte *_pSaveBuf;
	int _nBufSize;
	CBofPalette *_pSavePalette;
	SaveStateList _savesList;
};

} // namespace Bagel

#endif
