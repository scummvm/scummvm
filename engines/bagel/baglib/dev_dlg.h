
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

#ifndef BAGEL_BAGLIB_DEV_DLG_H
#define BAGEL_BAGLIB_DEV_DLG_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define MAX_CHARS 40

class CDevDlg : public CBagStorageDevDlg {
private:
	// for text entry:
	CHAR m_achGuess[MAX_CHARS + 2];
	CBofText *m_pGuessText;
	INT m_nGuessCount;
	INT m_nButtonX;
	INT m_nButtonY;
	BOOL m_bUseExtra;
	CBofText *m_pTitleText;

public:
	CDevDlg(INT nButtonX = -1, INT nButtonY = -1);
	~CDevDlg();

	ERROR_CODE Create(const CHAR *pszBmp, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *pRect, BOOL bUseEx = FALSE);
	VOID PaintText();
	VOID SetText(CBofString &, CBofRect *);
	VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnClose();
	virtual ERROR_CODE OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);
};

} // namespace Bagel

#endif
