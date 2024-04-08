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

#include "bagel/baglib/inv.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/movie_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/pda.h"

namespace Bagel {

#define FIRSSTASHMOV        "FIRST_STASH_MOVIE"

bool CBagInv::m_bFirstStash;

//
//	ActivateLocalObject
//	This is called when a object is inserted into the inventory
//  The wand on the PDA should animate
//

ERROR_CODE CBagInv::ActivateLocalObject(const CBofString &sName) {
	CBagCharacterObject *pWand = nullptr;
	CBagStorageDev *pPda = nullptr;
	CBagPDA *pPDAReally = nullptr;
	SBZoomPda *pZPDA = nullptr;
	bool bZoomed = FALSE;

	pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");

	// Don't do any wand animation if we are zoomed.
	pZPDA = (SBZoomPda *)SDEVMNGR->GetStorageDevice("BPDAZ_WLD");
	Assert(pZPDA != nullptr);
	bZoomed = (pZPDA ? pZPDA->GetZoomed() : FALSE);

	if (pPda && bZoomed == FALSE) {
		pWand = (CBagCharacterObject *)pPda->GetObject("WANDANIM");
		pPDAReally = (CBagPDA *)pPda;

		if (pWand) {
			pWand->SetPlaybackSpeed(1);
			pWand->SetNumOfLoops(1);

			// Let our character anim know that this is our PDA Wand,
			// it will need to let the pda update code know that
			// it needs to get redrawn.
			CBagCharacterObject::SetPDAWand(pWand);
			pPDAReally->SetDirty(TRUE);
		}
	}

	if (bZoomed == FALSE) {
		CBagVar *pVar;

		// Check to see if we are in the bar
		pVar = VARMNGR->GetVariable("INBAR");
		if (pVar != nullptr) {
			pVar = VARMNGR->GetVariable("FIRST_STASH");

			// If this is our first stash, play the smacker associated  with it.
			if ((pVar != nullptr) && (pVar->GetNumValue() == 0)) {
				CBagMovieObject *pMovie = (CBagMovieObject *)GetObject(FIRSSTASHMOV);
				if (pMovie) {
					if (pMovie->IsAttached() == FALSE) {
						pMovie->Attach();
						pMovie->SetVisible();
					}

					pMovie->RunObject();
				}

				pVar->SetValue(1);
			}
		}
	}

	return CBagStorageDevBmp::ActivateLocalObject(sName);
}

ERROR_CODE CBagInv::DeactivateLocalObject(const CBofString &sName) {
	CBagCharacterObject *pWand = nullptr;
	CBagStorageDev *pPda = nullptr;
	SBZoomPda *pZPDA = nullptr;
	bool bZoomed = FALSE;

	pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");

	// Don't do any wand animation if we are zoomed.
	pZPDA = (SBZoomPda *)SDEVMNGR->GetStorageDevice("BPDAZ_WLD");
	Assert(pZPDA != nullptr);
	bZoomed = (pZPDA ? pZPDA->GetZoomed() : FALSE);

	if (pPda && bZoomed == FALSE) {
		pWand = (CBagCharacterObject *)pPda->GetObject("WANDANIM");
		if (pWand) {
			pWand->SetPlaybackSpeed(-1);
			pWand->SetNumOfLoops(1);

			// Let it know our ending frame is 0, not 1.
			pWand->SetEndFrame(0);
		}
	}

	return CBagStorageDevBmp::DeactivateLocalObject(sName);;
}

} // namespace Bagel
