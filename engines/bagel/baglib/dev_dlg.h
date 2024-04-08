
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
	char m_achGuess[MAX_CHARS + 2];
	CBofText *m_pGuessText;
	int m_nGuessCount;
	int m_nButtonX;
	int m_nButtonY;
	bool m_bUseExtra;
	CBofText *m_pTitleText;

public:
	CDevDlg(int nButtonX = -1, int nButtonY = -1);
	~CDevDlg();

	ERROR_CODE Create(const char *pszBmp, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *pRect, bool bUseEx = FALSE);
	void PaintText();
	void SetText(CBofString &, CBofRect *);
	void OnKeyHit(uint32 lKey, uint32 lRepCount);

	virtual void OnMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnClose();
	virtual ERROR_CODE OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);
};

} // namespace Bagel

#endif
