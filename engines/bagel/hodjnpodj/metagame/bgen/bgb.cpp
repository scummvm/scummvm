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

#include "bagel/hodjnpodj/metagame/bgen/bgb.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

CBgbMgr *gpBgbMgr;

CBgbMgr::CBgbMgr() {
	gpBgbMgr = this;
}

CBgbMgr::~CBgbMgr(void) {
	ReleaseResources();

	delete m_xpGamePalette;
}

bool CBgbMgr::ReleaseResources() {
	int iError = 0;			// error code
	CBgbObject *pBgbObject;	// objects to be freed

	// stop all animations
	m_bAnimationsPaused = true;

	// flush the special effects list
	if (m_pFXList) {
		m_pFXList->FlushList();
		delete m_pFXList;
		m_pFXList = nullptr;
	}

	while ((pBgbObject = m_lpBgbChain) != nullptr) {
		m_lpBgbChain = pBgbObject->m_xpcNext;
		pBgbObject->m_xpcNext = nullptr;     // out of habit
		ClearBitmapObject(pBgbObject);    // free resources

		if (!pBgbObject->m_bNoDelete)     // if on the heap
			delete pBgbObject;
	}

	return iError != 0;
}

bool CBgbMgr::ClearBitmapObject(CBgbObject *pBgbObject) {
	pBgbObject->m_bLocked = false;
	CacheReleaseObject(pBgbObject);

	// object no longer initialized
	pBgbObject->m_bInit = false;

	return false;
}

void CBgbMgr::CacheReleaseObject(CBgbObject *pBgbObject) {
	// can't access a null pointer
	assert(pBgbObject != nullptr);

	// object must have already been Initialized
	assert(pBgbObject->m_bInit);

	if (pBgbObject->m_bLoaded && !pBgbObject->m_bLocked) {
		// make sure the object is valid
		assert(pBgbObject->m_pObject != nullptr);

		//ErrorLog("DEBUG.LOG", "Releasing %s", pBgbObject->m_szFileName);

		switch (pBgbObject->m_iBgbType) {
		// Object is a sprite
		//
		case BGBT_SPRITE:
			if (pBgbObject->m_pObject != nullptr) {
				// remove from sprite chain
				Sprite *spr = dynamic_cast<Sprite *>(pBgbObject->m_pObject);
				assert(spr);
				spr->unlinkSprite();

				// free it
				delete spr;

				// no more pointer
				pBgbObject->m_pObject = nullptr;
			}
			break;

		// Object is a bitmap
		//
		case BGBT_DIB:
			if (pBgbObject->m_pObject != nullptr) {
				delete pBgbObject->m_pObject;
				pBgbObject->m_pObject = nullptr;
			}
			break;

		// Invalid ID
		//
		default:
			assert(0);
			break;
		}

		// this object is now loaded
		pBgbObject->m_bLoaded = false;
	}
}

/*------------------------------------------------------------------------*/

CBgbObject::CBgbObject() {
	m_bChained = false;
	m_bNoDelete = false;
	m_bMasked = false;
	m_bInit = false;
	m_bCleared = false;
	m_bVisible = false;
	m_bEdge = false;
	m_bLoaded = false;
	m_bAnimated = false;
	m_bSpecial = false;
	m_bLocked = false;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
