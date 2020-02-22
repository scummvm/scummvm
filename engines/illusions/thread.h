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

enum ThreadType {
	kTTScriptThread      = 1,
	kTTTimerThread       = 2,
	kTTTalkThread        = 3,
	kTTAbortableThread   = 4,
	kTTSpecialThread     = 5,
	kTTCauseThread       = 6
};

enum ThreadStatus {
	kTSTerminate     = 1,
	kTSYield         = 2,
	kTSSuspend       = 3,
	kTSRun           = 4
};

class Thread {
public:
	Thread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags);
	virtual ~Thread();
	virtual int onUpdate();
	virtual void onSuspend();
	virtual void onNotify();
	virtual void onPause();
	virtual void onUnpause();
	virtual void onResume();
	virtual void onTerminated();
	virtual void onKill();
	virtual uint32 sendMessage(int msgNum, uint32 msgValue);
	void pause();
	void unpause();
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
	uint32 _sceneId;
	uint _notifyFlags;
};

class ThreadList {
public:
	ThreadList(IllusionsEngine *vm);
	void startThread(Thread *thread);
	void updateThreads();
	Thread *findThread(uint32 threadId);
	void suspendId(uint32 threadId);
	void notifyId(uint32 threadId);
	void notifyTimerThreads(uint32 callingThreadId);
	void suspendTimerThreads(uint32 callingThreadId);
	void terminateThreads(uint32 threadId);
	void terminateActiveThreads(uint32 threadId);
	void terminateThreadsBySceneId(uint32 sceneId, uint32 threadId);
	void suspendThreadsBySceneId(uint32 sceneId, uint32 threadId);
	void notifyThreads(uint32 threadId);
	void notifyThreadsBySceneId(uint32 sceneId, uint32 threadId);
	void pauseThreads(uint32 threadId);
	void unpauseThreads(uint32 threadId);
	void suspendThreads(uint32 threadId);
	void resumeThreads(uint32 threadId);
	void endTalkThreads();
	void endTalkThreadsNoNotify();
	void terminateThreadChain(uint32 threadId);
	void killThread(uint32 threadId);
	void setThreadSceneId(uint32 threadId, uint32 sceneId);
	uint32 getThreadSceneId(uint32 threadId);
	bool isActiveThread(int msgNum);
	~ThreadList();
protected:
	typedef Common::List<Thread*> List;
	typedef List::iterator Iterator;
	IllusionsEngine *_vm;
	List _threads;
};

} // End of namespace Illusions

#endif // ILLUSIONS_THREAD_H
