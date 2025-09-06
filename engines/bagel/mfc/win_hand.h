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
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

class CGdiObject;
class CDC;

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

template<typename T>
struct HandleTraits;

// Specialize for CGdiObject
template<>
struct HandleTraits<CGdiObject> {
	static HANDLE &GetHandle(CGdiObject &obj) {
		return *(HANDLE *)&obj.m_hObject;
	}
};

// Specialize for CDC
template<>
struct HandleTraits<CDC> {
	static HANDLE &GetHandle(CDC &obj) {
		return *(HANDLE *)&obj.m_hDC;
	}
};

template<class T>
class CHandleMap {
public:
	typedef Common::HashMap<void *, T *,
		VoidPtr_Hash, VoidPtr_EqualTo> CMapPtrToPtr;

private:
	CMapPtrToPtr _permanentMap;
	CMapPtrToPtr _temporaryMap;

public:
	~CHandleMap() {
		DeleteTemp();
	}

	T *FromHandle(HANDLE h) {
		if (h == nullptr)
			return nullptr;

		// Check for permanent object, and return if present
		T *pObject = LookupPermanent(h);
		if (pObject != nullptr)
			return pObject;
		else if ((pObject = LookupTemporary(h)) != nullptr) {
			assert(HandleTraits<T>::GetHandle(*pObject) == h ||
				HandleTraits<T>::GetHandle(*pObject) == nullptr);

			HandleTraits<T>::GetHandle(*pObject) = h;
			return pObject;
		}

		// This wasn't created by us, so create a temporary one.
		T *pTemp = (T *)T::CreateObject();
		assert(pTemp);

		_temporaryMap[h] = pTemp;

		// Now set the handle in the object and return it
		HandleTraits<T>::GetHandle(*pTemp) = h;
		return pTemp;
	}

	void DeleteTemp() {
		for (auto &entry : _temporaryMap) {
			HANDLE h = entry._key;
			T *pTemp = entry._value;

			// Zero out the handle
			assert(HandleTraits<T>::GetHandle(*pTemp) == h ||
				HandleTraits<T>::GetHandle(*pTemp) == nullptr);
			HandleTraits<T>::GetHandle(*pTemp) = nullptr;

			delete pTemp;
		}

		_temporaryMap.clear();
	}

	void SetPermanent(HANDLE h, T *permOb) {
		assert(!LookupPermanent(h));
		_permanentMap[(void *)h] = permOb;
	}

	void RemoveHandle(HANDLE h) {
		_permanentMap.erase((void *)h);
	}

	inline T *LookupPermanent(HANDLE h) const {
		return _permanentMap.contains((void *)h) ?
			_permanentMap[(void *)h] : nullptr;
	}

	inline T *LookupTemporary(HANDLE h) const {
		return _temporaryMap.contains((void *)h) ?
			_temporaryMap[(void *)h] : nullptr;
	}
};

} // namespace MFC
} // namespace Bagel

#endif
