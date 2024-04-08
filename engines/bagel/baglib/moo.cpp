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

CBagCharacterObject *CBagMoo::m_pMovie;
PDAMODE CBagMoo::m_eSavePDAMode;
PDAPOS CBagMoo::m_eSavePDAPos;

void CBagMoo::initialize() {
	m_pMovie = nullptr;
	m_eSavePDAMode = NOMODE;
	m_eSavePDAPos = UNINITIALIZED;
}

ERROR_CODE CBagMoo::Update(CBofBitmap *pBmp, CBofPoint /*pt*/, CBofRect *pSrcRect, int nMaskColor) {
	ERROR_CODE    ec = ERR_NONE;

	if (m_pMovie) {
		// Update the movie, assume only unzoomed pda right now
		CBofPoint cPos(116, 61);
		ec = m_pMovie->Update(pBmp, cPos, pSrcRect, nMaskColor);

		// If we're done or we encountered an error, then roll over and die.
		if (ec != ERR_NONE || m_pMovie->IsModalDone()) {
			StopMovie(TRUE);
		}
	}

	return ec;
}

CBagMoo::~CBagMoo() {
	// Can possibly have a movie left over.
	if (m_pMovie) {
		delete m_pMovie;
		m_pMovie = nullptr;
	}
}

ERROR_CODE CBagMoo::SetPDAMovie(CBofString &s) {
	ERROR_CODE    ec = ERR_NONE;

	// Should never happen, but just make sure.
	if (m_pMovie) {
		delete m_pMovie;
		m_pMovie = nullptr;
	}

	// Get a new movie object
	m_pMovie = new CBagCharacterObject();
	Assert(m_pMovie != nullptr);

	if (m_pMovie) {
		m_pMovie->SetFileName(s);

		// Attach this bad baby...
		ec = m_pMovie->Attach();
		if (ec == ERR_NONE) {
			m_pMovie->SetModal(FALSE);
			m_pMovie->SetNumOfLoops(1);
		}
	}

	return ec;
}

void CBagMoo::StopMovie(bool bResetPDA) {
	CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");

	if (bResetPDA) {
		if (pPDA) {
			// Assume this marks the end of the movie.
			switch (m_eSavePDAMode) {
			case MAPMODE:
				pPDA->ShowMap();
				break;

			case INVMODE:
				pPDA->ShowInventory();
				break;

			case LOGMODE:
				pPDA->ShowLog();
				break;

			case NOMODE:
			default:
				pPDA->HideMovie();
				break;
			}
		}
	}

	if (m_pMovie) {
		m_pMovie->Detach();
		delete m_pMovie;
		m_pMovie = nullptr;
	}

	// If we're done playing, then deactivate the PDA.
	if (pPDA && pPDA->GetDeactivate() == TRUE) {
		if (pPDA->IsActivated()) {
			pPDA->Deactivate();
		}
	}
}

} // namespace Bagel
