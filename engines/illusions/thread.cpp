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

namespace Illusions {

// Thread

Thread::Thread(IllusionsEngine *vm)
	: _vm(vm), _pauseCtr(0), _terminated(false) {
}

void Thread::pause() {
	if (!_terminated) {
		++_pauseCtr;
		if (_pauseCtr == 1)
			onPause();
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
	// NOTE Deletion of removed threads handled in caller
	int result = 2;
	if (!_terminated) {
		if (_pauseCtr > 0)
			result = 2;
		else
			result = onUpdate();
		if (result == 1)
			terminate();
		else if (result == 3)
			suspend();
	}
	return result;
}

void Thread::terminate() {
	if (!_terminated) {
		if (_callingThreadId) {
			if (!(_notifyFlags & 1)) {
				// TODO scrmgrNotifyID(_callingThreadId);
			}
			_callingThreadId = 0;
		}
		onTerminated();
		// TODO _vm->removeThread(_threadId, this);
		_terminated = true;
	}
}

// ThreadList

ThreadList::ThreadList(IllusionsEngine *vm)
	: _vm(vm) {
}

void ThreadList::startThread(Thread *thread) {
	// TODO tag has to be set by the Thread class scrmgrGetCurrentScene();
	_threads.push_back(thread);
	// TODO _vm->addThread(thread->_threadId, thread);
}

void ThreadList::updateThreads() {
	while (1) {
		Iterator it = _threads.begin();
		while (it != _threads.end()) {
			Thread *thread = *it;
			if (thread->_terminated) {
				it = _threads.erase(it);
				delete thread;
			} else {
				while (!thread->_terminated) {
					int updateResult = thread->update();
					if (updateResult == 1 || updateResult == 2)
						break;
				}
				++it;
			}
		}
		/* TODO
		if (script->threadUpdateContinueFlag)
			script->_threadUpdateContinueFlag = false;
		else
		*/
			break;		
	}
}

} // End of namespace Illusions
