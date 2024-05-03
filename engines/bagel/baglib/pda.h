
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
	int m_nActiveHeight;
	int m_nDeactiveHeight;
	static bool m_bFlashing;
	static bool m_bSoundsPaused;

	static CBofList<CBagMovieObject *> *m_pMovieList;

public:
	/**
	 * Constructor
	 * @param pParent       Pointer to the parent window
	 * @param xPoint        Upper-left corner of PDA in parent window
	 * @param bActivated    State of PDA whe constructed (optional)
	 */
	CBagPDA(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), bool bActivated = false);

	/**
	 * Destructor
	 */
	virtual ~CBagPDA();
	static void initialize();

	ErrorCode attach() override;

	/**
	 * Positions the PDA centered vertically and window
	 * @param cx        x position
	 * @param cy        y position
	 * @param nDist     Distance to bring move PDA
	 */
	void setPosInWindow(int cx, int cy, int nDist = PDA_INCREMENT);

	/**
	 * Update the PDA to the screen and then call CBagStorageDevBmp update
	 * @param pBmp          Background bitmap to paint to
	 * @param pt            Offset into bitmap
	 * @param pSrcRect      Clip rect of the object to be painted
	 * @param nMaskColor    Mask color for the object
	*/
	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /* nMaskColor */ = -1) override;

	/**
	 * Called to overload specific types of sprite objects
	 * @param pBmp          Bitmap
	 * @return              Success/failure
	 */
	virtual bool PaintFGObjects(CBofBitmap *pBmp) override;

	virtual ErrorCode loadFile(const CBofString &sFile) override;

	/**
	 * Called to overload specific types of sprite objects
	 * @param sInit     Init
	 * @return          Pointer to the new object
	 */
	CBagObject *onNewButtonObject(const CBofString &sInit) override;

	/**
	 * Called on the mouse left button down
	 * Is the PDA is deactivated mouse down activates the PDA
	 * else it calls CBagStorageDevBmp::onLButtonDown
	 * @param nFlags        Flags
	 * @param xPoint        X, Y position
	 * @param info          Optional info
	 */
	void onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info = nullptr) override;

	/**
	 * Called on the mouse left button down
	 * @param nFlags        Flags
	 * @param xPoint        X, Y position
	 * @param info          Optional info
	 */
	virtual void onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info = nullptr) override;

	bool isInside(const CBofPoint &xPoint) override;

	CBagObject *onNewUserObject(const CBofString &sInit) override;

	/**
	 * Allow the movie code to queue up asynch pda messages
	 * Must be before the destructor
	 */
	static void AddToMovieQueue(CBagMovieObject *);

	/**
	 * Just remove this message from the queue, remember that we
	 * don't own this object, so just remove it, don't delete it!!!!
	 */
	static void RemoveFromMovieQueue(CBagMovieObject *);

	/**
	 * Return true if a movie is waiting to play.
	 */
	static bool IsMovieWaiting();

	static bool IsMoviePlaying() {
		return _pdaMode == MOOMODE;
	}

	static void RunWaitingMovie();

	/**
	 * SHow the inventory
	*/
	virtual bool showInventory() override;

	/**
	 * Hide the inventory
	 */
	virtual bool hideInventory() override;

	/**
	 * Hide the current display
	 */
	virtual bool hideCurDisplay() override;

	/**
	 * Restore display
	 */
	virtual bool restoreCurDisplay() override;

	virtual ErrorCode attachActiveObjects() override;
	virtual ErrorCode detachActiveObjects() override;

	/**
	 * Handle switching back and forth from our flashing zoom button
	 */
	void HandleZoomButton(bool bButtonDown);
};

} // namespace Bagel

#endif
