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

#include "saga/script.h"

#include "saga/stream.h"
#include "saga/scene.h"
#include "saga/resnames.h"

namespace Saga {

void Script::setFramePtr(SCRIPT_THREAD *thread, int newPtr) {
	thread->framePtr = newPtr;
	dataBuffer(3)->length = ARRAYSIZE(thread->stackBuf) - thread->framePtr;
	dataBuffer(3)->data = (ScriptDataWord *) &(thread->stackBuf[newPtr]);
}

SCRIPT_THREAD *Script::SThreadCreate() {
	SCRIPT_THREAD *new_thread;

	if (!isInitialized()) {
		return NULL;
	}

	new_thread = _threadList.pushFront().operator->();

	new_thread->stackPtr = ARRAYSIZE(new_thread->stackBuf) - 1;
	setFramePtr(new_thread, new_thread->stackPtr);

	new_thread->flags = kTFlagWaiting;
	new_thread->waitType = kWaitTypePause;

	dataBuffer(4)->length = ARRAYSIZE(new_thread->threadVars);
	dataBuffer(4)->data = new_thread->threadVars;

	return new_thread;
}

void Script::wakeUpActorThread(int waitType, void *threadObj) {
	SCRIPT_THREAD *thread;
	ScriptThreadList::iterator threadIterator;

	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		thread = threadIterator.operator->();
		if ((thread->flags & kTFlagWaiting) && (thread->waitType == waitType) && (thread->threadObj == threadObj)) {
			thread->flags &= ~kTFlagWaiting;
		}
	}
}

void Script::wakeUpThreads(int waitType) {
	SCRIPT_THREAD *thread;
	ScriptThreadList::iterator threadIterator;
	
	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		thread = threadIterator.operator->();
		if ((thread->flags & kTFlagWaiting) && (thread->waitType == waitType)) {
			thread->flags &= ~kTFlagWaiting;
		}
	}
}

void Script::wakeUpThreadsDelayed(int waitType, int sleepTime) {
	SCRIPT_THREAD *thread;
	ScriptThreadList::iterator threadIterator;

	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		thread = threadIterator.operator->();
		if ((thread->flags & kTFlagWaiting) && (thread->waitType == waitType)) {
			thread->waitType = kWaitTypeDelay;
			thread->sleepTime = sleepTime;
		}
	}
}

int Script::executeThreads(uint msec) {
	SCRIPT_THREAD *thread;
	ScriptThreadList::iterator threadIterator;

	if (!isInitialized()) {
		return FAILURE;
	}

	threadIterator = _threadList.begin();

	while (threadIterator != _threadList.end()) {
		thread = threadIterator.operator->();

		if (thread->flags & (kTFlagFinished | kTFlagAborted)) {
			//if (thread->flags & kTFlagFinished) // FIXME. Missing function

			
			threadIterator = _threadList.erase(threadIterator);
			continue;
		}

		if (thread->flags & kTFlagWaiting) {
			
			if (thread->waitType == kWaitTypeDelay) {
				if (thread->sleepTime < msec) {
					thread->sleepTime = 0;
				} else {
					thread->sleepTime -= msec;
				}

				if (thread->sleepTime == 0)
					thread->flags &= ~kTFlagWaiting;			
			} else {
				if (thread->waitType == kWaitTypeWalk) {
					ActorData *actor;
					actor = (ActorData *)thread->threadObj;
					if (actor->currentAction == kActionWait) {
						thread->flags &= ~kTFlagWaiting;			
					}
				}
			}
		}

		if (!(thread->flags & kTFlagWaiting))
			SThreadRun(thread, STHREAD_TIMESLICE);

		++threadIterator;
	}

	return SUCCESS;
}

void Script::SThreadCompleteThread(void) {
	for (int i = 0; i < 40 &&  !_threadList.isEmpty() ; i++)
		executeThreads(0);
}

