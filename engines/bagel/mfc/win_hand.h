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

#ifndef BAGEL_MFC_WIN_HAND_H
#define BAGEL_MFC_WIN_HAND_H

#include "common/hashmap.h"
#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {

class CObject;
struct CRuntimeClass;

struct VoidPtr_EqualTo {
	bool operator()(const void *x, const void *y) const {
		return x == y;
	}
};

struct VoidPtr_Hash {
	uint operator()(const void *x) const {
		return (intptr)x;
	}
};

typedef Common::HashMap<void *, CObject *,
	VoidPtr_Hash, VoidPtr_EqualTo> CMapPtrToPtr;

class CHandleMap {
private:
	CMapPtrToPtr m_permanentMap;
	CMapPtrToPtr m_temporaryMap;
	const CRuntimeClass *m_pClass;
	size_t m_nOffset = 0;	// offset of handles in the object
	int m_nHandles = 0;		// 1 or 2 (for CDC)

public:
	// Constructor/Destructor
	CHandleMap(const CRuntimeClass *pClass, size_t nOffset, int nHandles = 1);
	~CHandleMap() {
		DeleteTemp();
	}

public:
	CObject *FromHandle(HANDLE h);
	void DeleteTemp();

	void SetPermanent(HANDLE h, CObject *permOb);
	void RemoveHandle(HANDLE h);

	inline CObject *LookupPermanent(HANDLE h) const {
		return m_permanentMap[(void *)h];
	}

	inline CObject *LookupTemporary(HANDLE h) const {
		return m_temporaryMap[(void *)h];
	}
};

} // namespace MFC
} // namespace Bagel

#endif
