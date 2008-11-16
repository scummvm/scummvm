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

#define RID_SCENE1_VOICE_START 57
#define RID_SCENE1_VOICE_END 186

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

	if (_vm->getGameType() == GType_IHNM)
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

			if (_vm->getGameType() == GType_IHNM) {
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
			if (runThread(thread, STHREAD_TIMESLICE)) {
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
	int limit = (_vm->getGameType() == GType_IHNM) ? 100 : 40;

	for (int i = 0; i < limit && !_threadList.empty(); i++)
		executeThreads(0);
}

bool Script::runThread(ScriptThread *thread, uint instructionLimit) {
	const char*operandName;
	uint instructionCount;
	uint16 savedInstructionOffset;

	byte *addr;
	byte mode;
	uint16 jmpOffset1;
	int16 iparam1;
	int16 iparam2;
	int16 iparam3;

	bool disContinue;
	byte argumentsCount;
	uint16 functionNumber;
	uint16 checkStackTopIndex;
	ScriptFunctionType scriptFunction;

	int operandChar;
	int i;

	MemoryReadStream scriptS(thread->_moduleBase, thread->_moduleBaseSize);

	scriptS.seek(thread->_instructionOffset);

	for (instructionCount = 0; instructionCount < instructionLimit; instructionCount++) {
		if (thread->_flags & (kTFlagAsleep))
			break;

		savedInstructionOffset = thread->_instructionOffset;
		operandChar = scriptS.readByte();


#define CASEOP(opName)	case opName:												\
							if (operandChar == opName) {							\
								operandName = #opName;								\
								debug(2, "%s", operandName);						\
							}

		debug(8, "Executing thread offset: %u (%x) stack: %d", thread->_instructionOffset, operandChar, thread->pushedSize());
		operandName="";
		switch (operandChar) {
		CASEOP(opNextBlock)
			// Some sort of "jump to the start of the next memory
			// page" instruction, I think.
			thread->_instructionOffset = (((thread->_instructionOffset) >> 10) + 1) << 10;
			break;

// STACK INSTRUCTIONS
		CASEOP(opDup)
			thread->push(thread->stackTop());
			break;
		CASEOP(opDrop)
			thread->pop();
			break;
		CASEOP(opZero)
			thread->push(0);
			break;
		CASEOP(opOne)
			thread->push(1);
			break;
		CASEOP(opConstint)
		CASEOP(opStrlit)
			iparam1 = scriptS.readSint16LE();
			thread->push(iparam1);
			debug(8, "0x%X", iparam1);
			break;

// DATA INSTRUCTIONS
		CASEOP(opGetFlag)
			addr = thread->baseAddress(scriptS.readByte());
			iparam1 = scriptS.readSint16LE();
			addr += (iparam1 >> 3);
			iparam1 = (1 << (iparam1 & 7));
			thread->push((*addr) & iparam1 ? 1 : 0);
			break;
		CASEOP(opGetInt)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			thread->push(readUint16(addr, mode));
			debug(8, "0x%X", readUint16(addr, mode));
			break;
		CASEOP(opPutFlag)
			addr = thread->baseAddress(scriptS.readByte());
			iparam1 = scriptS.readSint16LE();
			addr += (iparam1 >> 3);
			iparam1 = (1 << (iparam1 & 7));
			if (thread->stackTop()) {
				*addr |= iparam1;
			} else {
				*addr &= ~iparam1;
			}
			break;
		CASEOP(opPutInt)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			writeUint16(addr, thread->stackTop(), mode);
			break;
		CASEOP(opPutFlagV)
			addr = thread->baseAddress(scriptS.readByte());
			iparam1 = scriptS.readSint16LE();
			addr += (iparam1 >> 3);
			iparam1 = (1 << (iparam1 & 7));
			if (thread->pop()) {
				*addr |= iparam1;
			} else {
				*addr &= ~iparam1;
			}
			break;
		CASEOP(opPutIntV)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			writeUint16(addr, thread->pop(), mode);
			break;

// FUNCTION CALL INSTRUCTIONS
		CASEOP(opCall)
			argumentsCount = scriptS.readByte();
			iparam1 = scriptS.readByte();
			if (iparam1 != kAddressModule) {
				error("Script::runThread iparam1 != kAddressModule");
			}
			addr = thread->baseAddress(iparam1);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			thread->push(argumentsCount);

			jmpOffset1 = scriptS.pos();
			// NOTE: The original pushes the program
			// counter as a pointer here. But I don't think
			// we will have to do that.
			thread->push(jmpOffset1);
			// NOTE2: program counter is 32bit - so we should "emulate" it size - because kAddressStack relies on it
			thread->push(0);
			thread->_instructionOffset = iparam1;

			break;
		CASEOP(opCcall)
		CASEOP(opCcallV)
			argumentsCount = scriptS.readByte();
			functionNumber = scriptS.readUint16LE();
			if (functionNumber >= ((_vm->getGameType() == GType_IHNM) ?
								   IHNM_SCRIPT_FUNCTION_MAX : ITE_SCRIPT_FUNCTION_MAX)) {
				error("Script::runThread() Invalid script function number (%d)", functionNumber);
			}

			debug(2, "Calling #%d %s argCount=%i", functionNumber, _scriptFunctionsList[functionNumber].scriptFunctionName, argumentsCount);
			scriptFunction = _scriptFunctionsList[functionNumber].scriptFunction;
			checkStackTopIndex = thread->_stackTopIndex + argumentsCount;
			disContinue = false;
			(this->*scriptFunction)(thread, argumentsCount, disContinue);
			if (disContinue) {
				return true;
			}
			if (scriptFunction == &Saga::Script::sfScriptGotoScene ||
				scriptFunction == &Saga::Script::sfVsetTrack) {
				return true; // cause abortAllThreads called and _this_ thread destroyed
			}

			thread->_stackTopIndex = checkStackTopIndex;

			if (operandChar == opCcall) {// CALL function
				thread->push(thread->_returnValue);
			}

			if (thread->_flags & kTFlagAsleep)
				instructionCount = instructionLimit;	// break out of loop!
			break;
		CASEOP(opEnter)
			thread->push(thread->_frameIndex);
			thread->_frameIndex = thread->_stackTopIndex;
			thread->_stackTopIndex -= (scriptS.readSint16LE() / 2);
			break;
		CASEOP(opReturn)
			thread->_returnValue = thread->pop();
		CASEOP(opReturnV)
			thread->_stackTopIndex = thread->_frameIndex;
			thread->_frameIndex = thread->pop();
			if (thread->pushedSize() == 0) {
				thread->_flags |= kTFlagFinished;
				return true;
			} else {
				thread->pop(); //cause it 0
				thread->_instructionOffset = thread->pop();

				// Pop all the call parameters off the stack
				iparam1 = thread->pop();
				while (iparam1--) {
					thread->pop();
				}

				if (operandChar == opReturn) {
					thread->push(thread->_returnValue);
				}
			}
			break;

// BRANCH INSTRUCTIONS
		CASEOP(opJmp)
			jmpOffset1 = scriptS.readUint16LE();
			thread->_instructionOffset = jmpOffset1;
			break;
		CASEOP(opJmpTrueV)
			jmpOffset1 = scriptS.readUint16LE();
			if (thread->pop()) {
				thread->_instructionOffset = jmpOffset1;
			}
			break;
		CASEOP(opJmpFalseV)
			jmpOffset1 = scriptS.readUint16LE();
			if (!thread->pop()) {
				thread->_instructionOffset = jmpOffset1;
			}
			break;
		CASEOP(opJmpTrue)
			jmpOffset1 = scriptS.readUint16LE();
			if (thread->stackTop()) {
				thread->_instructionOffset = jmpOffset1;
			}
			break;
		CASEOP(opJmpFalse)
			jmpOffset1 = scriptS.readUint16LE();
			if (!thread->stackTop()) {
				thread->_instructionOffset = jmpOffset1;
			}
			break;
		CASEOP(opJmpSwitch)
			iparam1 = scriptS.readSint16LE();
			iparam2 = thread->pop();
			while (iparam1--) {
				iparam3 = scriptS.readUint16LE();
				thread->_instructionOffset = scriptS.readUint16LE();
				if (iparam3 == iparam2) {
					break;
				}
			}
			if (iparam1 < 0) {
				thread->_instructionOffset = scriptS.readUint16LE();
			}
			break;
		CASEOP(opJmpRandom)
			// Supposedly the number of possible branches.
			// The original interpreter ignores it.
			scriptS.readUint16LE();
			iparam1 = scriptS.readSint16LE();
			iparam1 = _vm->_rnd.getRandomNumber(iparam1 - 1);
			while (1) {
				iparam2 = scriptS.readSint16LE();
				thread->_instructionOffset = scriptS.readUint16LE();

				iparam1 -= iparam2;
				if (iparam1 < 0) {
					break;
				}
			}
			break;

// UNARY INSTRUCTIONS
		CASEOP(opNegate)
			thread->push(-thread->pop());
			break;
		CASEOP(opNot)
			thread->push(!thread->pop());
			break;
		CASEOP(opCompl)
			thread->push(~thread->pop());
			break;

		CASEOP(opIncV)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			iparam1 = readUint16(addr, mode);
			writeUint16(addr, iparam1 + 1, mode);
			break;
		CASEOP(opDecV)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			iparam1 = readUint16(addr, mode);
			writeUint16(addr, iparam1 - 1, mode);
			break;
		CASEOP(opPostInc)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			iparam1 = readUint16(addr, mode);
			thread->push(iparam1);
			writeUint16(addr, iparam1 + 1, mode);
			break;
		CASEOP(opPostDec)
			mode = scriptS.readByte();
			addr = thread->baseAddress(mode);
			iparam1 = scriptS.readSint16LE();
			addr += iparam1;
			iparam1 = readUint16(addr, mode);
			thread->push(iparam1);
			writeUint16(addr, iparam1 - 1, mode);
			break;

// ARITHMETIC INSTRUCTIONS
		CASEOP(opAdd)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 += iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opSub)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 -= iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opMul)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 *= iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opDiv)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 /= iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opMod)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 %= iparam2;
			thread->push(iparam1);
			break;

