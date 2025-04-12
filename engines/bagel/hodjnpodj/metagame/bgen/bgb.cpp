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

#include "common/file.h"
#include "bagel/hodjnpodj/metagame/bgen/bgb.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

CBgbMgr *gpBgbMgr;

// age
static uint32 glOldest = 0xFFFFFFFF;
static uint32 glAge = 0xFFFFFFFF;

CBgbMgr::CBgbMgr() {
	gpBgbMgr = this;
}

CBgbMgr::~CBgbMgr(void) {
	ReleaseResources();

	delete m_xpGamePalette;
	delete m_xpDc;
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
	// can't access a nullptr pointer
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

bool CBgbMgr::InitBitmapObject(CBgbObject *pBgbObject, XPSTR pszFileName) {
	CLList *pList;

	// can't access a nullptr pointer
	assert(pBgbObject != nullptr);

	// why would this object already be initialized?
	assert(pBgbObject->m_bInit == false);

	// if this object has not already been initialized
	//
	if (!pBgbObject->m_bInit) {

		// if not yet on chain to be freed
		//
		if (!pBgbObject->m_bChained) {
			pBgbObject->m_xpcNext = m_lpBgbChain;
			pBgbObject->m_xpcPrev = nullptr;

			if (m_lpBgbChain != nullptr)
				m_lpBgbChain->m_xpcPrev = pBgbObject;

			m_lpBgbChain = pBgbObject;
			pBgbObject->m_bChained = true;
		}

		// don't overwrite the filename buffer
		assert(strlen(pszFileName) < MAX_FNAME_LENGTH);

		// init this objects bitmap filename (must be done before cache)
		Common::strcpy_s(pBgbObject->m_szFileName, pszFileName);

		// object must be Initialized before any Caching
		pBgbObject->m_bInit = true;

		// don't yet need a ClearBackground()
		pBgbObject->m_bCleared = true;

		switch (pBgbObject->m_iBgbType) {

			// object is a CSprite
			//
		case BGBT_SPRITE:

			// if this is a special animation
			//
			if (pBgbObject->m_bAnimated && pBgbObject->m_bSpecial) {

				// calculate how many paints are required for this
				// animation sequence.
				//
				assert(pBgbObject->m_nCels > 0);
				pBgbObject->m_nRepeat *= (uint)pBgbObject->m_nCels;
				pBgbObject->m_nFreqCount = pBgbObject->m_nFreq - 1;

				if ((pList = new CLList((CObject *)pBgbObject)) != nullptr) {

					// add to special effects list
					if (m_pFXList == nullptr) {

						m_pFXList = pList;
						m_bAnimationsPaused = false;

					} else {
						m_pFXList->Insert(pList);
					}
				}
				assert(pList != nullptr);

			} else {
				pBgbObject->m_bLocked = true;
			}

			break;

			// object is a CBitmap
			//
		case BGBT_DIB:

			//
			// Nothing else to initialize if this object is a CBitmap
			//
			break;

			// Invalid ID
			//
		default:
			assert(0);
			break;
		}

		// Let 1st object init the game palette
		//
		if (m_xpGamePalette == nullptr) {
			CacheLoadObject(pBgbObject);
		}
	}

	return false;
}

VOID CBgbMgr::CacheLoadObject(CBgbObject *pBgbObject) {
	Sprite *pSprite;
	bool bSuccess = true;

	// Can't access a null pointer
	assert(pBgbObject != nullptr);

	// Object must have already been Initialized
	assert(pBgbObject->m_bInit);

	// Assign age to this object every time it's accessed
	pBgbObject->m_lAge = glAge--;

	// If it's not already loaded
	if (pBgbObject->m_bLoaded) {
		if (pBgbObject->m_bLocked)
			CacheOptimize(0);
		else {
			pBgbObject->m_bLocked = true;
			CacheOptimize(0);
			pBgbObject->m_bLocked = false;
		}
	} else {
		// Maintain default free
		CacheOptimize(0);

		// Make sure this file-bitmap exists
		assert(Common::File::exists(pBgbObject->m_szFileName));

		// Object must start empty
		assert(pBgbObject->m_pObject == nullptr);

		// Get a DC if we don't already have one
		if (m_xpDc == nullptr) {
			ReInitDc();
		}
		assert(m_xpDc != nullptr);

		// 1st bitmap loaded installs m_pGamePalette
		//
		if (m_xpGamePalette == nullptr) {
			GfxSurface *pBmp;

			pBmp = FetchBitmap(m_xpDc, &m_xpGamePalette, pBgbObject->m_szFileName);
			delete pBmp;
		}

		// which object type
		switch (pBgbObject->m_iBgbType) {
		// Object is a Sprite
		case BGBT_SPRITE:
			// palette must be valid
			assert(m_xpGamePalette != nullptr);

			// alloc sprite and test
// TODO: Fix null sprite minigame pointer
			if ((pSprite = new Sprite(nullptr)) != nullptr) {
				// set its palette to be the same as the game's
				pSprite->SharePalette(m_xpGamePalette);

				if (pBgbObject->m_bAnimated) {
					pSprite->loadCels(pBgbObject->m_szFileName, pBgbObject->m_nCels);
					if (!pBgbObject->m_bSpecial)
						pSprite->SetAnimated(false);

				} else {
					pSprite->loadBitmap(pBgbObject->m_szFileName);
				}

				assert(bSuccess);
				if (!bSuccess) {
					delete pSprite;
					pBgbObject->m_pObject = nullptr;
					break;
				}

				// it is a masked image (white/transparent background)
				// keep its bitmap inside a device context
				pSprite->SetMasked(pBgbObject->m_bMasked);

				pSprite->SetZOrder(pBgbObject->m_nZOrder);

				// it will move around the screen
				pSprite->SetMobile(true);

				// link into sprite chain
				pSprite->linkSprite();
			}

			pBgbObject->m_pObject = (CObject *)pSprite;
			break;


		// Object is a bitmap
		case BGBT_DIB:
			pBgbObject->m_pObject = (CObject *)FetchBitmap(m_xpDc, (m_xpGamePalette == nullptr ? &m_xpGamePalette : nullptr), pBgbObject->m_szFileName);
			assert(pBgbObject->m_pObject != nullptr);
			break;


		// Invalid ID
		default:
			assert(0);
			break;
		}

		// make sure we were able to load this object
		assert(pBgbObject->m_pObject != nullptr);

		if (pBgbObject->m_pObject == nullptr)
			pBgbObject->m_bLoaded = false;
		else
			pBgbObject->m_bLoaded = true;
	}
}

bool CBgbMgr::ReInitDc() {
	m_xpDc = new CDC();
	//InitDc(m_xpcView, m_xpBsuSet);

	return false;
}

void CBgbMgr::CacheOptimize(uint32 lReqSpace) {
#ifdef TODO
	CBgbObject *pBgbObject;
	uint32 lAvgAge;
	int nObjects;
	uint32 lSpace;

	// There are no objects to flush
	assert(m_lpBgbChain != nullptr);

	if (lReqSpace < lpMetaGame->m_dwFreeSpaceMargin)
		lSpace = lpMetaGame->m_dwFreeSpaceMargin;
	else
		lSpace = lReqSpace;

	while (CheckLowMemory(lSpace)) {
		GlobalCompact(lSpace);
		LocalCompact(4000);

		nObjects = 0;

		lAvgAge = ((glOldest - glAge) / 2) + glAge;
		glOldest = lAvgAge;

		pBgbObject = m_lpBgbChain;
		while (pBgbObject != nullptr) {

			//
			// remove objects that are too old
			//
			if (pBgbObject->m_bLoaded && !pBgbObject->m_bLocked) {

				nObjects++;
				if (pBgbObject->m_lAge >= lAvgAge) {
					CacheReleaseObject(pBgbObject);
				}
			}

			pBgbObject = pBgbObject->m_xpcNext;
		}

		// if there are no objects loaded then we really are out of memory
		if (nObjects == 0)
			break;
	}
#endif
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
