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

#ifndef PSP2_THREAD_H
#define PSP2_THREAD_H

#include <psp2common/types.h>
#include <psp2/kernel/processmgr.h>

#include "common/mutex.h"

// class for thread utils
class Psp2Thread {
public:
	// static functions
	static void delayMillis(uint32 ms);	// delay the current thread
	static void delayMicros(uint32 us);
};

class Psp2Semaphore {
private:
	uint32 _handle;
public:
	Psp2Semaphore(int initialValue, int maxValue=255);
	~Psp2Semaphore();
	bool take() { return takeWithTimeOut(0); }
	bool takeWithTimeOut(uint timeOut);
	bool give(int num=1);
	bool pollForValue(int value);	// check for a certain value
	int numOfWaitingThreads();
	int getValue();
};

class Psp2Mutex : public Common::MutexInternal {
private:
	Psp2Semaphore _semaphore;
	int _recursiveCount;
	int _ownerId;
public:
	Psp2Mutex(bool initialValue) : _semaphore(initialValue ? 1 : 0, 255), _recursiveCount(0), _ownerId(0) {}	// initial, max value
	bool lock();
	bool unlock();
	bool poll() { return _semaphore.pollForValue(1); }
	int numOfWaitingThreads() { return _semaphore.numOfWaitingThreads(); }
	bool getValue() { return (bool)_semaphore.getValue(); }
};

class Psp2Condition {
private:
	Psp2Mutex _mutex;
	int _waitingThreads;
	int _signaledThreads;
	Psp2Semaphore _waitSem;
	Psp2Semaphore _doneSem;
public:
	Psp2Condition() : _mutex(true), _waitingThreads(0), _signaledThreads(0),
								_waitSem(0), _doneSem(0) {}
	void wait(Psp2Mutex &externalMutex);
	void releaseAll();
};

#endif /* PSP2_THREADS_H */
