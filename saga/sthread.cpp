/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scripting module thread management component
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/interface.h"

#include "saga/script.h"

#include "saga/stream.h"
#include "saga/scene.h"
#include "saga/resnames.h"

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
	newThread->_stackTopIndex = newThread->_stackSize - 1; // or 2 - as in original
	newThread->_instructionOffset = _modules[scriptModuleNumber].entryPoints[scriptEntryPointNumber].offset;
	newThread->_commonBase = _commonBuffer;
	newThread->_staticBase = _commonBuffer + _modules[scriptModuleNumber].staticOffset;
	newThread->_moduleBase = _modules[scriptModuleNumber].moduleBase;
	newThread->_moduleBaseSize = _modules[scriptModuleNumber].moduleBaseSize;
	
	newThread->_strings = &_modules[scriptModuleNumber].strings;
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

int Script::executeThreads(uint msec) {
	ScriptThread *thread;
	ScriptThreadList::iterator threadIterator;

	if (!isInitialized()) {
		return FAILURE;
	}

	threadIterator = _threadList.begin();

	while (threadIterator != _threadList.end()) {
		thread = threadIterator.operator->();

		if (thread->_flags & (kTFlagFinished | kTFlagAborted)) {
			if (thread->_flags & kTFlagFinished)
				setPointerVerb();
			
			threadIterator = _threadList.erase(threadIterator);
			continue;
		}

		if (thread->_flags & kTFlagWaiting) {
			
			if (thread->_waitType == kWaitTypeDelay) {
				if (thread->_sleepTime < msec) {
					thread->_sleepTime = 0;
				} else {
					thread->_sleepTime -= msec;
				}

				if (thread->_sleepTime == 0)
					thread->_flags &= ~kTFlagWaiting;			
			} else {
				if (thread->_waitType == kWaitTypeWalk) {
					ActorData *actor;
					actor = (ActorData *)thread->_threadObj;
					if (actor->currentAction == kActionWait) {
						thread->_flags &= ~kTFlagWaiting;			
					}
				}
			}
		}

		if (!(thread->_flags & kTFlagWaiting))
			runThread(thread, STHREAD_TIMESLICE);

		++threadIterator;
	}

	return SUCCESS;
}

void Script::completeThread(void) {
	for (int i = 0; i < 40 &&  !_threadList.isEmpty() ; i++)
		executeThreads(0);
}

int Script::SThreadDebugStep() {
	if (_dbg_singlestep) {
		_dbg_dostep = 1;
	}

	return SUCCESS;
}

