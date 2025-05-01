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

#include "bagel/mfc/afx.h"

namespace Bagel {
namespace MFC {

const CRuntimeClass CObject::classCObject;

CObject::CObject() {
}

CObject::CObject(const CObject &objectSrc) {
	*this = objectSrc;
}

CObject::~CObject() {
}

void CObject::operator=(const CObject &objectSrc) {
}

BOOL CObject::IsSerializable() const {
	return false;
}

BOOL CObject::IsKindOf(const CRuntimeClass *pClass) const {
	for (const CRuntimeClass *p = &classCObject; p;
			p = p->m_pBaseClass) {
		if (p == pClass)
			return true;
	}

	return false;
}

	virtual void Serialize(CArchive &ar);

#if defined(_DEBUG) || defined(_AFXDLL)
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

	// Implementation
public:

} // namespace MFC
} // namespace Bagel
