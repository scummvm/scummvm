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



#include "backends/platform/sdl/psp2/thread.h"
#include "common/debug.h"

// Psp2Thread class
// Utilities to access general thread functions

void Psp2Thread::delayMillis(uint32 ms) {
	sceKernelDelayThread(ms * 1000);
}

void Psp2Thread::delayMicros(uint32 us) {
	sceKernelDelayThread(us);
}

// Class Psp2Semaphore ------------------------------------------------

Psp2Semaphore::Psp2Semaphore(int initialValue, int maxValue/*=255*/) {
	_handle = 0;
	_handle = (uint32)sceKernelCreateSema("ScummVM Sema", 0 /* attr */,
								  initialValue, maxValue,
								  0 /*option*/);
	if (!_handle)
		debug(8, "failed to create semaphore.");
}

Psp2Semaphore::~Psp2Semaphore() {
	if (_handle)
		if (sceKernelDeleteSema((SceUID)_handle) < 0)
		    debug(8, "failed to delete semaphore.");
}

int Psp2Semaphore::numOfWaitingThreads() {
	SceKernelSemaInfo info;
	info.numWaitThreads = 0;

	if (sceKernelGetSemaInfo((SceUID)_handle, &info) < 0)
		debug(8, "failed to retrieve semaphore info for handle %d", _handle);

	return info.numWaitThreads;
}

int Psp2Semaphore::getValue() {
	SceKernelSemaInfo info;
	info.currentCount = 0;

	if (sceKernelGetSemaInfo((SceUID)_handle, &info) < 0)
		debug(8, "failed to retrieve semaphore info for handle %d", _handle);

	return info.currentCount;
}

bool Psp2Semaphore::pollForValue(int value) {
	if (sceKernelPollSema((SceUID)_handle, value) < 0)
		return false;

	return true;
}

// false: timeout or error
bool Psp2Semaphore::takeWithTimeOut(uint timeOut) {
	uint *pTimeOut = 0;
	if (timeOut)
		pTimeOut = &timeOut;

	if (sceKernelWaitSema(_handle, 1, pTimeOut) < 0)	// we always wait for 1
		return false;
	return true;
}

bool Psp2Semaphore::give(int num /*=1*/) {
	if (sceKernelSignalSema((SceUID)_handle, num) < 0)
		return false;
	return true;
}

// Class PspMutex ------------------------------------------------------------

bool Psp2Mutex::lock() {
	int threadId = sceKernelGetThreadId();
	bool ret = true;

	if (_ownerId == threadId) {
		_recursiveCount++;
	} else {
		ret = _semaphore.take();
		_ownerId = threadId;
		_recursiveCount = 0;
	}
	return ret;
}

bool Psp2Mutex::unlock() {
	int threadId = sceKernelGetThreadId();
	bool ret = true;

	if (_ownerId != threadId) {
		debug(8, "attempt to unlock mutex by thread[%x] as opposed to owner[%x]",
			threadId, _ownerId);
		return false;
	}

	if (_recursiveCount) {
		_recursiveCount--;
	} else {
		_ownerId = 0;
		ret = _semaphore.give(1);
	}
	return ret;
}

// Class PspCondition -------------------------------------------------

// Release all threads waiting on the condition
void Psp2Condition::releaseAll() {
	_mutex.lock();
	if (_waitingThreads > _signaledThreads) {	// we have signals to issue
		int numWaiting = _waitingThreads - _signaledThreads;	// threads we haven't signaled
		_signaledThreads = _waitingThreads;

		_waitSem.give(numWaiting);
		_mutex.unlock();
		for (int i=0; i<numWaiting; i++)	// wait for threads to tell us they're awake
			_doneSem.take();
	} else {
		_mutex.unlock();
	}
}

// Mutex must be taken before entering wait
void Psp2Condition::wait(Psp2Mutex &externalMutex) {
	_mutex.lock();
	_waitingThreads++;
	_mutex.unlock();

	externalMutex.unlock();	// must unlock external mutex

	_waitSem.take();	// sleep on the wait semaphore

	// let the signaling thread know we're done
	_mutex.lock();
	if (_signaledThreads > 0 ) {
		_doneSem.give();	// let the thread know
		_signaledThreads--;
	}
	_waitingThreads--;
	_mutex.unlock();

	externalMutex.lock();		// must lock external mutex here for continuation
}