void Script::runThread(ScriptThread *thread, uint instructionLimit) {
	uint instructionCount;
	uint16 savedInstructionOffset;

	byte *addr;
	uint16 param1;
	uint16 param2;
	int16 iparam1;
	int16 iparam2;
	long iresult;

	byte argumentsCount;
	uint16 functionNumber;
	int scriptFunctionReturnValue;
	ScriptFunctionType scriptFunction;

	uint16 data;
	int debug_print = 0;
	int n_buf;
//	int bitstate;
	int operandChar;
	int i;
	int unhandled = 0;

	// Handle debug single-stepping
	if ((thread == _dbg_thread) && _dbg_singlestep) {
		if (_dbg_dostep) {
			debug_print = 1;
			thread->_sleepTime = 0;
			instructionLimit = 1;
			_dbg_dostep = 0;
		} else {
			return;
		}
	}

	MemoryReadStream scriptS(thread->_moduleBase, thread->_moduleBaseSize);

	scriptS.seek(thread->_instructionOffset);

	for (instructionCount = 0; instructionCount < instructionLimit; instructionCount++) {
		if (thread->_flags & (kTFlagAsleep))
			break;

		savedInstructionOffset = thread->_instructionOffset;
		operandChar = scriptS.readByte();

#define CASEOP(opName)	case opName:									\
							if (operandChar == opName) {				\
								debug(8, #opName);						\
								_vm->_console->DebugPrintf(#opName);	\
							}
						
//		debug print (opCode name etc) should be placed here
//		SDebugPrintInstr(thread)

		debug(8, "Executing thread offset: %lu (%x) stack: %d", thread->_instructionOffset, operandChar, thread->pushedSize());
		switch (operandChar) {
		CASEOP(opNextBlock)
			// Some sort of "jump to the start of the next memory
			// page" instruction, I think.
			thread->_instructionOffset = 1024 * ((thread->_instructionOffset / 1024) + 1);
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
			param1 = scriptS.readUint16LE();
			thread->push(param1);
			break;

// DATA INSTRUCTIONS  
		CASEOP(opGetFlag)
			addr = thread->baseAddress(scriptS.readByte());
			param1 = scriptS.readUint16LE();
			addr += (param1 >> 3);
			param1 = (1 << (param1 & 7));
			thread->push((*addr) & param1 ? 1 : 0);
			break;
		CASEOP(opGetInt)
			addr = thread->baseAddress(scriptS.readByte());
			param1 = scriptS.readUint16LE();
			addr += param1;
			thread->push(*((uint16*)addr));
			break;
		CASEOP(opPutFlag)
			addr = thread->baseAddress(scriptS.readByte());
			param1 = scriptS.readUint16LE();
			addr += (param1 >> 3);
			param1 = (1 << (param1 & 7));
			if (thread->stackTop()) {
				*addr |= param1;
			} else {
				*addr &= ~param1;
			}
			break;
		CASEOP(opPutInt)
			addr = thread->baseAddress(scriptS.readByte());
			param1 = scriptS.readUint16LE();
			addr += param1;
			*(uint16*)addr =  thread->stackTop();
			break;
		CASEOP(opPutFlagV)
			addr = thread->baseAddress(scriptS.readByte());
			param1 = scriptS.readUint16LE();
			addr += (param1 >> 3);
			param1 = (1 << (param1 & 7));
			if (thread->pop()) {
				*addr |= param1;
			} else {
				*addr &= ~param1;
			}
			break;
		CASEOP(opPutIntV)
			addr = thread->baseAddress(scriptS.readByte());
			param1 = scriptS.readUint16LE();
			addr += param1;
			*(uint16*)addr =  thread->pop();
			break;

// CONTROL INSTRUCTIONS    

		CASEOP(opCall)
			argumentsCount = scriptS.readByte();
			param1 = scriptS.readByte();
			if (param1 != kAddressModule) {
				error("Script::runThread param1 != kAddressModule");
			}
			addr = thread->baseAddress(param1);
			param1 = scriptS.readUint16LE();
			addr += param1;
			thread->push(argumentsCount);

			param2 = scriptS.pos();
			// NOTE: The original pushes the program
			// counter as a pointer here. But I don't think
			// we will have to do that.
			thread->push(param2);
			thread->_instructionOffset = param1;
			
			break;
		CASEOP(opCcall)
		CASEOP(opCcallV)
			argumentsCount = scriptS.readByte();
			functionNumber = scriptS.readUint16LE();
			if (functionNumber >= SCRIPT_FUNCTION_MAX) {
				error("Script::runThread() Invalid script function number");
			}

			debug(8, "Calling 0x%X %s", functionNumber, _scriptFunctionsList[functionNumber].scriptFunctionName);
			scriptFunction = _scriptFunctionsList[functionNumber].scriptFunction;
			scriptFunctionReturnValue = (this->*scriptFunction)(thread, argumentsCount);
			if (scriptFunctionReturnValue != SUCCESS) {
				_vm->_console->DebugPrintf(S_WARN_PREFIX "%X: Script function %d failed.\n", thread->_instructionOffset, scriptFunctionReturnValue);
			}

			if (functionNumber ==  16) { // SF_gotoScene
				instructionCount = instructionLimit; // break the loop
				break;
			}

			if (operandChar == opCcall) {// CALL function
				thread->push(thread->_returnValue);
			}

			if (thread->_flags & kTFlagAsleep)
				instructionCount = instructionLimit;	// break out of loop!
			break;
		CASEOP(opEnter)
			thread->push(thread->_frameIndex);
			thread->_frameIndex = thread->_stackTopIndex;
			thread->_stackTopIndex -= (scriptS.readUint16LE() / 2);
			break;
		CASEOP(opReturn)
			thread->_returnValue = thread->pop();
		CASEOP(opReturnV)
			thread->_stackTopIndex = thread->_frameIndex;
			thread->_frameIndex = thread->pop();
			if (thread->pushedSize() == 0) {
				_vm->_console->DebugPrintf("Script execution complete.\n");
				thread->_flags |= kTFlagFinished;
				return;
			} else {
				thread->_instructionOffset = thread->pop();
				param1 = thread->pop();
				param1 +=param1;
				while (param1--) {
					thread->pop();
				}

				if (operandChar == opReturn) {
					thread->push(thread->_returnValue);
				}
			}
			break;

// BRANCH INSTRUCTIONS    

			// (JMP): Unconditional jump
		case 0x1D:
			param1 = scriptS.readUint16LE();
			thread->_instructionOffset = (unsigned long)param1;
			break;
			// (JNZP): Jump if nonzero + POP
		case 0x1E:
			param1 = scriptS.readUint16LE();
			data = thread->pop();
			if (data) {
				thread->_instructionOffset = (unsigned long)param1;
			}
			break;
			// (JZP): Jump if zero + POP
		case 0x1F:
			param1 = scriptS.readUint16LE();
			data = thread->pop();
			if (!data) {
				thread->_instructionOffset = (unsigned long)param1;
			}
			break;
			// (JNZ): Jump if nonzero
		case 0x20:
			param1 = scriptS.readUint16LE();
			data = thread->stackTop();
			if (data) {
				thread->_instructionOffset = (unsigned long)param1;
			}
			break;
			// (JZ): Jump if zero
		case 0x21:
			param1 = scriptS.readUint16LE();
			data = thread->stackTop();
			if (!data) {
				thread->_instructionOffset = (unsigned long)param1;
			}
			break;
			// (SWCH): Switch
		case 0x22:
			{
				int n_switch;
				uint16 switch_num;
				unsigned int switch_jmp;
				unsigned int default_jmp;
				int case_found = 0;

				data = thread->pop();
				n_switch = scriptS.readUint16LE();

				for (i = 0; i < n_switch; i++) {
					switch_num = scriptS.readUint16LE();
					switch_jmp = scriptS.readUint16LE();
					// Found the specified case
					if (data == switch_num) {
						thread->_instructionOffset = switch_jmp;
						case_found = 1;
						break;
					}
				}

				// Jump to default case
				if (!case_found) {
					default_jmp = scriptS.readUint16LE();
					thread->_instructionOffset = default_jmp;
				}
			}
			break;
			// (RJMP): Random branch
		case 0x24:
			{
				// Supposedly the number of possible branches.
				// The original interpreter ignores it.
				scriptS.readUint16LE();

				uint16 probability = _vm->_rnd.getRandomNumber(scriptS.readUint16LE() - 1);

				while (1) {
					uint16 branch_probability = scriptS.readUint16LE();
					uint16 offset = scriptS.readUint16LE();

					if (branch_probability > probability) {
						thread->_instructionOffset = offset;
						break;
					}

					probability -= branch_probability;
				}
			}
			break;

// UNARY INSTRUCTIONS

			// (NEG) Negate stack by 2's complement
		case 0x25:
			data = thread->pop();
			thread->push(-(int)data);
			break;
			// (TSTZ) Test for zero
		case 0x26:
			data = thread->pop();
			thread->push(!data);
			break;
			// (NOT) Binary not
		case 0x27:
			data = thread->pop();
			thread->push(~data);
			break;
		case 0x28: // inc_v increment, don't push
			n_buf = scriptS.readByte();
			param1 = scriptS.readUint16LE();
			//getWord(n_buf, param1, &data);
			//putWord(n_buf, param1, data + 1);
			break;
		case 0x29: // dec_v decrement, don't push
			n_buf = scriptS.readByte();
			param1 = scriptS.readUint16LE();
			//getWord(n_buf, param1, &data);
			//putWord(n_buf, param1, data - 1);
			break;
		case 0x2A: // postinc
			n_buf = scriptS.readByte();
			param1 = scriptS.readUint16LE();
			//getWord(n_buf, param1, &data);
//			thread->push(data);
			//putWord(n_buf, param1, data + 1);
			break;
		case 0x2B: // postdec
			n_buf = scriptS.readByte();
			param1 = scriptS.readUint16LE();
			//getWord(n_buf, param1, &data);
//			thread->push(data);
			//putWord(n_buf, param1, data - 1);
			break;

// ARITHMETIC INSTRUCTIONS    

			// (ADD): Addition
		case 0x2C:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 + iparam2;
			thread->push(iresult);
			break;
			// (SUB): Subtraction
		case 0x2D:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 - iparam2;
			thread->push(iresult);
			break;
			// (MULT): Integer multiplication
		case 0x2E:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 * iparam2;
			thread->push(iresult);
			break;
			// (DIV): Integer division
		case 0x2F:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 / iparam2;
			thread->push(iresult);
			break;
			// (MOD) Modulus
		case 0x30:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 % iparam2;
			thread->push(iresult);
			break;
			// (EQU) Test equality
		case 0x33:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 == iparam2) ? 1 : 0;
			thread->push(data);
			break;
			// (NEQU) Test inequality
		case 0x34:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 != iparam2) ? 1 : 0;
			thread->push(data);
			break;
			// (GRT) Test Greater-than
		case 0x35:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 > iparam2) ? 1 : 0;
			thread->push(data);
			break;
			// (LST) Test Less-than
		case 0x36:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 < iparam2) ? 1 : 0;
			thread->push(data);
			break;
			// (GRTE) Test Greater-than or Equal to
		case 0x37:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 >= iparam2) ? 1 : 0;
			thread->push(data);
			break;
			// (LSTE) Test Less-than or Equal to
		case 0x38:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 <= iparam2) ? 1 : 0;
			thread->push(data);
			break;

