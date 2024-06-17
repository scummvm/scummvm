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
#include "bagel/baglib/pda.h"

namespace Bagel {

#define FIRS_STASH_MOV        "FIRST_STASH_MOVIE"

bool CBagInv::_firstStashFl;

//
//	This is called when a object is inserted into the inventory
//  The wand on the PDA should animate
//

ErrorCode CBagInv::activateLocalObject(const CBofString &objectName) {

	CBagStorageDev *pdaSDev = g_SDevManager->getStorageDevice("BPDA_WLD");

	// Don't do any wand animation if we are zoomed.
	SBZoomPda *zoomPda = (SBZoomPda *)g_SDevManager->getStorageDevice("BPDAZ_WLD");
	assert(zoomPda != nullptr);
	const bool zoomedFl = (zoomPda ? zoomPda->getZoomed() : false);

	if (pdaSDev && zoomedFl == false) {
		CBagCharacterObject *wand = (CBagCharacterObject *)pdaSDev->getObject("WANDANIM");
		CBagPDA *pda = (CBagPDA *)pdaSDev;

		if (wand) {
			wand->setPlaybackSpeed(1);
			wand->setNumOfLoops(1);

			// Let our character anim know that this is our PDA Wand,
			// it will need to let the pda update code know that
			// it needs to get redrawn.
			CBagCharacterObject::setPdaWand(wand);
			pda->setDirty(true);
		}
	}

	if (zoomedFl == false) {
		// Check to see if we are in the bar
		CBagVar *var = g_VarManager->getVariable("INBAR");
		if (var != nullptr) {
			var = g_VarManager->getVariable("FIRST_STASH");

			// If this is our first stash, play the smacker associated with it.
			if ((var != nullptr) && (var->getNumValue() == 0)) {
				CBagMovieObject *pMovie = (CBagMovieObject *)getObject(FIRS_STASH_MOV);
				if (pMovie) {
					if (pMovie->isAttached() == false) {
						pMovie->attach();
						pMovie->setVisible();
					}

					pMovie->runObject();
				}

				var->setValue(1);
			}
		}
	}

	return CBagStorageDevBmp::activateLocalObject(objectName);
}

/**
 * This is called when a object is removed from the inventory
 * The wand on the PDA should animate
 */
ErrorCode CBagInv::deactivateLocalObject(const CBofString &objectName) {
	CBagStorageDev *pdaSDev = g_SDevManager->getStorageDevice("BPDA_WLD");

	// Don't do any wand animation if we are zoomed.
	SBZoomPda *zoomPDA = (SBZoomPda *)g_SDevManager->getStorageDevice("BPDAZ_WLD");
	assert(zoomPDA != nullptr);
	const bool zoomedFl = (zoomPDA ? zoomPDA->getZoomed() : false);

	if (pdaSDev && zoomedFl == false) {
		CBagCharacterObject *wand = (CBagCharacterObject *)pdaSDev->getObject("WANDANIM");
		if (wand) {
			wand->setPlaybackSpeed(-1);
			wand->setNumOfLoops(1);

			// Let it know our ending frame is 0, not 1.
			wand->setEndFrame(0);
		}
	}

	return CBagStorageDevBmp::deactivateLocalObject(objectName);;
}

} // namespace Bagel
