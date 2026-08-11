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

#include "bagel/spacebar/baglib/moo.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/pan_window.h"

namespace Bagel {
namespace SpaceBar {

CBagCharacterObject *CBagMoo::_pMovie;
PdaMode CBagMoo::_eSavePDAMode;
PdaPos CBagMoo::_eSavePDAPos;

void CBagMoo::initialize() {
	_pMovie = nullptr;
	_eSavePDAMode = PDA_NO_MODE;
	_eSavePDAPos = PDA_POS_UNINITIALIZED;
}

ErrorCode CBagMoo::update(CBofBitmap *pBmp, CBofPoint /*pt*/, CBofRect *pSrcRect, int nMaskColor) {
	ErrorCode errorCode = ERR_NONE;

	if (_pMovie) {
		// Update the movie, assume only unzoomed pda right now
		const CBofPoint cPos(116, 61);
		errorCode = _pMovie->update(pBmp, cPos, pSrcRect, nMaskColor);

		// If we're done or we encountered an error, then roll over and die.
		if (errorCode != ERR_NONE || _pMovie->isModalDone()) {
			stopMovie(true);
		}
	}

	return errorCode;
}

CBagMoo::~CBagMoo() {
	// Can possibly have a movie left over.
	delete _pMovie;
	_pMovie = nullptr;
}

ErrorCode CBagMoo::setPDAMovie(CBofString &s) {
	// Should never happen, but just make sure.
	delete _pMovie;
	_pMovie = nullptr;

	// Get a new movie object
	_pMovie = new CBagCharacterObject();

	_pMovie->setFileName(s);

	// Attach this bad baby...
	const ErrorCode errorCode = _pMovie->attach();
	if (errorCode == ERR_NONE) {
		_pMovie->setModal(false);
		_pMovie->setNumOfLoops(1);
	}

	return errorCode;
}

void CBagMoo::stopMovie(bool bResetPDA) {
	CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice("BPDA_WLD");

	if (bResetPDA) {
		if (pPDA) {
			// Assume this marks the end of the movie.
			switch (_eSavePDAMode) {
			case PDA_MAP_MODE:
				pPDA->showMap();
				break;

			case PDA_INV_MODE:
				pPDA->showInventory();
				break;

			case PDA_LOG_MODE:
				pPDA->showLog();
				break;

			case PDA_NO_MODE:
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

} // namespace SpaceBar
} // namespace Bagel
