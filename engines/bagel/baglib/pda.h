
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

#ifndef BAGEL_BAGLIB_PDA_H
#define BAGEL_BAGLIB_PDA_H

#include "bagel/baglib/base_pda.h"
#include "bagel/baglib/movie_object.h"

namespace Bagel {

#define PDA_INCREMENT 13

class CBagPDA : public CBagStorageDevBmp, public SBBasePda {
protected:
	INT m_nActiveHeight;
	INT m_nDeactiveHeight;
	static INT m_nLastUpdate;
	static BOOL m_bFlashing;
	static BOOL m_bSoundsPaused;

	static CBofList<CBagMovieObject *> *m_pMovieList;

public:
	CBagPDA(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), BOOL bActivated = FALSE);
	virtual ~CBagPDA();

	virtual ERROR_CODE Attach();

	VOID SetPosInWindow(int cx, int cy, int nDist = PDA_INCREMENT);

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT /* nMaskColor */ = -1);

	virtual BOOL PaintFGObjects(CBofBitmap *pBmp);

	virtual ERROR_CODE LoadFile(const CBofString &sFile);

	// CBagObject* 		OnNewWldObject(const CBofString& sInit);
	CBagObject *OnNewButtonObject(const CBofString &sInit);

	BOOL OnLButtonUp(UINT nFlags, CBofPoint xPoint, void *info = nullptr);
	virtual BOOL OnLButtonDown(UINT, CPoint, void * = nullptr);
	BOOL IsInside(const CBofPoint &xPoint);

	CBagObject *OnNewUserObject(const CBofString &sInit);

	static VOID AddToMovieQueue(CBagMovieObject *);
	static VOID RemoveFromMovieQueue(CBagMovieObject *);
	static BOOL IsMovieWaiting();
	static BOOL IsMoviePlaying() { return m_ePdaMode == MOOMODE; }
	static VOID RunWaitingMovie();

	virtual BOOL ShowInventory(); // Show the inventory
	virtual BOOL HideInventory(); // Hide the inventory

	virtual BOOL HideCurDisplay();    // Hide the current display
	virtual BOOL RestoreCurDisplay(); // restore display, must be paired with hide

	virtual ERROR_CODE AttachActiveObjects();
	virtual ERROR_CODE DetachActiveObjects();

	VOID HandleZoomButton(BOOL);
};

} // namespace Bagel

#endif
