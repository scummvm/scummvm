
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

#ifndef BAGEL_BAGLIB_RESTORE_DIALOG_H
#define BAGEL_BAGLIB_RESTORE_DIALOG_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/scroll_bar.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/edit_text.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/string.h"
#include "bagel/baglib/save_dialog.h"

namespace Bagel {

#define NUM_RESTORE_BTNS 6

const CHAR *BuildSysDir(const CHAR *pszFile);

class CBagRestoreDialog : public CBofDialog {
public:
	CBagRestoreDialog();

#if BOF_DEBUG
	virtual ~CBagRestoreDialog();
#endif

	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	ST_BAGEL_SAVE *GetSaveGameBuffer(INT &nLength) {
		nLength = m_nBufSize;
		return m_pSaveBuf;
	}

	VOID SetSaveGameBuffer(ST_BAGEL_SAVE *pBuf, INT nLength) {
		m_pSaveBuf = pBuf;
		m_nBufSize = nLength;
	}

	BOOL Restored() {
		return m_bRestored;
	}

#if 1
	virtual VOID OnInitDialog();
#endif

protected:
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnBofButton(CBofObject *pObject, INT nState);
	virtual VOID OnBofListBox(CBofObject *pObject, INT nItemIndex);

	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	ERROR_CODE RestoreAndClose();

	// Data
	//
	ST_SAVEDGAME_HEADER m_stGameInfo;
	CBofBmpButton *m_pButtons[NUM_RESTORE_BTNS];
	CBofScrollBar *m_pScrollBar;

	CBofText *m_pText;
	CBofListBox *m_pListBox;
	CBagSaveGameFile *m_pSaveGameFile;
	INT m_nSelectedItem;
	ST_BAGEL_SAVE *m_pSaveBuf;
	INT m_nBufSize;
	BOOL m_bRestored;
	CBofPalette *m_pSavePalette;
};

} // namespace Bagel

#endif