// BITWISE INSTRUCTIONS   

			// (SHR): Arithmetic binary shift right
		case 0x3F:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			// Preserve most significant bit
			data = (0x01 << ((sizeof(param1) * CHAR_BIT) - 1)) & param1;
			for (i = 0; i < (int)iparam2; i++) {
				param1 >>= 1;
				param1 |= data;
			}
			thread->push(param1);
			break;
			// (SHL) Binary shift left
		case 0x40:
			param2 = thread->pop();
			param1 = thread->pop();
			param1 <<= param2;
			thread->push(param1);
			break;
			// (AND) Binary AND
		case 0x41:
			param2 = thread->pop();
			param1 = thread->pop();
			param1 &= param2;
			thread->push(param1);
			break;
			// (OR) Binary OR
		case 0x42:
			param2 = thread->pop();
			param1 = thread->pop();
			param1 |= param2;
			thread->push(param1);
			break;
			// (XOR) Binary XOR
		case 0x43:
			param2 = thread->pop();
			param1 = thread->pop();
			param1 ^= param2;
			thread->push(param1);
			break;

// BOOLEAN LOGIC INSTRUCTIONS     

			// (LAND): Logical AND
		case 0x44:
			param2 = thread->pop();
			param1 = thread->pop();
			data = (param1 && param2) ? 1 : 0;
			thread->push(data);
			break;
			// (LOR): Logical OR
		case 0x45:
			param2 = thread->pop();
			param1 = thread->pop();
			data = (param1 || param2) ? 1 : 0;
			thread->push(data);
			break;
			// (LXOR): Logical XOR
		case 0x46:
			param2 = thread->pop();
			param1 = thread->pop();
			data = ((param1) ? !(param2) : !!(param2));
			thread->push(data);
			break;

