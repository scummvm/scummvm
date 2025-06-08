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

#include "bagel/mfc/win_hand.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

CHandleMap::CHandleMap(const CRuntimeClass *pClass,
		size_t nOffset, int nHandles) : m_pClass(pClass),
		m_nOffset(nOffset), m_nHandles(nHandles) {
	assert(pClass != nullptr);
	assert(nHandles == 1 || nHandles == 2);
}

CObject *CHandleMap::FromHandle(HANDLE h) {
	assert(m_pClass != nullptr);
	assert(m_nHandles == 1 || m_nHandles == 2);

	if (h == nullptr)
		return nullptr;

	// Check for permanent object, and return if present
	CObject *pObject = LookupPermanent(h);
	if (pObject != nullptr)
		return pObject;
	else if ((pObject = LookupTemporary(h)) != nullptr) {
		HANDLE *ph = (HANDLE *)((byte *)pObject + m_nOffset);
		assert(ph[0] == h || ph[0] == nullptr);

		ph[0] = h;
		if (m_nHandles == 2) {
			assert(ph[1] == h || ph[1] == nullptr);
			ph[1] = h;
		}

		return pObject;
	}

	// This wasn't created by us, so create a temporary one.
	// Not sure if this is needed in ScummVM, but it may
	// be useful in cases where we directly create an
	// implementation instance without having an outer
	// MFC class wrapping it initially
	CObject *pTemp = m_pClass->CreateObject();
	assert(pTemp);

	m_temporaryMap[h] = pTemp;

	// Now set the handle in the object
	HANDLE *ph = (HANDLE *)((byte *)pTemp + m_nOffset);
	ph[0] = h;
	if (m_nHandles == 2)
		ph[1] = h;

	return pTemp;
}

void CHandleMap::DeleteTemp() {
	if (this == nullptr)
		return;

	for (auto &entry : m_temporaryMap) {
		HANDLE h = entry._key;
		CObject *pTemp = entry._value;

		// zero out the handles
		assert(m_nHandles == 1 || m_nHandles == 2);
		HANDLE *ph = (HANDLE *)((BYTE *)pTemp + m_nOffset);  // after CObject
		assert(ph[0] == h || ph[0] == nullptr);
		ph[0] = nullptr;

		if (m_nHandles == 2) {
			assert(ph[1] == h || ph[1] == nullptr);
			ph[1] = nullptr;
		}

		delete pTemp;
	}

	m_temporaryMap.clear();
}

void CHandleMap::SetPermanent(HANDLE h, CObject *permOb) {
	 m_permanentMap[(LPVOID)h] = permOb;
}

void CHandleMap::RemoveHandle(HANDLE h) {
	m_permanentMap.erase((LPVOID)h);
}

} // namespace MFC
} // namespace Bagel
