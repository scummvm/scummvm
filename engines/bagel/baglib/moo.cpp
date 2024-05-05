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

#include "bagel/baglib/moo.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"

namespace Bagel {

CBagCharacterObject *CBagMoo::_pMovie;
PDAMODE CBagMoo::_eSavePDAMode;
PDAPOS CBagMoo::_eSavePDAPos;

void CBagMoo::initialize() {
	_pMovie = nullptr;
	_eSavePDAMode = NOMODE;
	_eSavePDAPos = UNINITIALIZED;
}

ErrorCode CBagMoo::update(CBofBitmap *pBmp, CBofPoint /*pt*/, CBofRect *pSrcRect, int nMaskColor) {
	ErrorCode    ec = ERR_NONE;

	if (_pMovie) {
		// Update the movie, assume only unzoomed pda right now
		CBofPoint cPos(116, 61);
		ec = _pMovie->update(pBmp, cPos, pSrcRect, nMaskColor);

		// If we're done or we encountered an error, then roll over and die.
		if (ec != ERR_NONE || _pMovie->isModalDone()) {
			stopMovie(true);
		}
	}

	return ec;
}

CBagMoo::~CBagMoo() {
	// Can possibly have a movie left over.
	if (_pMovie) {
		delete _pMovie;
		_pMovie = nullptr;
	}
}

ErrorCode CBagMoo::setPDAMovie(CBofString &s) {
	ErrorCode ec = ERR_NONE;

	// Should never happen, but just make sure.
	if (_pMovie) {
		delete _pMovie;
		_pMovie = nullptr;
	}

	// Get a new movie object
	_pMovie = new CBagCharacterObject();
	assert(_pMovie != nullptr);

	if (_pMovie) {
		_pMovie->setFileName(s);

		// Attach this bad baby...
		ec = _pMovie->attach();
		if (ec == ERR_NONE) {
			_pMovie->setModal(false);
			_pMovie->setNumOfLoops(1);
		}
	}

	return ec;
}

void CBagMoo::stopMovie(bool bResetPDA) {
	CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice("BPDA_WLD");

	if (bResetPDA) {
		if (pPDA) {
			// Assume this marks the end of the movie.
			switch (_eSavePDAMode) {
			case MAPMODE:
				pPDA->showMap();
				break;

			case INVMODE:
				pPDA->showInventory();
				break;

			case LOGMODE:
				pPDA->showLog();
				break;

			case NOMODE:
			default:
				pPDA->hideMovie();
				break;
			}
		}
	}

	if (_pMovie) {
		_pMovie->detach();
		delete _pMovie;
		_pMovie = nullptr;
	}

	// If we're done playing, then deactivate the PDA.
	if (pPDA && pPDA->getDeactivate() == true && pPDA->isActivated()) {
		pPDA->deactivate();
	}
}

} // namespace Bagel