int Script::SThreadSetEntrypoint(SCRIPT_THREAD *thread, int ep_num) {
	SCRIPT_BYTECODE *bytecode;
	int max_entrypoint;

	assert(isInitialized());

	bytecode = currentScript()->bytecode;
	max_entrypoint = bytecode->n_entrypoints;

	if ((ep_num < 0) || (ep_num >= max_entrypoint)) {
		return FAILURE;
	}

	thread->ep_num = ep_num;
	thread->ep_offset = bytecode->entrypoints[ep_num].offset;

	return SUCCESS;
}

int Script::SThreadExecute(SCRIPT_THREAD *thread, int ep_num) {
	assert(isInitialized());

	if ((currentScript() == NULL) || (!currentScript()->loaded)) {
		return FAILURE;
	}

	SThreadSetEntrypoint(thread, ep_num);

	thread->i_offset = thread->ep_offset;
	thread->flags = kTFlagNone;

	return SUCCESS;
}



unsigned char *Script::SThreadGetReadPtr(SCRIPT_THREAD *thread) {
	return currentScript()->bytecode->bytecode_p + thread->i_offset;
}

unsigned long Script::SThreadGetReadOffset(const byte *read_p) {
	return (unsigned long)(read_p - (unsigned char *)currentScript()->bytecode->bytecode_p);
}

size_t Script::SThreadGetReadLen(SCRIPT_THREAD *thread) {
	return currentScript()->bytecode->bytecode_len - thread->i_offset;
}




int Script::SThreadDebugStep() {
	if (_dbg_singlestep) {
		_dbg_dostep = 1;
	}

	return SUCCESS;
}

