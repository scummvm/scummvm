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

#ifndef HODJNPODJ_METAGAME_GTL_DOC_H
#define HODJNPODJ_METAGAME_GTL_DOC_H

#include "bagel/hodjnpodj/metagame/boardgame/gtl_app.h"
#include "bagel/hodjnpodj/metagame/boardgame/gtl_data.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class CGtlDoc {
private:
	CGtlData *m_xpGtlData = nullptr;

	void initDocument(const char *xpszPathName);
	void onChangedViewList();
	void fixChecks() {}
	void UpdateAllViews(CView *pSender, long lHint = 0L, CObject *pHint = nullptr) {}

public:
	CGtlDoc() {}

	void onNewDocument();
	void deleteContents();
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
