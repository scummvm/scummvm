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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/win_hand.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNCREATE(CGdiObject, CObject)

CGdiObject::~CGdiObject() {
	if (m_hObject && _permanent)
		DeleteObject();
}

bool CGdiObject::Attach(HGDIOBJ hObject) {
	// Detach any prior object
	Detach();

	// Attach new object
	m_hObject = hObject;
	AfxHookObject();

	return true;
}

HGDIOBJ CGdiObject::Detach() {
	HGDIOBJ result = m_hObject;
	AfxUnhookObject();

	return result;
}

bool CGdiObject::DeleteObject() {
	AfxUnhookObject();

	// Check added to ensure I don't accidentally
	// delete the _defaultBitmap field of the CDC::Impl
	assert(!dynamic_cast<CDC::DefaultBitmap *>(this));

	delete m_hObject;
	m_hObject = nullptr;
	return true;
}

CGdiObject *CGdiObject::FromHandle(HGDIOBJ h) {
	CHandleMap<CGdiObject> *pMap = AfxGetApp()->afxMapHGDIOBJ(true);
	assert(pMap != nullptr);

	CGdiObject *pObject = pMap->FromHandle(h);
	assert(pObject == nullptr || pObject->m_hObject == h);
	return pObject;
}

void CGdiObject::AfxHookObject() {
	CHandleMap<CGdiObject> *pMap = AfxGetApp()->afxMapHGDIOBJ(true);
	assert(pMap != nullptr);

	if (!pMap->LookupPermanent(m_hObject)) {
		pMap->SetPermanent(m_hObject, this);
		_permanent = true;
	}
}

void CGdiObject::AfxUnhookObject() {
	if (m_hObject && _permanent) {
		CHandleMap<CGdiObject> *pMap = AfxGetApp()->afxMapHGDIOBJ(true);
		assert(pMap != nullptr);

		pMap->RemoveHandle(m_hObject);
		_permanent = false;
	}
}


} // namespace MFC
} // namespace Bagel
