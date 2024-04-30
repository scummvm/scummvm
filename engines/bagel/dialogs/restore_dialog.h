
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

#ifndef BAGEL_DIALOGS_RESTORE_DIALOG_H
#define BAGEL_DIALOGS_RESTORE_DIALOG_H

#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/scroll_bar.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/string.h"
#include "bagel/dialogs/save_dialog.h"

namespace Bagel {

#define NUM_RESTORE_BTNS 6

class CBagRestoreDialog : public CBofDialog {
private:
	ErrorCode RestoreAndclose();

	CBofBmpButton *_pButtons[NUM_RESTORE_BTNS];
	CBofScrollBar *_pScrollBar = nullptr;

	CBofText *_pText = nullptr;
	CBofListBox *_pListBox = nullptr;
	int _nSelectedItem = -1;
	ST_BAGEL_SAVE *_pSaveBuf = nullptr;
	int _nBufSize = 0;
	bool _bRestored = false;
	CBofPalette *_pSavePalette = nullptr;
	SaveStateList _savesList;

protected:
	void onPaint(CBofRect *pRect) override;
	void onBofButton(CBofObject *pObject, int nState) override;
	void onBofListBox(CBofObject *pObject, int nItemIndex) override;

	void onKeyHit(uint32 lKey, uint32 lRepCount) override;

public:
	CBagRestoreDialog();

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	ST_BAGEL_SAVE *getSaveGameBuffer(int &nLength) {
		nLength = _nBufSize;
		return _pSaveBuf;
	}

	void setSaveGameBuffer(ST_BAGEL_SAVE *pBuf, int nLength) {
		_pSaveBuf = pBuf;
		_nBufSize = nLength;
	}

	bool restored() {
		return _bRestored;
	}

	void onInitDialog() override;
};

} // namespace Bagel

#endif