// GAME INSTRUCTIONS  
		case opSpeak: {	// (opSpeak): Play Character Speech
				int stringsCount;
				uint16 actorId;
				int speechFlags;
				int sampleResourceId = -1;
				int first;
				const char *strings[ACTOR_SPEECH_STRING_MAX];

				if (_vm->_actor->isSpeaking()) {
					thread->wait(kWaitTypeSpeech);
					return;
				}

				stringsCount = scriptS.readByte();
				actorId =  scriptS.readUint16LE();
				speechFlags = scriptS.readByte();
				scriptS.readUint16LE(); // x,y skip
				
				if (stringsCount == 0)
					error("opSpeak stringsCount == 0");

				if (stringsCount > ACTOR_SPEECH_STRING_MAX)
					error("opSpeak stringsCount=0x%X exceed ACTOR_SPEECH_STRING_MAX", stringsCount);
				
				data = first = thread->stackTop();
				for (i = 0; i < stringsCount; i++) {
					 data = thread->pop();
					 strings[i] = thread->_strings->getString(data);
				}
				// now data contains last string index

				if (_vm->getGameId() == GID_ITE_DISK_G) { // special ITE dos
					if ((_vm->_scene->currentSceneNumber() == ITE_DEFAULT_SCENE) && (data >= 288) && (data <= (RID_SCENE1_VOICE_138 - RID_SCENE1_VOICE_009 + 288))) {
						sampleResourceId = RID_SCENE1_VOICE_009 + data - 288;
					}
				} else {
					if (thread->_voiceLUT->voicesCount > first) {
						sampleResourceId = thread->_voiceLUT->voices[first];
					}
				}

				_vm->_actor->actorSpeech(actorId, strings, stringsCount, sampleResourceId, speechFlags);				

				if (!(speechFlags & kSpeakAsync)) {
					thread->wait(kWaitTypeSpeech);
					thread->_instructionOffset = scriptS.pos();
					return;
				}
			}
			break;

		case opDialogBegin: // (DLGS): Initialize dialogue interface
			if (_conversingThread) {
				thread->wait(kWaitTypeDialogBegin);
				return;
			}
			_conversingThread = thread;
			_vm->_interface->converseClear();
			break;

		case opDialogEnd: // (DLGX): Run dialogue interface
			if (thread == _conversingThread) {
				_vm->_interface->activate();
				_vm->_interface->setMode(kPanelConverse);
				thread->wait(kWaitTypeDialogEnd);
				return;
			}
			break;

		case opReply: // (DLGO): Add a dialogue option to interface
			{
				uint16 n = 0;
				const char *str;
				int replyNum = scriptS.readByte();
				int flags = scriptS.readByte();

				if (flags & kReplyOnce) {
					n = scriptS.readUint16LE();
					// TODO:
				}

				str = thread->_strings->getString(thread->pop());
				if (_vm->_interface->converseAddText(str, replyNum, flags, n))
					warning("Error adding ConverseText (%s, %d, %d, %d)", str, replyNum, flags, n);
			}
			break;
		case 0x57: // animate
			scriptS.readUint16LE();
			scriptS.readUint16LE();
			iparam1 = (long)scriptS.readByte();
			thread->_instructionOffset += iparam1;
			break;

// End instruction list

		default:
			scriptError(thread, "Invalid opcode encountered");
			return;
		}

		// Set instruction offset only if a previous instruction didn't branch
		if (savedInstructionOffset == thread->_instructionOffset) {
			thread->_instructionOffset = scriptS.pos();
		} else {
			if (thread->_instructionOffset >= scriptS.size()) {
				scriptError(thread, "Out of range script execution");
				return;
			}

			scriptS.seek(thread->_instructionOffset);
		}

		if (unhandled) { // TODO: remove it
			scriptError(thread, "Unhandled opcode");
		}
	}
}

} // End of namespace Saga

