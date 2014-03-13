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

#ifndef ILLUSIONS_THREAD_H
#define ILLUSIONS_THREAD_H

#include "common/list.h"

namespace Illusions {

class IllusionsEngine;

class Thread {
public:
	Thread(IllusionsEngine *vm);
	virtual ~Thread();
	virtual int onUpdate() = 0;
	virtual void onSuspend() = 0;
	virtual void onNotify() = 0;
	virtual void onPause() = 0;
	virtual void onResume() = 0;
	virtual void onTerminated() = 0;
	void pause();
	void resume();
	void suspend();
	void notify();
	int update();
	void terminate();
public:
	IllusionsEngine *_vm;
	//field_0 dw
	int _pauseCtr;
	bool _terminated;
	//field_6 dw
	uint _type;
	uint32 _threadId;
	uint32 _callingThreadId;
	uint32 _tag;
	uint _notifyFlags;
};

class ThreadList {
public:
	ThreadList(IllusionsEngine *vm);
	void startThread(Thread *thread);
	void updateThreads();
protected:
	typedef Common::List<Thread*> List;
	typedef List::iterator Iterator;
	IllusionsEngine *_vm;
	List _threads;
};

} // End of namespace Illusions

#endif // ILLUSIONS_THREAD_H
