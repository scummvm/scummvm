
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

	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	UBYTE *GetSaveGameBuffer(INT &nLength) {
		nLength = m_nBufSize;
		return m_pSaveBuf;
	}

	VOID SetSaveGameBuffer(UBYTE *pBuf, INT nLength) {
		m_pSaveBuf = pBuf;
		m_nBufSize = nLength;
	}

#if 1
	virtual VOID OnInitDialog();
#endif

protected:
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnBofButton(CBofObject *pObject, INT nState);
	virtual VOID OnBofListBox(CBofObject *pObject, INT nItemIndex);
#if BOF_MAC
	virtual VOID OnMainLoop();
#endif
	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	VOID SaveAndClose();

	// Data
	//
	ST_SAVEDGAME_HEADER m_stGameInfo;
	CBofBmpButton *m_pButtons[NUM_BUTTONS];
	CBofScrollBar *m_pScrollBar;

	CBofEditText *m_pEditText;
	CBofListBox *m_pListBox;
	CBagSaveGameFile *m_pSaveGameFile;
	INT m_nSelectedItem;
	UBYTE *m_pSaveBuf;
	INT m_nBufSize;
#if BOF_MAC
	BOOL m_bResetFocus;
#endif
	CBofPalette *m_pSavePalette;
};

const CHAR *BuildSRDir(const CHAR *pszFile);
} // namespace Bagel

#endif
