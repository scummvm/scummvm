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

#include "bagel/hodjnpodj/metagame/boardgame/gtl_data.h"
#include "bagel/hodjnpodj/metagame/boardgame/gtl_frame.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

CMap::CMap() {
	m_bPositionDetermined = false;
	m_bSprite = false;
	m_bPalette = false;
	m_bOverlay = false;
	m_bMasked = false;
	m_bMetaGame = false;
	m_bRelocatable = false;
	m_bPositionSpecified = false;
	m_bSpecialPaint = false;
}

/*------------------------------------------------------------------------*/

CNode::CNode() {
	memset(&m_szLabel, 0, sizeof(char) * MAX_LABEL_LENGTH);
	memset(&m_iLinks, 0, sizeof(int) * MAX_LINKS_PER_NODE);
	m_iLocationCode = 0;
	m_iSector = 0;
	m_iBitmap = 0;
	m_iX = 0;
	m_iY = 0;
	m_iNumLinks = 0;
	m_bDeleted = false;
	m_bSelected = false;
	m_bRelative = false;
	m_bRelocatable = false;
	m_bWgtSpec = false;
	m_bSenSpec = false;
	m_bMenu = false;
	m_iWeight = 0;
	m_iSensitivity = 0;
}

/*------------------------------------------------------------------------*/

CGtlData::CGtlData() {
	TimeDate *td[2] = { &m_stAcceptClickActive, &m_stLDownTime };

	for (TimeDate *d : td) {
		d->tm_sec = 0;
		d->tm_min = 0;
		d->tm_hour = 0;
		d->tm_mday = 0;
		d->tm_mon = 0;
		d->tm_year = 0;
		d->tm_wday = 0;
	}

	m_cBbtMgr.m_lpBgbMgr = &m_cBgbMgr;
	m_cMiniButton.m_iBbtId = BUTTON_MINIMAP;
	m_cInvButton.m_iBbtId = BUTTON_INVENTORY;
	m_cScrollButton.m_iBbtId = BUTTON_SCROLL;

	m_lpMaps = new CMap[MAX_BITMAPS]; // allocate bitmap array
	m_lpNodes = new CNode[MAX_NODES];     // allocate node array
	SetColors();       // set color fields

	CreateOffScreenBmp();
}

CGtlData::~CGtlData() {
	CXodj *xpXodj, *xpXodjNext;

	DeleteOffScreenBmp();

	delete[] m_lpMaps;
	delete[] m_lpNodes;

	// Delete Xodj chain
	for (xpXodj = m_xpXodjChain; xpXodj; xpXodj = xpXodjNext) {
		xpXodjNext = xpXodj->m_xpXodjNext;
		delete xpXodj;
	}
}

void CGtlData::CreateOffScreenBmp() {
	CGtlApp *xpGtlApp = (CGtlApp *)AfxGetApp();

	if (m_pOffScreenBmp == nullptr) {
		m_pOffScreenBmp = new GfxSurface();
		assert(m_pOffScreenBmp);

		m_pOffScreenBmp->create(xpGtlApp->m_iWidth, xpGtlApp->m_iHeight);
	}
}

void CGtlData::DeleteOffScreenBmp() {
	delete m_pOffScreenBmp;
	m_pOffScreenBmp = nullptr;
}

bool CGtlData::SetColors() {
	m_cNodePenColor = m_cNodeBrushColor = RGBCOLOR_RED;
	m_cSelNodePenColor = RGBCOLOR_MAGENTA;
	m_cSelNodeBrushColor = RGBCOLOR_GREEN;

	m_cNodeSensitivityColor = m_cNodePenColor;
	m_cNodePassThruColor = RGBCOLOR_WHITE;
	m_cNodeHeavyColor = RGBCOLOR_BLACK;

	m_cLinkPenColor = RGBCOLOR_BLUE;
	m_cSelLinkPenColor = RGBCOLOR_YELLOW;
	m_cBackColor = RGBCOLOR_GREEN;

	return false;
}

