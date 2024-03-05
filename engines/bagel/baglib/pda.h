
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
	/**
	 * Constructor
	 * @param pParent       Pointer to the parent window
	 * @param xPoint        Upper-left corner of PDA in parent window
	 * @param bActivated    State of PDA whe constructed (optional)
	 */
	CBagPDA(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), BOOL bActivated = FALSE);

	/**
	 * Destructor
	 */
	virtual ~CBagPDA();
	static void initStatics();

	virtual ERROR_CODE Attach();

	/**
	 * Positions the PDA centered vertically and window
	 * @param width     Width of area to center pda
	 * @param height    Height of area to place pda
	 * @param nDist     Distance to bring move PDA
	 */
	VOID SetPosInWindow(int cx, int cy, int nDist = PDA_INCREMENT);

	/**
	 * Update the PDA to the screen and then call CBagStorageDevBmp Update
	 * @param pBmp          Background bitmap to paint to
	 * @param pt            Offset into bitmap
	 * @param pSrcRect      Clip rect of the object to be painted
	 * @param nMaskColor    Mask color for the object
	*/
	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT /* nMaskColor */ = -1);

	/**
	 * Called to overload specific types of sprite objects
	 * @param pBmp          Bitmap
	 * @return              Success/failure
	 */
	virtual BOOL PaintFGObjects(CBofBitmap *pBmp);

	virtual ERROR_CODE LoadFile(const CBofString &sFile);

	/**
	 * Called to overload specific types of sprite objects
	 * @param sInit     Init
	 * @return          Pointer to the new object
	 */
	CBagObject *OnNewButtonObject(const CBofString &sInit);

	/**
	 * Called on the mouse left button down
	 * Is the PDA is deactivated mouse down activates the PDA
	 * else it call CBagStorageDevBmp::OnLButtonDown
	 * @param nFlags        Flags
	 * @param xPoint        X, Y position
	 * @param info          Optional info
	 */
	BOOL OnLButtonUp(UINT nFlags, CBofPoint xPoint, void *info = nullptr);

	/**
	 * Called on the mouse left button down
	 * @param nFlags        Flags
	 * @param xPoint        X, Y position
	 * @param info          Optional info
	 */
	virtual BOOL OnLButtonDown(UINT nFlags, CBofPoint xPoint, void *info = nullptr);

	BOOL IsInside(const CBofPoint &xPoint);

	CBagObject *OnNewUserObject(const CBofString &sInit);

	/**
	 * Allow the movie code to queue up asynch pda messages
	 * Must be before the destructor
	 */
	static VOID AddToMovieQueue(CBagMovieObject *);

	/**
	 * Just remove this message from the queue, remember that we
	 * don't own this object, so just remove it, don't delete it!!!!
	 */
	static VOID RemoveFromMovieQueue(CBagMovieObject *);

	/**
	 * Return true if a movie is waiting to play.
	 */
	static BOOL IsMovieWaiting();

	static BOOL IsMoviePlaying() {
		return m_ePdaMode == MOOMODE;
	}

	static VOID RunWaitingMovie();

	/**
	 * SHow the inventory
	*/
	virtual BOOL ShowInventory();

	/**
	 * Hide the inventory
	 */
	virtual BOOL HideInventory();

	/**
	 * Hide the current display
	 */
	virtual BOOL HideCurDisplay();

	/**
	 * Restore display
	 */
	virtual BOOL RestoreCurDisplay();

	virtual ERROR_CODE AttachActiveObjects();
	virtual ERROR_CODE DetachActiveObjects();

	/**
	 * Handle switching back and forth from our flashing zoom button
	 */
	VOID HandleZoomButton(BOOL bButtonDown);
};

} // namespace Bagel

#endif
