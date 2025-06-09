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

namespace Bagel {
namespace MFC {

CRuntimeClass::CRuntimeClass(const char *m_lpszClassName_, int m_nObjectSize_, UINT m_wSchema_,
	CObject *(*m_pfnCreateObject_)(), const CRuntimeClass *m_pBaseClass_,
	const CRuntimeClass *m_pNextClass_) {
	m_lpszClassName = m_lpszClassName_;
	m_nObjectSize = m_nObjectSize_;
	m_wSchema = m_wSchema_;
	m_pfnCreateObject = m_pfnCreateObject_;
	m_pBaseClass = m_pBaseClass_;
	m_pNextClass = m_pNextClass_;
}

CObject *CRuntimeClass::CreateObject() const {
	return m_pfnCreateObject();
}

bool CRuntimeClass::IsDerivedFrom(const CRuntimeClass *pBaseClass) const {
	assert(pBaseClass != nullptr);

	for (const CRuntimeClass *rtClass = this;
			rtClass; rtClass = rtClass->m_pBaseClass) {
		if (rtClass == pBaseClass)
			return true;
	}

	return false;
}

} // namespace MFC
} // namespace Bagel
