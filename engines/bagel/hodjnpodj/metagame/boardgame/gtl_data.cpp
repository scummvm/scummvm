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
#include "bagel/hodjnpodj/metagame/boardgame/boardgame.h"
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

CNode::CNode(void) {
	// the node table must stay under 32k
	assert((uint)sizeof(CNode) * MAX_NODES < (uint)0x7FFF);

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
	m_pOffScreenBmp = NULL;
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


} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
