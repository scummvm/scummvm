
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

const enum PDAMODE {
	NOMODE,
	MAPMODE,
	INVMODE,
	LOGMODE,
	MOOMODE
};
const enum PDAPOS {
	UNINITIALIZED,
	PDAUP,
	PDADOWN
};

class SBBasePda {
protected:
	// All PDA's in the game should share the same mode
	static PDAMODE m_ePdaMode;
	static PDAPOS m_ePDAPos;
	INT m_nNumMoves;
	INT m_bActivating;
	BOOL m_bActivated;
	INT m_nMoveDist;
	CBagStorageDevBmp *m_xMooWnd;     // Pointer to the PDAMOVIE object
	CBagStorageDevBmp *m_xInvWnd;     // Pointer to the inventory object
	CBagStorageDevBmp *m_xMapWnd;     // Pointer to the Map object
	CBagStorageDevBmp *m_xLogWnd;     // Pointer to the Map object
	CBagStorageDevBmp *m_xCurDisplay; // Pointer to the object currently displayed in PDA
	CBofWindow *m_pParent;
	BOOL m_bZoomed;
	BOOL m_bDeactivate; // should we deactivate when done w/movie?

	// jwl 1.3.97 hold this info for restore method
	CBagStorageDevBmp *m_xHoldDisplay;
	static PDAMODE m_eHoldMode;

public:
	SBBasePda(CBofWindow *pParent = NULL, const CBofRect &xRect = CBofRect(), BOOL bActivated = FALSE);
	virtual ~SBBasePda();

	// jwl 10.31.96 allow PDA mode to be set by script
	static VOID SetPDAMode(PDAMODE ePDAMode) { m_ePdaMode = ePDAMode; }
	static PDAMODE GetPDAMode() { return m_ePdaMode; }
	BOOL IsActivated() { return (m_bActivating) ? !m_bActivated : m_bActivated; }
	BOOL IsActivating() { return (m_bActivating); }

	virtual ERROR_CODE AttachActiveObjects();
	virtual ERROR_CODE DetachActiveObjects();

	// jwl 11.05.96 sync starting options
	VOID SynchronizePDAState();

	BOOL Deactivate() {
		if (!m_bActivating) {

			m_bActivating = m_nNumMoves;
			m_bActivated = FALSE;
		}
		m_ePDAPos = PDADOWN;
		SetPDAState();
		return TRUE;
	}

	BOOL Activate() {
		if (!m_bActivating) {
			m_bActivating = m_nNumMoves;
			m_bActivated = TRUE;
		}

		m_ePDAPos = PDAUP;
		SetPDAState();
		return TRUE;
	}

	virtual BOOL ShowInventory(); // Show the inventory
	virtual BOOL HideInventory(); // Hide the inventory

	virtual BOOL ShowMovie();     // Show the movie window
	virtual BOOL HideMovie();     // Hide the movie window
	BOOL SetMovie(CBofString &s); // Set the movie
	VOID StopMovie(BOOL);         // Stop any asynch movies playing

	// jwl 12.30.96
	VOID SetDeactivate(BOOL b = FALSE) { m_bDeactivate = b; }
	BOOL GetDeactivate() { return m_bDeactivate; }

	virtual BOOL ShowMap(); // Show the Map
	virtual BOOL HideMap(); // Hide the Map

	virtual BOOL Zoom() { return TRUE; } // Zoom the current display

	virtual BOOL ShowLog();

	virtual BOOL MsgLight();

	virtual BOOL HideCurDisplay();    // Hide the current display
	virtual BOOL RestoreCurDisplay(); // Restore cur display, paired with hide

	static void *fPdaButtonHandler(int /* nRefId */, void *pvInfo);

	VOID SetPDAState();
	VOID GetPDAState();

	VOID SetZoomed(BOOL b = FALSE) { m_bZoomed = b; }
	BOOL GetZoomed() { return m_bZoomed; }

	INT GetProperCursor(const CBofPoint &xPoint, CBofRect &pdaRect);

	// jwl 11.26.96 allow for getting the background rect.
	CBofRect GetViewRect();
};

#define GLOBALTOLOCAL 0
#define LOCALTOGLOBAL 1

} // namespace Bagel

#endif
