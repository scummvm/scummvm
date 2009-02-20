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
 * $URL$
 * $Id$
 *
 */

// Scripting module thread management component
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/interface.h"

#include "saga/script.h"

#include "saga/scene.h"

namespace Saga {

ScriptThread *Script::createThread(uint16 scriptModuleNumber, uint16 scriptEntryPointNumber) {
	ScriptThread *newThread;

	loadModule(scriptModuleNumber);
	if (_modules[scriptModuleNumber].entryPointsCount <= scriptEntryPointNumber) {
		error("Script::createThread wrong scriptEntryPointNumber");
	}

	newThread = _threadList.pushFront().operator->();
	newThread->_flags = kTFlagNone;
	newThread->_stackSize = DEFAULT_THREAD_STACK_SIZE;
	newThread->_stackBuf = (uint16 *)malloc(newThread->_stackSize * sizeof(*newThread->_stackBuf));
	newThread->_stackTopIndex = newThread->_stackSize - 2;
	newThread->_instructionOffset = _modules[scriptModuleNumber].entryPoints[scriptEntryPointNumber].offset;
	newThread->_commonBase = _commonBuffer;
	newThread->_staticBase = _commonBuffer + _modules[scriptModuleNumber].staticOffset;
	newThread->_moduleBase = _modules[scriptModuleNumber].moduleBase;
	newThread->_moduleBaseSize = _modules[scriptModuleNumber].moduleBaseSize;

	newThread->_strings = &_modules[scriptModuleNumber].strings;

	if (_vm->getGameId() == GID_IHNM)
		newThread->_voiceLUT = &_globalVoiceLUT;
	else
		newThread->_voiceLUT = &_modules[scriptModuleNumber].voiceLUT;

	return newThread;
}

void Script::wakeUpActorThread(int waitType, void *threadObj) {
	ScriptThread *thread;
	ScriptThreadList::iterator threadIterator;

	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		thread = threadIterator.operator->();
		if ((thread->_flags & kTFlagWaiting) && (thread->_waitType == waitType) && (thread->_threadObj == threadObj)) {
			thread->_flags &= ~kTFlagWaiting;
		}
	}
}

void Script::wakeUpThreads(int waitType) {
	ScriptThread *thread;
	ScriptThreadList::iterator threadIterator;

	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		thread = threadIterator.operator->();
		if ((thread->_flags & kTFlagWaiting) && (thread->_waitType == waitType)) {
			thread->_flags &= ~kTFlagWaiting;
		}
	}
}

void Script::wakeUpThreadsDelayed(int waitType, int sleepTime) {
	ScriptThread *thread;
	ScriptThreadList::iterator threadIterator;

	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		thread = threadIterator.operator->();
		if ((thread->_flags & kTFlagWaiting) && (thread->_waitType == waitType)) {
			thread->_waitType = kWaitTypeDelay;
			thread->_sleepTime = sleepTime;
		}
	}
}

void Script::executeThreads(uint msec) {
	ScriptThread *thread;
	ScriptThreadList::iterator threadIterator;

	if (_vm->_interface->_statusTextInput) {
		return;
	}

	threadIterator = _threadList.begin();

	while (threadIterator != _threadList.end()) {
		thread = threadIterator.operator->();

		if (thread->_flags & (kTFlagFinished | kTFlagAborted)) {
			if (thread->_flags & kTFlagFinished)
				setPointerVerb();

			if (_vm->getGameId() == GID_IHNM) {
				thread->_flags &= ~kTFlagFinished;
				thread->_flags |= kTFlagAborted;
				++threadIterator;
			} else {
				threadIterator = _threadList.erase(threadIterator);
			}
			continue;
		}

		if (thread->_flags & kTFlagWaiting) {

			switch (thread->_waitType) {
			case kWaitTypeDelay:
				if (thread->_sleepTime < msec) {
					thread->_sleepTime = 0;
				} else {
					thread->_sleepTime -= msec;
				}

				if (thread->_sleepTime == 0)
					thread->_flags &= ~kTFlagWaiting;
				break;

			case kWaitTypeWalk:
				{
					ActorData *actor;
					actor = (ActorData *)thread->_threadObj;
					if (actor->_currentAction == kActionWait) {
						thread->_flags &= ~kTFlagWaiting;
					}
				}
				break;

			case kWaitTypeWaitFrames: // IHNM
				if (thread->_frameWait < _vm->_frameCount)
					thread->_flags &= ~kTFlagWaiting;
				break;
			}
		}

		if (!(thread->_flags & kTFlagWaiting)) {
			if (runThread(thread)) {
				break;
			}
		}

		++threadIterator;
	}

}

void Script::abortAllThreads(void) {
	ScriptThread *thread;
	ScriptThreadList::iterator threadIterator;

	threadIterator = _threadList.begin();

	while (threadIterator != _threadList.end()) {
		thread = threadIterator.operator->();
		thread->_flags |= kTFlagAborted;
		++threadIterator;
	}
	executeThreads(0);
}

void Script::completeThread(void) {
	int limit = (_vm->getGameId() == GID_IHNM) ? 100 : 40;

	for (int i = 0; i < limit && !_threadList.empty(); i++)
		executeThreads(0);
}

bool Script::runThread(ScriptThread *thread) {
	uint16 savedInstructionOffset;
	bool stopParsing = false;
	bool breakOut = false;
	int operandChar;

	MemoryReadStream scriptS(thread->_moduleBase, thread->_moduleBaseSize);

	scriptS.seek(thread->_instructionOffset);

	for (uint instructionCount = 0; instructionCount < STHREAD_TIMESLICE; instructionCount++) {
		if (thread->_flags & (kTFlagAsleep))
			break;

		savedInstructionOffset = thread->_instructionOffset;
		operandChar = scriptS.readByte();

		debug(8, "Executing thread offset: %u (%x) stack: %d", thread->_instructionOffset, operandChar, thread->pushedSize());

		stopParsing = false;
		debug(4, "Calling op %s", this->_scriptOpsList[operandChar].scriptOpName);
		(this->*_scriptOpsList[operandChar].scriptOp)(thread, &scriptS, stopParsing, breakOut);
		if (stopParsing)
			return breakOut;

		if (thread->_flags & (kTFlagFinished | kTFlagAborted)) {
			error("Wrong flags %d in thread", thread->_flags);
		}

		// Set instruction offset only if a previous instruction didn't branch
		if (savedInstructionOffset == thread->_instructionOffset) {
			thread->_instructionOffset = scriptS.pos();
		} else {
			if (thread->_instructionOffset >= scriptS.size()) {
				error("Script::runThread() Out of range script execution");
			}

			scriptS.seek(thread->_instructionOffset);
		}

		if (breakOut)
			break;
	}
	return false;
}

} // End of namespace Saga