bool CGtlData::AdjustToView(CGtlView *xpGtlView) {
	int iError = 0;            // error code
	CBgbObject *lpcBgbObject;
	char szPath[200];
	CMap *lpMap;
	CNode *lpNode;
	int iK; // , iL = 0;            // loop variables
	bool bNewBitmap = false;
	CLocTable *xpLocTable = CMgStatic::cLocTable, *xpLocEntry;

	for (iK = 0; iK < m_iMaps; ++iK) {

		lpMap = m_lpMaps + iK;

		if (m_bMetaGame || !lpMap->m_bMetaGame) {
			lpcBgbObject = lpMap->m_lpcBgbObject;

			// the object should have already been allocted in GTLCPL
			assert(lpcBgbObject != nullptr);

			if (!lpcBgbObject->m_bInit) {

				bNewBitmap = true;

				lpcBgbObject->m_iBgbType = lpMap->m_bSprite ? BGBT_SPRITE : BGBT_DIB;
				lpcBgbObject->m_crPosition.m_bRelocatable = (lpMap->m_bRelocatable != 0);

				// next load in the bitmap segment and test
				Common::strcpy_s(szPath, m_szBmpDirectory);
				Common::strcat_s(szPath, lpMap->m_szFilename);

				m_cBgbMgr.InitBitmapObject(lpcBgbObject, szPath);
			}
		}
	}

	NormalizeData(xpGtlView);  // normalize data so point (0,0)
	// is in upper LH corner

	if (xpGtlView) {
		if (!xpGtlView->m_bBsuInit && !xpGtlView->m_cViewBsuSet.InitWndBsuSet(xpGtlView))
			xpGtlView->m_bBsuInit = true;

		xpGtlView->m_cViewBsuSet.PrepareWndBsuSet(CSize(m_iSizeX, m_iSizeY),
			Common::Rect(0, m_iMargin, 0, 0));
	}

	for (iK = 0; iK < m_iNodes; ++iK) {
		if (!(lpNode = m_lpNodes + iK)->m_bDeleted) {

			if (scumm_stricmp(lpNode->m_szLabel, "menu") == 0)
				lpNode->m_bMenu = true;
#if 0
			// find this nodes sector
			//
			lpNode->m_iSector = MG_SECTOR_ANY;
			pSectorEntry = CMgStatic::cSectorTable;
			while (pSectorEntry->m_iSectorCode != 0) {
				if (scumm_stricmp(pSectorEntry->m_lpszLabel, lpNode->m_szSector) == 0) {
					lpNode->m_iSector = pSectorEntry->m_iSectorCode;
					break;
				}
				pSectorEntry++;
			}
#endif
			for (xpLocEntry = xpLocTable; xpLocEntry->m_iLocCode && scumm_stricmp(lpNode->m_szLabel, xpLocEntry->m_lpszLabel); ++xpLocEntry)
				;       // null loop body

			// if this location is in table
			//
			if (xpLocEntry->m_iLocCode)
				lpNode->m_iLocationCode = xpLocEntry->m_iLocCode;

			if (lpNode->m_iSensitivity == 0 && !lpNode->m_bSenSpec) {
				if (lpNode->m_bMenu)
					lpNode->m_iSensitivity = NODERADIUS;
				else if (lpNode->m_iWeight > 0 && lpNode->m_bRelocatable && lpNode->m_iNumLinks > 0)
					lpNode->m_iSensitivity = NODESENSITIVITY;
			}
		}
	}

	// determine current positions of sprites and movable overlays
	for (iK = 0; iK < m_iMaps; ++iK) {
		lpMap = m_lpMaps + iK;

		if ((m_bMetaGame || !lpMap->m_bMetaGame) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != nullptr)) {

			// Adjust sprite positions in case anything's moved
			// Note: May have to add code here to erase sprite background
			//  under some circumstances.
			if (lpMap->m_bSprite && lpMap->m_iRelationType == KT_NODE) {

				lpNode = m_lpNodes + lpMap->m_iRelation;

				// point to relative node
				m_cBgbMgr.SetPosition(lpcBgbObject, NodeToPoint(lpNode));
			}


			if (scumm_stricmp(lpMap->m_szLabel, "f00") >= 0 && scumm_stricmp(lpMap->m_szLabel, "f24") <= 0) {

				int iFurlong = 10 * lpMap->m_szLabel[1] + lpMap->m_szLabel[2] - 11 * '0';

				if (iFurlong >= 0 && iFurlong < DIMENSION(m_lpFurlongMaps))
					m_lpFurlongMaps[iFurlong] = lpMap;

				lpMap->m_bSpecialPaint = true;

			} else if (scumm_stricmp(lpMap->m_szLabel, "Hodj") == 0 || scumm_stricmp(lpMap->m_szLabel, "Podj") == 0) {

				InitOverlay(lpMap);    // initialize sprite

			} else if (scumm_stricmp(lpMap->m_szLabel, "Minib1") == 0) {
				lpMap->m_bSpecialPaint = true;
				m_cBbtMgr.LinkButton(&m_cMiniButton, lpcBgbObject, nullptr);

			} else if (scumm_stricmp(lpMap->m_szLabel, "Minib2") == 0) {
				lpMap->m_bSpecialPaint = true;
				m_cBbtMgr.LinkButton(&m_cMiniButton, nullptr, lpcBgbObject);

			} else if (scumm_stricmp(lpMap->m_szLabel, "Invb1") == 0) {
				lpMap->m_bSpecialPaint = true;
				m_cBbtMgr.LinkButton(&m_cInvButton, lpcBgbObject, nullptr);

			} else if (scumm_stricmp(lpMap->m_szLabel, "Invb2") == 0) {
				lpMap->m_bSpecialPaint = true;
				m_cBbtMgr.LinkButton(&m_cInvButton, nullptr, lpcBgbObject);

			} else if (scumm_stricmp(lpMap->m_szLabel, "Scrob1") == 0) {
				lpMap->m_bSpecialPaint = true;
				m_cBbtMgr.LinkButton(&m_cScrollButton, lpcBgbObject, nullptr);

			} else if (scumm_stricmp(lpMap->m_szLabel, "Scrob2") == 0) {
				lpMap->m_bSpecialPaint = true;
				m_cBbtMgr.LinkButton(&m_cScrollButton, nullptr, lpcBgbObject);

#ifndef NODEEDIT
			} else if (scumm_stricmp(lpMap->m_szLabel, "podjb1") == 0) {

				assert(m_xpXodjChain != nullptr);
				m_xpXodjChain->m_lpcIcon = lpcBgbObject;

			} else if (scumm_stricmp(lpMap->m_szLabel, "hodjb1") == 0) {

				assert(m_xpXodjChain != nullptr);
				assert(m_xpXodjChain->m_xpXodjNext != nullptr);
				m_xpXodjChain->m_xpXodjNext->m_lpcIcon = lpcBgbObject;
#endif
			}
		}
	}

	if (bNewBitmap) {
		// this forces PositionCharacters to repaint
		m_bSamePlace = true;

		PositionCharacters();
	}

	return iError != 0;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