int Script::SThreadRun(SCRIPT_THREAD *thread, int instr_limit) {
	int instr_count;
	uint32 saved_offset;
	ScriptDataWord param1;
	ScriptDataWord param2;
	long iparam1;
	long iparam2;
	long iresult;

	ScriptDataWord data;
	ScriptDataWord scriptRetVal = 0;
	int debug_print = 0;
	int n_buf;
	int bitstate;
	int operandChar;
	int i;
	int unhandled = 0;

	// Handle debug single-stepping
	if ((thread == _dbg_thread) && _dbg_singlestep) {
		if (_dbg_dostep) {
			debug_print = 1;
			thread->sleepTime = 0;
			instr_limit = 1;
			_dbg_dostep = 0;
		} else {
			return SUCCESS;
		}
	}

	MemoryReadStream/*Endian*/ scriptS(currentScript()->bytecode->bytecode_p, currentScript()->bytecode->bytecode_len/*, IS_BIG_ENDIAN*/);

	dataBuffer(2)->length = currentScript()->bytecode->bytecode_len / sizeof(ScriptDataWord);
	dataBuffer(2)->data = (ScriptDataWord *) currentScript()->bytecode->bytecode_p;

	scriptS.seek(thread->i_offset);

	for (instr_count = 0; instr_count < instr_limit; instr_count++) {
		if (thread->flags & kTFlagWaiting)
			break;

		saved_offset = thread->i_offset;
		operandChar = scriptS.readByte();

//		debug(2, "Executing thread offset: %lu (%x) stack: %d", thread->i_offset, operandChar, thread->stackSize());
		switch (operandChar) {
		case 0x01: // nextblock
			// Some sort of "jump to the start of the next memory
			// page" instruction, I think.
			thread->i_offset = 1024 * ((thread->i_offset / 1024) + 1);
			break;

// STACK INSTRUCTIONS
		case 0x02: // Dup top element (DUP)
			thread->push(thread->stackTop());
			break;
		case 0x03: // Pop nothing (POPN)
			thread->pop();
			break;
		case 0x04: // Push false (PSHF)
			thread->push(0);
			break;
		case 0x05: // Push true (PSHT)
			thread->push(1);
			break;
		case 0x06: // Push word (PUSH)
		case 0x08: // Push word (PSHD) (dialogue string index)
			param1 = (ScriptDataWord)scriptS.readUint16LE();
			thread->push(param1);
			break;

// DATA INSTRUCTIONS  

		case 0x0B: // Test flag (TSTF)
			n_buf = scriptS.readByte();
			param1 = (ScriptDataWord)scriptS.readUint16LE();
			getBit(n_buf, param1, &bitstate);
			thread->push(bitstate);
			break;
		case 0x0C: // Get word (GETW)
			n_buf = scriptS.readByte();
			param1 = scriptS.readUint16LE();
			getWord(n_buf, param1, &data);
			thread->push(data);
			break;
		case 0x0F: // Modify flag (MODF)
			n_buf = scriptS.readByte();
			param1 = (ScriptDataWord)scriptS.readUint16LE();
			bitstate = getUWord(param1);
			data = thread->stackTop();
			if (bitstate) {
				setBit(n_buf, data, 1);
			} else {
				setBit(n_buf, data, 0);
			}
			break;
		case 0x10: // Put word (PUTW)
			n_buf = scriptS.readByte();
			param1 = (ScriptDataWord)scriptS.readUint16LE();
			data = thread->stackTop();
			putWord(n_buf, param1, data);
			break;
		case 0x13: // Modify flag and pop (MDFP)
			n_buf = scriptS.readByte();
			param1 = (ScriptDataWord)scriptS.readUint16LE();
			param1 = thread->pop();
			bitstate = getUWord(param1);
			if (bitstate) {
				setBit(n_buf, param1, 1);
			} else {
				setBit(n_buf, param1, 0);
			}
			break;
		case 0x14: // Put word and pop (PTWP)
			n_buf = scriptS.readByte();
			param1 = (ScriptDataWord)scriptS.readUint16LE();
			data = thread->stackTop();
			putWord(n_buf, param1, data);
			break;

// CONTROL INSTRUCTIONS    

		case 0x17: // (GOSB): Call subscript
			{
				int n_args;
				int temp;

				n_args = scriptS.readByte();
				temp = scriptS.readByte();
				if (temp != 2)
					error("Calling dynamically generated script? Wow");
				param1 = (ScriptDataWord)scriptS.readUint16LE();
				data = scriptS.pos();
				thread->push(n_args);
				// NOTE: The original pushes the program
				// counter as a pointer here. But I don't think
				// we will have to do that.
				thread->push(data);
				thread->i_offset = (unsigned long)param1;
			}
			break;
		case opCcall: // (CALL): Call function
		case opCcallV: // (CALL_V): Call function and discard return value
			{
				int argumentsCount;
				uint16 functionNumber;
				int scriptFunctionReturnValue;
				ScriptFunctionType scriptFunction;

				argumentsCount = scriptS.readByte();
				functionNumber = scriptS.readUint16LE();
				if (functionNumber >= SCRIPT_FUNCTION_MAX) {
					_vm->_console->DebugPrintf(S_ERROR_PREFIX "Invalid script function number: (%X)\n", functionNumber);
					thread->flags |= kTFlagAborted;
					break;
				}

				debug(9, "opCCall* Calling 0x%X %s", functionNumber, _scriptFunctionsList[functionNumber].scriptFunctionName);
				scriptFunction = _scriptFunctionsList[functionNumber].scriptFunction;
				scriptFunctionReturnValue = (this->*scriptFunction)(thread, argumentsCount);
				if (scriptFunctionReturnValue != SUCCESS) {
					_vm->_console->DebugPrintf(S_WARN_PREFIX "%X: Script function %d failed.\n", thread->i_offset, scriptFunctionReturnValue);
				}

				if (functionNumber == 16) { // SF_gotoScene
					instr_count = instr_limit; // break the loop
					break;
				}

				if (operandChar == 0x18) // CALL function
					thread->push(thread->retVal);

				if (thread->flags & kTFlagAsleep)
					instr_count = instr_limit;	// break out of loop!
			}
			break;
		case 0x1A: // (ENTR) Enter the dragon
			thread->push(thread->framePtr);
			setFramePtr(thread, thread->stackPtr);
			param1 = scriptS.readUint16LE();
			thread->stackPtr -= (param1 / 2);
			break;
		case 0x1B: // Return with value
			scriptRetVal = thread->pop();
			// FALL THROUGH
		case 0x1C: // Return with void
			thread->stackPtr = thread->framePtr;
			setFramePtr(thread, thread->pop());
			if (thread->stackSize() == 0) {
				_vm->_console->DebugPrintf("Script execution complete.\n");
				thread->flags |= kTFlagFinished;
			} else {
				thread->i_offset = thread->pop();
				/* int n_args = */ thread->pop();
				if (operandChar == 0x1B)
					thread->push(scriptRetVal);
			}
			break;

// BRANCH INSTRUCTIONS    

			// (JMP): Unconditional jump
		case 0x1D:
			param1 = scriptS.readUint16LE();
			thread->i_offset = (unsigned long)param1;
			break;
			// (JNZP): Jump if nonzero + POP
		case 0x1E:
			param1 = scriptS.readUint16LE();
			data = thread->pop();
			if (data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JZP): Jump if zero + POP
		case 0x1F:
			param1 = scriptS.readUint16LE();
			data = thread->pop();
			if (!data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JNZ): Jump if nonzero
		case 0x20:
			param1 = scriptS.readUint16LE();
			data = thread->stackTop();
			if (data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JZ): Jump if zero
		case 0x21:
			param1 = scriptS.readUint16LE();
			data = thread->stackTop();
			if (!data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (SWCH): Switch
		case 0x22:
			{
				int n_switch;
				unsigned int switch_num;
				unsigned int switch_jmp;
				unsigned int default_jmp;
				int case_found = 0;

				data = thread->pop();
				n_switch = scriptS.readUint16LE();
				for (i = 0; i < n_switch; i++) {
					switch_num = scriptS.readUint16LE();
					switch_jmp = scriptS.readUint16LE();
					// Found the specified case
					if (data == (ScriptDataWord) switch_num) {
						thread->i_offset = switch_jmp;
						case_found = 1;
						break;
					}
				}

				// Jump to default case
				if (!case_found) {
					default_jmp = scriptS.readUint16LE();
					thread->i_offset = default_jmp;
				}
			}
			break;
			// (RJMP): Random branch
		case 0x24:
			{
				int n_branch;
				unsigned int branch_wt;
				unsigned int branch_jmp;
				int rand_sel = 0;
				int branch_found = 0;

				// Ignored?
				scriptS.readUint16LE();
				n_branch = scriptS.readUint16LE();
				for (i = 0; i < n_branch; i++) {
					branch_wt = scriptS.readUint16LE();
					branch_jmp = scriptS.readUint16LE();
					if (rand_sel == i) {
						thread->i_offset = branch_jmp;
						branch_found = 1;
						break;
					}
				}
				if (!branch_found) {
					_vm->_console->DebugPrintf(S_ERROR_PREFIX "%X: Random jump target out of bounds.\n", thread->i_offset);
				}
			}
			break;

// UNARY INSTRUCTIONS

			// (NEG) Negate stack by 2's complement
		case 0x25:
			data = thread->pop();
			data = ~data;
			data++;
			thread->push(data);
			break;
			// (TSTZ) Test for zero
		case 0x26:
			data = thread->pop();
			data = data ? 0 : 1;
			thread->push(data);
			break;
			// (NOT) Binary not
		case 0x27:
			data = thread->pop();
			data = ~data;
			thread->push(data);
			break;
		case 0x28: // inc_v increment, don't push
			unhandled = 1;
			//debug(2, "??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;
		case 0x29: // dec_v decrement, don't push
			unhandled = 1;
			//debug(2, "??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;
		case 0x2A: // postinc
			unhandled = 1;
			//debug(2, "??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;
		case 0x2B: // postdec
			unhandled = 1;
			//debug(2, "??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;

// ARITHMETIC INSTRUCTIONS    

			// (ADD): Addition
		case 0x2C:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 + iparam2;
			thread->push((ScriptDataWord) iresult);
			break;
			// (SUB): Subtraction
		case 0x2D:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 - iparam2;
			thread->push((ScriptDataWord) iresult);
			break;
			// (MULT): Integer multiplication
		case 0x2E:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 * iparam2;
			thread->push((ScriptDataWord) iresult);
			break;
			// (DIV): Integer division
		case 0x2F:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 / iparam2;
			thread->push((ScriptDataWord) iresult);
			break;
			// (MOD) Modulus
		case 0x30:
			param2 = thread->pop();
			param1 = thread->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 % iparam2;
			thread->push((ScriptDataWord) iresult);
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
			data = (0x01 << ((sizeof param1 * CHAR_BIT) - 1)) & param1;
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
					return SUCCESS;
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
					 strings[i] = getString(data);
				}
				// now data contains last string index

				if (_vm->_gameId == GID_ITE_DISK_G) { // special ITE dos
					if ((_vm->_scene->currentSceneNumber() == ITE_DEFAULT_SCENE) && (data >= 288) && (data <= (RID_SCENE1_VOICE_138 - RID_SCENE1_VOICE_009 + 288))) {
						sampleResourceId = RID_SCENE1_VOICE_009 + data - 288;
					}
				} else {
					if (isVoiceLUTPresent()) {
						if (currentScript()->voice->n_voices > first) {
							sampleResourceId = currentScript()->voice->voices[first];
						}
					}
				}

				_vm->_actor->actorSpeech(actorId, strings, stringsCount, sampleResourceId, speechFlags);				

				if (!(speechFlags & kSpeakAsync)) {
					thread->wait(kWaitTypeSpeech);
				}
			}
			break;
			// (DLGS): Initialize dialogue interface
		case 0x54:
			warning("dialog_begin opcode: stub");
			break;
			// (DLGX): Run dialogue interface
		case 0x55:
			warning("dialog_end opcode: stub");
			break;
			// (DLGO): Add a dialogue option to interface
		case 0x56:
			{
				ScriptDataWord param3 = 0;
				param1 = scriptS.readByte();
				param2 = scriptS.readByte();
				if (param2 & 1) {
					param3 = scriptS.readUint16LE();
				}
				debug(2, "DLGO | %02X %02X %04X", param1, param2, param3);
			}
			break;
		case 0x57: // animate
			scriptS.readUint16LE();
			scriptS.readUint16LE();
			iparam1 = (long)scriptS.readByte();
			thread->i_offset += iparam1;
			break;

// End instruction list

		default:
			_vm->_console->DebugPrintf(S_ERROR_PREFIX "%X: Invalid opcode encountered: (%X).\n", thread->i_offset, operandChar);
			thread->flags |= kTFlagAborted;
			break;
		}

		// Set instruction offset only if a previous instruction didn't branch
		if (saved_offset == thread->i_offset) {
			thread->i_offset = scriptS.pos();
		} else {
			if (thread->i_offset >= scriptS.size()) {
				_vm->_console->DebugPrintf("Out of range script execution at %x size %x\n", thread->i_offset, scriptS.size());
				thread->flags |= kTFlagFinished;
			}
			else
				scriptS.seek(thread->i_offset);
		}
		if (unhandled) {
			_vm->_console->DebugPrintf(S_ERROR_PREFIX "%X: Unhandled opcode.\n", thread->i_offset);
			thread->flags |= kTFlagAborted;
		}
		if ((thread->flags == kTFlagNone) && debug_print) {
			SDebugPrintInstr(thread);
		}
	}

	return SUCCESS;
}

} // End of namespace Saga

