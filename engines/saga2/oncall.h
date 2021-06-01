/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ONCALL_H
#define SAGA2_ONCALL_H

#include "saga2/bitarray.h"
#include "saga2/rmem.h"
#include "saga2/localize.h"

namespace Saga2 {

#define isValidPtr(p) ((p!=NULL)&&(p!=(void *)0xCDCDCDCD))

class HandleArray {
private:
	Common::Array<byte*> _handles;
	uint32 _tileID;
	byte*(*_loader)(hResID, bool);
public:
	HandleArray(uint16 size, byte*(*loadfunction)(hResID, bool), uint32 newID) {
		for (int i = 0; i < size; ++i)
			_handles.push_back(nullptr);
		_loader = loadfunction;
		_tileID = newID;
	}

	void flush() {
		for (int i = 0; i < _handles.size(); ++i) {
			if (_handles[i]) {
				free(_handles[i]);
				_handles[i] = nullptr;
			}
		}
	}

	byte *operator[](uint32 ind) {
		if (_handles[ind])
			return _handles[ind];

		return _handles[ind] = _loader(_tileID + MKTAG(0, 0, 0, ind), false);
	}
};

template <class RESTYPE> class LoadOnCall {
	BitArray        wanted;
	BitArray        locked;
	BitArray        recent;
	RESTYPE         *handle;
	uint16          handles;
	uint32          tileID;
	RHANDLE(*loader)(hResID, bool);

	RESTYPE rLoad(uint16 ind, bool asynch);
	void rFree(uint16);
	void rInit(uint16);

public:

	LoadOnCall(uint16   newSize, RHANDLE(*loadfunction)(hResID, bool), uint32 newID) {
		tileID = newID;
		wanted.resize(newSize);
		locked.resize(newSize);
		recent.resize(newSize);
		handle = (RESTYPE *)malloc(sizeof(RESTYPE) * newSize);
		handles = newSize;
		loader = loadfunction;
		for (uint16 i = 0; i < newSize; i++)
			rInit(i);
	}

	//  destructor
	~LoadOnCall() {
		//if (handles) delete[] handle;
	}

	RESTYPE operator[](uint32 ind) {
//		assert (ind<handles);
		if (!locked[ind]) handle[ind] = rLoad(ind, FALSE);
		return handle[ind];
	}

	void flush(void) {
		//recent.clearAll();
		for (uint16 i = 0; i < handles; i++)
			if ((locked[i]) && (!recent[i]))
				rFree(i);
		//locked|=recent;
		recent.clearAll();
	}

	BitArray lockState(void) {
		return locked;
	}

	BitArray wantState(void) {
		return wanted;
	}

};

template <class RESTYPE> RESTYPE LoadOnCall<RESTYPE>::rLoad(uint16 ind, bool asynch) {
	RESTYPE t;
	if (isValidPtr(handle[ ind ]) && locked[ind]) {
		RLockHandle((RHANDLE) handle[ ind ]);
		locked.Bit(ind, TRUE);
		wanted.Bit(ind, FALSE);
		recent.Bit(ind, TRUE);
		return handle[ ind ];
	}
	if (isValidPtr(handle[ ind ]) && wanted[ind]) {
		// wait for handle
		RLockHandle((RHANDLE) handle[ ind ]);
		locked.Bit(ind, TRUE);
		wanted.Bit(ind, FALSE);
		recent.Bit(ind, TRUE);
		return NULL;
	}

	//t = (RESTYPE) tileRes->load( tileID + RES_ID( 0,0,0,bankNum ) );
	t = (RESTYPE) loader(tileID + RES_ID(0, 0, 0, ind), asynch);

	locked.Bit(ind, !asynch);
	wanted.Bit(ind, asynch);
	recent.Bit(ind, TRUE);

	if (asynch) {
		handle[ ind ] = (RESTYPE) NULL;
	} else {
		if (t == NULL) {
			error("Resource %d could not load (Tile bank)", ind);
		}
		handle[ ind ] = t;
	}
	return t;
}

template <class RESTYPE> void LoadOnCall<RESTYPE>::rFree(uint16 ind) {
	if (isValidPtr(handle[ ind ])) {
		RUnlockHandle((RHANDLE) handle[ ind ]);
		locked.Bit(ind, FALSE);
		//recent.Bit(ind,FALSE);
//		washHandle(handle[ind]);
	}
}

template <class RESTYPE> void LoadOnCall<RESTYPE>::rInit(uint16 ind) {
	RESTYPE t;
	if (!isValidPtr(handle[ ind ])) {
		t = (RESTYPE) loader(tileID + RES_ID(0, 0, 0, ind), FALSE);
		handle[ ind ] = t;
		locked.Bit(ind, TRUE);
		RUnlockHandle((RHANDLE) handle[ ind ]);
		locked.Bit(ind, FALSE);
		recent.Bit(ind, FALSE);
	}
}

//typedef LoadOnCall<TileBankHandle> tileBankArray; //(64,tileResLoad);

} // end of namespace Saga2

#endif