// COMPARISION INSTRUCTIONS
		CASEOP(opEq)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 == iparam2) ? 1 : 0);
			debug(8, "0x%X 0x%X", iparam1, iparam2);
			break;
		CASEOP(opNe)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 != iparam2) ? 1 : 0);
			break;
		CASEOP(opGt)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 > iparam2) ? 1 : 0);
			break;
		CASEOP(opLt)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 < iparam2) ? 1 : 0);
			break;
		CASEOP(opGe)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 >= iparam2) ? 1 : 0);
			break;
		CASEOP(opLe)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 <= iparam2) ? 1 : 0);
			break;

// SHIFT INSTRUCTIONS
		CASEOP(opRsh)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 >>= iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opLsh)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 <<= iparam2;
			thread->push(iparam1);
			break;

// BITWISE INSTRUCTIONS
		CASEOP(opAnd)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 &= iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opOr)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 |= iparam2;
			thread->push(iparam1);
			break;
		CASEOP(opXor)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			iparam1 ^= iparam2;
			thread->push(iparam1);
			break;

// LOGICAL INSTRUCTIONS
		CASEOP(opLAnd)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 && iparam2) ? 1 : 0);
			break;
		CASEOP(opLOr)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push((iparam1 || iparam2) ? 1 : 0);
			break;
		CASEOP(opLXor)
			iparam2 = thread->pop();
			iparam1 = thread->pop();
			thread->push(((iparam1 && !iparam2) || (!iparam1 && iparam2)) ? 1 : 0);
			break;

