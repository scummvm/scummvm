/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
#include "saga/yslib.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"

#include "saga/script.h"

#include "saga/sdata.h"

#include "common/stack.h"

namespace Saga {

R_SCRIPT_THREAD *Script::SThreadCreate() {
	YS_DL_NODE *new_node;
	R_SCRIPT_THREAD *new_thread;

	if (!isInitialized()) {
		return NULL;
	}

	new_thread = (R_SCRIPT_THREAD *)calloc(1, sizeof *new_thread);
	if (new_thread == NULL) {
		return NULL;
	}

	new_thread->stack = new Common::Stack<SDataWord_T>();

	new_node = ys_dll_add_head(threadList(), new_thread, sizeof *new_thread);

	free(new_thread);

	return (R_SCRIPT_THREAD *)ys_dll_get_data(new_node);
}

int Script::SThreadDestroy(R_SCRIPT_THREAD *thread) {
	if (thread == NULL) {
		return R_FAILURE;
	}

	delete thread->stack;

	return R_SUCCESS;
}

int Script::SThreadExecThreads(int msec) {
	YS_DL_NODE *walk_p;
	R_SCRIPT_THREAD *thread;

	if (!isInitialized()) {
		return R_FAILURE;
	}

	for (walk_p = ys_dll_head(threadList()); walk_p != NULL; walk_p = ys_dll_next(walk_p)) {
		thread = (R_SCRIPT_THREAD *)ys_dll_get_data(walk_p);
		if (thread->executing) {
			SThreadRun(thread, STHREAD_DEF_INSTR_COUNT, msec);
		}
	}

	return R_SUCCESS;
}

void Script::SThreadCompleteThread(void) {
	for (int i = 0; i < 40 && (ys_dll_head(threadList()) != NULL); i++)
		SThreadExecThreads(0);
}

int Script::SThreadSetEntrypoint(R_SCRIPT_THREAD *thread, int ep_num) {
	R_SCRIPT_BYTECODE *bytecode;
	int max_entrypoint;

	assert(isInitialized());

	bytecode = currentScript()->bytecode;
	max_entrypoint = bytecode->n_entrypoints;

	if ((ep_num < 0) || (ep_num >= max_entrypoint)) {
		return R_FAILURE;
	}

	thread->ep_num = ep_num;
	thread->ep_offset = bytecode->entrypoints[ep_num].offset;

	return R_SUCCESS;
}

int Script::SThreadExecute(R_SCRIPT_THREAD *thread, int ep_num) {
	assert(isInitialized());

	if ((currentScript() == NULL) || (!currentScript()->loaded)) {
		return R_FAILURE;
	}

	SThreadSetEntrypoint(thread, ep_num);

	thread->i_offset = thread->ep_offset;
	thread->executing = 1;

	return R_SUCCESS;
}

unsigned char *Script::SThreadGetReadPtr(R_SCRIPT_THREAD *thread) {
	return currentScript()->bytecode->bytecode_p + thread->i_offset;
}

unsigned long Script::SThreadGetReadOffset(const byte *read_p) {
	return (unsigned long)(read_p - (unsigned char *)currentScript()->bytecode->bytecode_p);
}

size_t Script::SThreadGetReadLen(R_SCRIPT_THREAD *thread) {
	return currentScript()->bytecode->bytecode_len - thread->i_offset;
}


int Script::SThreadHoldSem(R_SEMAPHORE *sem) {
	if (sem == NULL) {
		return R_FAILURE;
	}

	sem->hold_count++;

	return R_SUCCESS;
}

int Script::SThreadReleaseSem(R_SEMAPHORE *sem) {
	if (sem == NULL) {
		return R_FAILURE;
	}

	sem->hold_count--;
	if (sem->hold_count < 0) {
		sem->hold_count = 0;
	}

	return R_SUCCESS;
}

int Script::SThreadDebugStep() {
	if (_dbg_singlestep) {
		_dbg_dostep = 1;
	}

	return R_SUCCESS;
}

int Script::SThreadRun(R_SCRIPT_THREAD *thread, int instr_limit, int msec) {
	int instr_count;
	uint32 saved_offset;
	SDataWord_T param1;
	SDataWord_T param2;
	long iparam1;
	long iparam2;
	long iresult;

	SDataWord_T data;
	int debug_print = 0;
	int n_buf;
	int bitstate;
	int in_char;
	int i;
	int unhandled = 0;

	// Handle debug single-stepping
	if ((thread == _dbg_thread) && _dbg_singlestep) {
		if (_dbg_dostep) {
			debug_print = 1;
			thread->sleep_time = 0;
			instr_limit = 1;
			_dbg_dostep = 0;
		} else {
			return R_SUCCESS;
		}
	}

	MemoryReadStream scriptS(currentScript()->bytecode->bytecode_p, currentScript()->bytecode->bytecode_len);

	scriptS.seek(thread->i_offset);

	for (instr_count = 0; instr_count < instr_limit; instr_count++) {
		if ((!thread->executing) || (thread->sem.hold_count)) {
			break;
		}

		thread->sleep_time -= msec;
		if (thread->sleep_time < 0) {
			thread->sleep_time = 0;
		}

		if (thread->sleep_time) {
			break;
		}

		saved_offset = thread->i_offset;
		in_char = scriptS.readByte();

		debug(0, "Executing thread offset: %lu (%x)", thread->i_offset, in_char);

		switch (in_char) {
		case 0x01: // nextblock
			debug(0, "Stub: opcode 0x01(nextblock)");
			break;

// STACK INSTRUCTIONS
		case 0x02: // Dup top element (DUP)
			thread->stack->push(thread->stack->top());
			break;
		case 0x03: // Pop nothing (POPN)
			thread->stack->pop();
			break;
		case 0x04: // Push false (PSHF)
			thread->stack->push(0);
			break;
		case 0x05: // Push true (PSHT)
			thread->stack->push(1);
			break;
		case 0x06: // Push word (PUSH)
		case 0x08: // Push word (PSHD) (dialogue string index)
			param1 = (SDataWord_T)scriptS.readUint16LE();
			thread->stack->push(param1);
			break;

// DATA INSTRUCTIONS  

		case 0x0B: // Test flag (TSTF)
			n_buf = scriptS.readByte();
			param1 = (SDataWord_T)scriptS.readUint16LE();
			_vm->_sdata->getBit(n_buf, param1, &bitstate);
			thread->stack->push(bitstate);
			break;
		case 0x0C: // Get word (GETW)
			n_buf = scriptS.readByte();
			param1 = scriptS.readUint16LE();
			_vm->_sdata->getWord(n_buf, param1, &data);
			thread->stack->push(data);
			break;
		case 0x0F: // Modify flag (MODF)
			n_buf = scriptS.readByte();
			param1 = (SDataWord_T)scriptS.readUint16LE();
			bitstate = _vm->_sdata->readWordU(param1);
			data = thread->stack->top();
			if (bitstate) {
				_vm->_sdata->setBit(n_buf, data, 1);
			} else {
				_vm->_sdata->setBit(n_buf, data, 0);
			}
			break;
		case 0x10: // Put word (PUTW)
			n_buf = scriptS.readByte();
			param1 = (SDataWord_T)scriptS.readUint16LE();
			data = thread->stack->top();
			_vm->_sdata->putWord(n_buf, param1, data);
			break;
		case 0x13: // Modify flag and pop (MDFP)
			n_buf = scriptS.readByte();
			param1 = (SDataWord_T)scriptS.readUint16LE();
			param1 = thread->stack->pop();
			bitstate = _vm->_sdata->readWordU(param1);
			if (bitstate) {
				_vm->_sdata->setBit(n_buf, param1, 1);
			} else {
				_vm->_sdata->setBit(n_buf, param1, 0);
			}
			break;
		case 0x14: // Put word and pop (PTWP)
			n_buf = scriptS.readByte();
			param1 = (SDataWord_T)scriptS.readUint16LE();
			data = thread->stack->top();
			_vm->_sdata->putWord(n_buf, param1, data);
			break;

// CONTROL INSTRUCTIONS    

			// (GOSB): Call subscript ?
		case 0x17:
			{
				int temp;
				int temp2;

				temp = scriptS.readByte();
				temp2 = scriptS.readByte();
				param1 = (SDataWord_T)scriptS.readUint16LE();
				data = scriptS.pos();
				//thread->stack->push((SDataWord_T)temp);
				thread->stack->push(data);
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (CALL): Call function
		case 0x19:
		case 0x18:
			{
				int n_args;
				uint16 func_num;
				int FIXME_SHADOWED_result;
				SFunc_T sfunc;

				n_args = scriptS.readByte();
				func_num = scriptS.readUint16LE();
				if (func_num >= R_SFUNC_NUM) {
					_vm->_console->print(S_ERROR_PREFIX "Invalid script function number: (%X)\n", func_num);
					thread->executing = 0;
					break;
				}

				sfunc = _SFuncList[func_num].sfunc_fp;
				if (sfunc == NULL) {
					_vm->_console->print(S_WARN_PREFIX "%X: Undefined script function number: (%X)\n",
							thread->i_offset, func_num);
					_vm->_console->print(S_WARN_PREFIX "Removing %d operand(s) from stack.\n", n_args);
					for (i = 0; i < n_args; i++) {
						thread->stack->pop();
					}
				} else {
					FIXME_SHADOWED_result = (this->*sfunc)(thread);
					if (FIXME_SHADOWED_result != R_SUCCESS) {
						_vm->_console->print(S_WARN_PREFIX "%X: Script function %d failed.\n", thread->i_offset, func_num);
					}
				}
			}
			break;
		case 0x1A: // (ENTR) Enter the dragon
			//data = scriptS.pos();
			//thread->stack->push(data);
			
			param1 = scriptS.readUint16LE();
			break;
		case 0x1B: // Return with value
			unhandled = 1;
			break;
		case 0x1C: // Return with void
			if (thread->stack->size() == 0) {
				_vm->_console->print("Script execution complete.");
				thread->executing = 0;
			} else {
				data = thread->stack->pop();
				thread->i_offset = data;
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
			data = thread->stack->pop();
			if (data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JZP): Jump if zero + POP
		case 0x1F:
			param1 = scriptS.readUint16LE();
			data = thread->stack->pop();
			if (!data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JNZ): Jump if nonzero
		case 0x20:
			param1 = scriptS.readUint16LE();
			data = thread->stack->top();
			if (data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JZ): Jump if zero
		case 0x21:
			param1 = scriptS.readUint16LE();
			data = thread->stack->top();
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

				data = thread->stack->pop();
				n_switch = scriptS.readUint16LE();
				for (i = 0; i < n_switch; i++) {
					switch_num = scriptS.readUint16LE();
					switch_jmp = scriptS.readUint16LE();
					// Found the specified case
					if (data == (SDataWord_T) switch_num) {
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
					_vm->_console->print(S_ERROR_PREFIX "%X: Random jump target out of " "bounds.", thread->i_offset);
				}
			}
			break;

// UNARY INSTRUCTIONS

			// (NEG) Negate stack by 2's complement
		case 0x25:
			data = thread->stack->pop();
			data = ~data;
			data++;
			thread->stack->push(data);
			break;
			// (TSTZ) Test for zero
		case 0x26:
			data = thread->stack->pop();
			data = data ? 0 : 1;
			thread->stack->push(data);
			break;
			// (NOT) Binary not
		case 0x27:
			data = thread->stack->pop();
			data = ~data;
			thread->stack->push(data);
			break;
		case 0x28: // inc_v increment, don't push
			unhandled = 1;
			printf("??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;
		case 0x29: // dec_v decrement, don't push
			unhandled = 1;
			printf("??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;
		case 0x2A: // postinc
			unhandled = 1;
			printf("??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;
		case 0x2B: // postdec
			unhandled = 1;
			printf("??? ");
			scriptS.readByte();
			scriptS.readUint16LE();
			break;

// ARITHMETIC INSTRUCTIONS    

			// (ADD): Addition
		case 0x2C:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 + iparam2;
			thread->stack->push((SDataWord_T) iresult);
			break;
			// (SUB): Subtraction
		case 0x2D:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 - iparam2;
			thread->stack->push((SDataWord_T) iresult);
			break;
			// (MULT): Integer multiplication
		case 0x2E:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 * iparam2;
			thread->stack->push((SDataWord_T) iresult);
			break;
			// (DIV): Integer division
		case 0x2F:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 / iparam2;
			thread->stack->push((SDataWord_T) iresult);
			break;
			// (MOD) Modulus
		case 0x30:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 % iparam2;
			thread->stack->push((SDataWord_T) iresult);
			break;
			// (EQU) Test equality
		case 0x33:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 == iparam2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (NEQU) Test inequality
		case 0x34:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 != iparam2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (GRT) Test Greater-than
		case 0x35:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 > iparam2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (LST) Test Less-than
		case 0x36:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 < iparam2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (GRTE) Test Greater-than or Equal to
		case 0x37:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 >= iparam2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (LSTE) Test Less-than or Equal to
		case 0x38:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 <= iparam2) ? 1 : 0;
			thread->stack->push(data);
			break;

// BITWISE INSTRUCTIONS   

			// (SHR): Arithmetic binary shift right
		case 0x3F:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			iparam2 = (long)param2;
			// Preserve most significant bit
			data = (0x01 << ((sizeof param1 * CHAR_BIT) - 1)) & param1;
			for (i = 0; i < (int)iparam2; i++) {
				param1 >>= 1;
				param1 |= data;
			}
			thread->stack->push(param1);
			break;
			// (SHL) Binary shift left
		case 0x40:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			param1 <<= param2;
			thread->stack->push(param1);
			break;
			// (AND) Binary AND
		case 0x41:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			param1 &= param2;
			thread->stack->push(param1);
			break;
			// (OR) Binary OR
		case 0x42:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			param1 |= param2;
			thread->stack->push(param1);
			break;
			// (XOR) Binary XOR
		case 0x43:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			param1 ^= param2;
			thread->stack->push(param1);
			break;

// BOOLEAN LOGIC INSTRUCTIONS     

			// (LAND): Logical AND
		case 0x44:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			data = (param1 && param2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (LOR): Logical OR
		case 0x45:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			data = (param1 || param2) ? 1 : 0;
			thread->stack->push(data);
			break;
			// (LXOR): Logical XOR
		case 0x46:
			param2 = thread->stack->pop();
			param1 = thread->stack->pop();
			data = ((param1) ? !(param2) : !!(param2));
			thread->stack->push(data);
			break;

// GAME INSTRUCTIONS  

			// (DLGP): Play Character Dialogue
		case 0x53:
			{
				int n_voices;
				int a_index;
				int voice_rn;

				n_voices = scriptS.readByte();
				param1 = (SDataWord_T) scriptS.readUint16LE();
				// ignored ?
				scriptS.readByte();
				scriptS.readUint16LE();

				a_index = _vm->_actor->getActorIndex(param1);
				if (a_index < 0) {
					_vm->_console->print(S_WARN_PREFIX "%X: DLGP Actor id not found.", thread->i_offset);
				}

				for (i = 0; i < n_voices; i++) {
					data = thread->stack->pop();
					if (a_index < 0)
						continue;
					if (!isVoiceLUTPresent()) {
						voice_rn = -1;
					} else {
						voice_rn = currentScript()->voice->voices[data];
					}
					_vm->_actor->speak(a_index, currentScript()->diag-> str[data], voice_rn, &thread->sem);
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
				int FIXME_SHADOWED_param1;
				int FIXME_SHADOWED_param2;
				int FIXME_SHADOWED_param3;

				printf("DLGO | ");
				FIXME_SHADOWED_param1 = scriptS.readByte();
				FIXME_SHADOWED_param2 = scriptS.readByte();
				printf("%02X %02X ", FIXME_SHADOWED_param1, FIXME_SHADOWED_param2);

				if (FIXME_SHADOWED_param2 > 0) {
					FIXME_SHADOWED_param3 = scriptS.readUint16LE();
					printf("%04X", FIXME_SHADOWED_param3);
				}
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

			_vm->_console->print(S_ERROR_PREFIX "%X: Invalid opcode encountered: " "(%X).\n", thread->i_offset, in_char);
			thread->executing = 0;
			break;
		}

		// Set instruction offset only if a previous instruction didn't branch
		if (saved_offset == thread->i_offset) {
			thread->i_offset = scriptS.pos();
		} else {
			scriptS.seek(thread->i_offset);
		}
		if (unhandled) {
			_vm->_console->print(S_ERROR_PREFIX "%X: Unhandled opcode.\n", thread->i_offset);
			thread->executing = 0;
		}
		if (thread->executing && debug_print) {
			SDebugPrintInstr(thread);
		}
	}

	return R_SUCCESS;
}

} // End of namespace Saga

