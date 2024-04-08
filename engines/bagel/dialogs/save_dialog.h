
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

#ifndef BAGEL_BAGLIB_SAVE_DIALOG_H
#define BAGEL_BAGLIB_SAVE_DIALOG_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/save_game_file.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/scroll_bar.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/edit_text.h"
#include "bagel/boflib/string.h"

namespace Bagel {

#define NUM_BUTTONS 6

class CBagSaveDialog : public CBofDialog {
public:
	CBagSaveDialog();

#if BOF_DEBUG
	virtual ~CBagSaveDialog();
#endif

	virtual ErrorCode Attach();
	virtual ErrorCode Detach();

	byte *GetSaveGameBuffer(int &nLength) {
		nLength = m_nBufSize;
		return m_pSaveBuf;
	}

	void SetSaveGameBuffer(byte *pBuf, int nLength) {
		m_pSaveBuf = pBuf;
		m_nBufSize = nLength;
	}

#if 1
	virtual void OnInitDialog();
#endif

protected:
	virtual void OnPaint(CBofRect *pRect);
	virtual void OnBofButton(CBofObject *pObject, int nState);
	virtual void OnBofListBox(CBofObject *pObject, int nItemIndex);
#if BOF_MAC
	virtual void OnMainLoop();
#endif
	virtual void OnKeyHit(uint32 lKey, uint32 lRepCount);

	void SaveAndClose();

	// Data
	//
	ST_SAVEDGAME_HEADER m_stGameInfo;
	CBofBmpButton *m_pButtons[NUM_BUTTONS];
	CBofScrollBar *m_pScrollBar;

	CBofEditText *m_pEditText;
	CBofListBox *m_pListBox;
	int m_nSelectedItem;
	byte *m_pSaveBuf;
	int m_nBufSize;
	CBofPalette *m_pSavePalette;
	SaveStateList _savesList;
};

const char *BuildSRDir(const char *pszFile);
} // namespace Bagel

#endif