// GAME INSTRUCTIONS
		CASEOP(opSpeak) {
				int stringsCount;
				uint16 actorId;
				uint16 speechFlags;
				int sampleResourceId = -1;
				int16 first;
				const char *strings[ACTOR_SPEECH_STRING_MAX];

				if (_vm->_actor->isSpeaking()) {
					thread->wait(kWaitTypeSpeech);
					return false;
				}

				stringsCount = scriptS.readByte();
				actorId = scriptS.readUint16LE();
				speechFlags = scriptS.readByte();
				scriptS.readUint16LE(); // x,y skip

				if (stringsCount == 0)
					error("opSpeak stringsCount == 0");

				if (stringsCount > ACTOR_SPEECH_STRING_MAX)
					error("opSpeak stringsCount=0x%X exceed ACTOR_SPEECH_STRING_MAX", stringsCount);

				iparam1 = first = thread->stackTop();
				for (i = 0; i < stringsCount; i++) {
					 iparam1 = thread->pop();
					 strings[i] = thread->_strings->getString(iparam1);
				}
				// now data contains last string index

				if (_vm->getGameId() == GID_ITE_DISK_G) { // special ITE dos
					if ((_vm->_scene->currentSceneNumber() == ITE_DEFAULT_SCENE) &&
						(iparam1 >= 288) && (iparam1 <= (RID_SCENE1_VOICE_END - RID_SCENE1_VOICE_START + 288))) {
						sampleResourceId = RID_SCENE1_VOICE_START + iparam1 - 288;
					}
				} else {
					if (thread->_voiceLUT->voicesCount > first) {
						sampleResourceId = thread->_voiceLUT->voices[first];
					}
				}

				if (sampleResourceId < 0 || sampleResourceId > 4000)
					sampleResourceId = -1;

				if (_vm->getGameType() == GType_ITE && !sampleResourceId)
					sampleResourceId = -1;

				_vm->_actor->actorSpeech(actorId, strings, stringsCount, sampleResourceId, speechFlags);

				if (!(speechFlags & kSpeakAsync)) {
					thread->wait(kWaitTypeSpeech);
				}
			}
			break;
		CASEOP(opDialogBegin)
			if (_conversingThread) {
				thread->wait(kWaitTypeDialogBegin);
				return false;
			}
			_conversingThread = thread;
			_vm->_interface->converseClear();
			break;
		CASEOP(opDialogEnd)
			if (thread == _conversingThread) {
				_vm->_interface->activate();
				_vm->_interface->setMode(kPanelConverse);
				thread->wait(kWaitTypeDialogEnd);
				return false;
			}
			break;
		CASEOP(opReply) {
				const char *str;
				byte replyNum;
				byte flags;
				replyNum = scriptS.readByte();
				flags = scriptS.readByte();
				iparam1 = 0;
				int strID = thread->pop();

				if (flags & kReplyOnce) {
					iparam1 = scriptS.readSint16LE();
					addr = thread->_staticBase + (iparam1 >> 3);
					if (*addr & (1 << (iparam1 & 7))) {
						break;
					}
				}

				str = thread->_strings->getString(strID);
				if (_vm->_interface->converseAddText(str, strID, replyNum, flags, iparam1))
					warning("Error adding ConverseText (%s, %d, %d, %d)", str, replyNum, flags, iparam1);
			}
			break;
		CASEOP(opAnimate)
			scriptS.readUint16LE();
			scriptS.readUint16LE();
			jmpOffset1 = scriptS.readByte();
			thread->_instructionOffset += jmpOffset1;
			break;

		default:
			error("Script::runThread() Invalid opcode encountered 0x%X", operandChar);
		}

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
	}
	return false;
}

} // End of namespace Saga

