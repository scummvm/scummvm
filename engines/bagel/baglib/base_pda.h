
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

#ifndef BAGEL_BAGLIB_BASE_PDA_H
#define BAGEL_BAGLIB_BASE_PDA_H

#include "bagel/boflib/stdinc.h"
#include "bagel/baglib/storage_dev_bmp.h"

namespace Bagel {

#define MAP (100)
#define STASH (101)
#define ZOOM (102)
#define SYSTEM (103)
#define LOG (104)
#define OFF (105)
#define MSGLIGHT (110)
#define ZOOMFLASH "ZOOMFLASH"

enum PDAMODE {
	NOMODE,
	MAPMODE,
	INVMODE,
	LOGMODE,
	MOOMODE
};
enum PDAPOS {
	UNINITIALIZED,
	PDAUP,
	PDADOWN
};

class SBBasePda {
protected:
	// All PDA's in the game should share the same mode
	static PDAMODE m_ePdaMode;
	static PDAPOS m_ePDAPos;
	int m_nNumMoves;
	int m_bActivating;
	bool m_bActivated;
	int m_nMoveDist;
	CBagStorageDevBmp *m_xMooWnd;     // Pointer to the PDAMOVIE object
	CBagStorageDevBmp *m_xInvWnd;     // Pointer to the inventory object
	CBagStorageDevBmp *m_xMapWnd;     // Pointer to the Map object
	CBagStorageDevBmp *m_xLogWnd;     // Pointer to the Map object
	CBagStorageDevBmp *m_xCurDisplay; // Pointer to the object currently displayed in PDA
	CBofWindow *m_pParent;
	bool m_bZoomed;
	bool m_bDeactivate; // should we deactivate when done w/movie?

	// hold this info for restore method
	CBagStorageDevBmp *m_xHoldDisplay;
	static PDAMODE m_eHoldMode;

public:
	SBBasePda(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), bool bActivated = FALSE);
	virtual ~SBBasePda();
	static void initialize();

	/**
	 * Allows PDA mode to be set by script
	 */
	static void SetPDAMode(PDAMODE ePDAMode) {
		m_ePdaMode = ePDAMode;
	}
	static PDAMODE GetPDAMode() {
		return m_ePdaMode;
	}
	bool IsActivated() {
		return m_bActivating ? !m_bActivated : m_bActivated;
	}
	bool IsActivating() {
		return m_bActivating;
	}

	virtual ERROR_CODE AttachActiveObjects();
	virtual ERROR_CODE DetachActiveObjects();

	/**
	 * Sync starting options
	 */
	void SynchronizePDAState();

	bool Deactivate() {
		if (!m_bActivating) {
			m_bActivating = m_nNumMoves;
			m_bActivated = FALSE;
		}

		m_ePDAPos = PDADOWN;
		SetPDAState();
		return true;
	}

	bool Activate() {
		if (!m_bActivating) {
			m_bActivating = m_nNumMoves;
			m_bActivated = true;
		}

		m_ePDAPos = PDAUP;
		SetPDAState();
		return true;
	}

	/**
	 * Show the inventory
	 */
	virtual bool ShowInventory();

	/**
	 * Hide the inventory
	 */
	virtual bool HideInventory();

	/**
	 * Show the movie window
	 */
	virtual bool ShowMovie();

	/**
	 * Hide the movie window
	 */
	virtual bool HideMovie();

	/**
	 * Set the movie to play
	 * @param s         Movie filename
	 * @return          Success/failure
	 */
	bool SetMovie(CBofString &s); // Set the movie

	/**
	 * Stops any playing movie
	 */
	void StopMovie(bool);

	void SetDeactivate(bool b = FALSE) {
		m_bDeactivate = b;
	}
	bool GetDeactivate() {
		return m_bDeactivate;
	}

	/**
	 * Show the map
	 */
	virtual bool ShowMap();

	/**
	 * Hide the map
	 */
	virtual bool HideMap();

	/**
	 * Zoom the current display
	 */
	virtual bool Zoom() {
		return true;
	}

	virtual bool ShowLog();

	virtual bool MsgLight();

	/**
	 * Hide the current display and reset the m_xCurDisplay to nullptr
	 * @return      Success/failure
	 */
	virtual bool HideCurDisplay();

	/**
	 * Hide the current display and reset the m_xCurDisplay to nullptr
	 * @return      Success/Failure
	 */
	virtual bool RestoreCurDisplay();

	static void *fPdaButtonHandler(int /* nRefId */, void *pvInfo);

	void SetPDAState();
	void GetPDAState();

	void SetZoomed(bool b = FALSE) {
		m_bZoomed = b;
	}
	bool GetZoomed() {
		return m_bZoomed;
	}

	int GetProperCursor(const CBofPoint &xPoint, CBofRect &pdaRect);

	/**
	 * Returns the background rect
	 */
	CBofRect GetViewRect();
};

#define GLOBALTOLOCAL 0
#define LOCALTOGLOBAL 1

} // namespace Bagel

#endif
