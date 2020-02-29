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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/thread.h"
#include "illusions/actor.h"

namespace Illusions {

// Thread

Thread::Thread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags)
	: _vm(vm), _threadId(threadId), _callingThreadId(callingThreadId), _notifyFlags(notifyFlags),
	_pauseCtr(0), _terminated(false) {
}

Thread::~Thread() {
}

int Thread::onUpdate() {
	return kTSTerminate;
}

void Thread::onSuspend() {
}

void Thread::onNotify() {
}

void Thread::onPause() {
}

void Thread::onUnpause() {
}

void Thread::onResume() {
}

void Thread::onTerminated() {
}

void Thread::onKill() {
	_vm->_controls->threadIsDead(_threadId);
	terminate();
}

uint32 Thread::sendMessage(int msgNum, uint32 msgValue) {
	return 0;
}

void Thread::pause() {
	if (!_terminated) {
		++_pauseCtr;
		if (_pauseCtr == 1)
			onPause();
	}
}

void Thread::unpause() {
	if (!_terminated) {
		--_pauseCtr;
		if (_pauseCtr == 0)
			onUnpause();
	}
}

void Thread::resume() {
	if (!_terminated) {
		--_pauseCtr;
		if (_pauseCtr == 0)
			onResume();
	}
}

void Thread::suspend() {
	if (!_terminated) {
		++_pauseCtr;
		if (_pauseCtr == 1)
			onSuspend();
	}
}

void Thread::notify() {
	if (!_terminated) {
		--_pauseCtr;
		if (_pauseCtr == 0)
			onNotify();
	}
}

int Thread::update() {
	// NOTE Deletion of terminated threads handled in caller
	int status = kTSYield;
	if (!_terminated && _pauseCtr <= 0) {
		status = onUpdate();
		if (status == kTSTerminate)
			terminate();
		else if (status == kTSSuspend)
			suspend();
	}
	return status;
}

void Thread::terminate() {
	if (!_terminated) {
		if (!(_notifyFlags & 1))
			_vm->notifyThreadId(_callingThreadId);
		_callingThreadId = 0;
		onTerminated();
		_terminated = true;
	}
}

// ThreadList

ThreadList::ThreadList(IllusionsEngine *vm)
	: _vm(vm) {
}

void ThreadList::startThread(Thread *thread) {
	_threads.push_back(thread);
}

void ThreadList::updateThreads() {
	while (1) {
		Iterator it = _threads.begin();
		while (it != _threads.end()) {
			Thread *thread = *it;
			if (thread->_terminated) {
				delete thread;
				it = _threads.erase(it);
			} else {
				int status = kTSRun;
				while (!thread->_terminated && status != kTSTerminate && status != kTSYield) {
					status = thread->update();
				}
				++it;
			}
		}
		if (_vm->_rerunThreads)
			_vm->_rerunThreads = false;
		else
			break;
	}
}

Thread *ThreadList::findThread(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		if ((*it)->_threadId == threadId && !(*it)->_terminated)
			return (*it);
	}
	return 0;
}

void ThreadList::suspendId(uint32 threadId) {
	Thread *thread = findThread(threadId);
	if (thread)
		thread->suspend();
}

void ThreadList::notifyId(uint32 threadId) {
	Thread *thread = findThread(threadId);
	if (thread)
		thread->notify();
}

void ThreadList::notifyTimerThreads(uint32 callingThreadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_type == kTTTimerThread && thread->_callingThreadId == callingThreadId)
			thread->notify();
	}
}

void ThreadList::suspendTimerThreads(uint32 callingThreadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_type == kTTTimerThread && thread->_callingThreadId == callingThreadId)
			thread->suspend();
	}
}

void ThreadList::terminateThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_threadId != threadId)
			thread->terminate();
	}
}

void ThreadList::terminateActiveThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_pauseCtr <= 0 && thread->_threadId != threadId)
			thread->terminate();
	}
}

void ThreadList::terminateThreadsBySceneId(uint32 sceneId, uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_sceneId == sceneId && thread->_threadId != threadId)
			thread->terminate();
	}
}

void ThreadList::suspendThreadsBySceneId(uint32 sceneId, uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_sceneId == sceneId && thread->_threadId != threadId)
			thread->suspend();
	}
}

void ThreadList::notifyThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_threadId != threadId)
			thread->notify();
	}
}

void ThreadList::notifyThreadsBySceneId(uint32 sceneId, uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_sceneId == sceneId && thread->_threadId != threadId)
			thread->notify();
	}
}

void ThreadList::pauseThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_threadId != threadId)
			thread->pause();
	}
}

void ThreadList::unpauseThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_threadId != threadId)
			thread->unpause();
	}
}

void ThreadList::suspendThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_threadId != threadId)
			thread->suspend();
	}
}

void ThreadList::resumeThreads(uint32 threadId) {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_threadId != threadId)
			thread->resume();
	}
}

void ThreadList::endTalkThreads() {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_type == kTTTalkThread)
			thread->terminate();
	}
}

void ThreadList::endTalkThreadsNoNotify() {
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (thread->_type == kTTTalkThread && thread->_callingThreadId == 0)
			thread->terminate();
	}
}

void ThreadList::terminateThreadChain(uint32 threadId) {
	while (threadId) {
		Thread *thread = findThread(threadId);
		thread->terminate();
		threadId = thread->_callingThreadId;
	}
}

void ThreadList::killThread(uint32 threadId) {

	if (!threadId)
		return;

	Thread *thread = findThread(threadId);
	if (!thread)
		return;

	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *childThread = *it;
		if (childThread->_callingThreadId == threadId)
			killThread(childThread->_threadId);
	}

	thread->onKill();

}

void ThreadList::setThreadSceneId(uint32 threadId, uint32 sceneId) {
	Thread *thread = findThread(threadId);
	if (thread)
		thread->_sceneId = sceneId;
}

uint32 ThreadList::getThreadSceneId(uint32 threadId) {
	Thread *thread = findThread(threadId);
	return thread ? thread->_sceneId : 0;
}

bool ThreadList::isActiveThread(int msgNum) {
	// Check if at least one thread returns a non-null value for the message
	for (Iterator it = _threads.begin(); it != _threads.end(); ++it) {
		Thread *thread = *it;
		if (!thread->_terminated && thread->_pauseCtr <= 0 &&
			thread->sendMessage(msgNum, 0) != 0)
			return true;
	}
	return false;
}

ThreadList::~ThreadList() {
	Iterator it = _threads.begin();
	while (it != _threads.end()) {
		delete *it;
		it = _threads.erase(it);
	}
}

} // End of namespace Illusions
