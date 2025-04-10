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

#include "bagel/hodjnpodj/metagame/boardgame/gtl_doc.h"
#include "bagel/hodjnpodj/metagame/boardgame/boardgame.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

bool CGtlDoc::onNewDocument() {
	CGtlApp *xpGtlApp = AfxGetApp();
	
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	if (++xpGtlApp->m_iNumOpens == 1 && xpGtlApp->m_szFilename[0])
		initDocument(xpGtlApp->m_szFilename);
	else
		initDocument("default.gtl");

	return true;
}

void CGtlDoc::initDocument(const char *xpszPathName) {
	CGtlApp *xpGtlApp = AfxGetApp();

	m_xpGtlData = new CGtlData();
	m_xpGtlData->m_xpcGtlDoc = this;
	m_xpGtlData->m_bShowNodes = xpGtlApp->m_bShowNodes;
	m_xpGtlData->m_bPaintBackground = xpGtlApp->m_bPaintBackground;

	onChangedViewList();
	fixChecks();

	if (xpszPathName && *xpszPathName)
		m_xpGtlData->compile(xpszPathName);

	// TODO: More stuff
}

void CGtlDoc::onChangedViewList() {
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
