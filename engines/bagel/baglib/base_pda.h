
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

#include "bagel/baglib/storage_dev_bmp.h"

namespace Bagel {

#define PDA_MAP (100)
#define PDA_STASH (101)
#define PDA_ZOOM (102)
#define PDA_SYSTEM (103)
#define PDA_LOG (104)
#define PDA_OFF (105)
#define PDA_MSGLIGHT (110)
#define PDA_ZOOMFLASH "ZOOMFLASH"

enum PdaMode {
	PDA_NO_MODE,
	PDA_MAP_MODE,
	PDA_INV_MODE,
	PDA_LOG_MODE,
	PDA_MOO_MODE
};
enum PdaPos {
	PDA_POS_UNINITIALIZED,
	PDA_UP,
	PDA_DOWN
};

class SBBasePda {
protected:
	// All PDA's in the game should share the same mode
	static PdaMode _pdaMode;
	static PdaPos _pdaPos;
	int _numMoves;
	int _activating;
	bool _activated;
	int _moveDist;
	CBagStorageDevBmp *_mooWnd;     // Pointer to the PDAMOVIE object
	CBagStorageDevBmp *_invWnd;     // Pointer to the inventory object
	CBagStorageDevBmp *_mapWnd;     // Pointer to the Map object
	CBagStorageDevBmp *_logWnd;     // Pointer to the Map object
	CBagStorageDevBmp *_curDisplay; // Pointer to the object currently displayed in PDA
	CBofWindow *_parent;
	bool _zoomedFl;
	bool _deactivateFl; // should we deactivate when done w/movie?

	// hold this info for restore method
	CBagStorageDevBmp *_holdDisplay;
	static PdaMode _holdMode;

public:
	SBBasePda(CBofWindow *parent = nullptr, const CBofRect &rect = CBofRect(), bool activated = false);
	virtual ~SBBasePda();
	static void initialize();

	/**
	 * Allows PDA mode to be set by script
	 */
	static void setPdaMode(PdaMode pdaMode) {
		_pdaMode = pdaMode;
	}
	static PdaMode getPdaMode() {
		return _pdaMode;
	}
	bool isActivated() const {
		return _activating ? !_activated : _activated;
	}
	bool isActivating() const {
		return _activating;
	}

	virtual ErrorCode attachActiveObjects();
	virtual ErrorCode detachActiveObjects();

	/**
	 * Sync starting options
	 */
	void synchronizePdaState();

	void activate();
	void deactivate();

	/**
	 * Show the inventory
	 */
	virtual bool showInventory();

	/**
	 * Hide the inventory
	 */
	virtual bool hideInventory();

	/**
	 * Show the movie window
	 */
	virtual bool showMovie();

	/**
	 * Hide the movie window
	 */
	virtual bool hideMovie();

	/**
	 * Set the movie to play
	 * @param movieName   Movie filename
	 * @return            Success/failure
	 */
	bool setMovie(CBofString &movieName) const;

	/**
	 * Stops any playing movie
	 */
	void stopMovie(bool bResetPDA) const;

	void setDeactivate(bool b) {
		_deactivateFl = b;
	}
	
	bool getDeactivate() const {
		return _deactivateFl;
	}

	/**
	 * Show the map
	 */
	virtual bool showMap();

	/**
	 * Hide the map
	 */
	virtual bool hideMap();

	/**
	 * Zoom the current display
	 */
	virtual bool zoom() {
		return true;
	}

	virtual bool showLog();

	virtual bool msgLight();

	/**
	 * Hide the current display and reset the _curDisplay to nullptr
	 * @return      Success/failure
	 */
	virtual bool hideCurDisplay();

	/**
	 * Hide the current display and reset the _curDisplay to nullptr
	 * @return      Success/Failure
	 */
	virtual bool restoreCurDisplay();

	static void *pdaButtonHandler(int refId, void *info);

	void setPdaState();
	void getPdaState();

	void setZoomed(bool newVal) {
		_zoomedFl = newVal;
	}
	bool getZoomed() const {
		return _zoomedFl;
	}

	int getProperCursor(const CBofPoint &pos, CBofRect &pdaRect) const;

	/**
	 * Returns the background rect
	 */
	CBofRect getViewRect() const;
};

} // namespace Bagel

#endif
