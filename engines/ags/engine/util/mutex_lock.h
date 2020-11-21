//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AGS_EE_UTIL__MUTEX_LOCK_H
#define __AGS_EE_UTIL__MUTEX_LOCK_H

#include "util/mutex.h"

namespace AGS
{
namespace Engine
{


class MutexLock
{
private:
	BaseMutex *_m;
	MutexLock(MutexLock const &); // non-copyable
	MutexLock& operator=(MutexLock const &); // not copy-assignable

public:
	void Release()
	{
		if (_m != nullptr) _m->Unlock();
		_m = nullptr;
	}

	void Acquire(BaseMutex &mutex)
	{
		Release();
		_m = &mutex;
		_m->Lock();
	}

	MutexLock() : _m(nullptr)
	{
	}

	explicit MutexLock(BaseMutex &mutex) : _m(nullptr)
	{
		Acquire(mutex);
	}

	~MutexLock()
	{
		Release();
	}
}; // class MutexLock


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__MUTEX_LOCK_H
