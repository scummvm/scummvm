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
#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

#include "actor_mod.h"
#include "console_mod.h"
#include "text_mod.h"

#include "script.h"
#include "script_mod.h"

#include "sdata.h"
#include "sstack.h"
#include "sthread.h"
#include "sfuncs.h"

namespace Saga {

R_SCRIPT_THREAD *STHREAD_Create() {
	YS_DL_NODE *new_node;
	R_SCRIPT_THREAD *new_thread;

	int result;

	if (!ScriptModule.initialized) {
		return NULL;
	}

	new_thread = (R_SCRIPT_THREAD *)calloc(1, sizeof *new_thread);
	if (new_thread == NULL) {
		return NULL;
	}

	result = SSTACK_Create(&(new_thread->stack), R_DEF_THREAD_STACKSIZE, STACK_GROW);

	if (result != STACK_SUCCESS) {
		return NULL;
	}

	new_node = ys_dll_add_head(ScriptModule.thread_list, new_thread, sizeof *new_thread);

	free(new_thread);

	return (R_SCRIPT_THREAD *)ys_dll_get_data(new_node);
}

int STHREAD_Destroy(R_SCRIPT_THREAD *thread) {
	if (thread == NULL) {
		return R_FAILURE;
	}

	SSTACK_Destroy(thread->stack);

	return R_SUCCESS;
}

int STHREAD_ExecThreads(int msec) {
	YS_DL_NODE *walk_p;
	R_SCRIPT_THREAD *thread;

	if (!ScriptModule.initialized) {
		return R_FAILURE;
	}

	for (walk_p = ys_dll_head(ScriptModule.thread_list); walk_p != NULL; walk_p = ys_dll_next(walk_p)) {
		thread = (R_SCRIPT_THREAD *)ys_dll_get_data(walk_p);
		if (thread->executing) {
			STHREAD_Run(thread, STHREAD_DEF_INSTR_COUNT, msec);
		}
	}

	return R_SUCCESS;
}

int STHREAD_SetEntrypoint(R_SCRIPT_THREAD *thread, int ep_num) {
	R_SCRIPT_BYTECODE *bytecode;
	int max_entrypoint;

	assert(ScriptModule.initialized);

	bytecode = ScriptModule.current_script->bytecode;
	max_entrypoint = bytecode->n_entrypoints;

	if ((ep_num < 0) || (ep_num >= max_entrypoint)) {
		return R_FAILURE;
	}

	thread->ep_num = ep_num;
	thread->ep_offset = bytecode->entrypoints[ep_num].offset;

	return R_SUCCESS;
}

int STHREAD_Execute(R_SCRIPT_THREAD *thread, int ep_num) {
	assert(ScriptModule.initialized);

	if ((ScriptModule.current_script == NULL) || (!ScriptModule.current_script->loaded)) {
		return R_FAILURE;
	}

	STHREAD_SetEntrypoint(thread, ep_num);

	thread->i_offset = thread->ep_offset;
	thread->executing = 1;

	return R_SUCCESS;
}

unsigned char *GetReadPtr(R_SCRIPT_THREAD *thread) {
	return ScriptModule.current_script->bytecode->bytecode_p + thread->i_offset;
}

unsigned long GetReadOffset(const byte *read_p) {
	return (unsigned long)(read_p - (unsigned char *)ScriptModule.current_script->bytecode->bytecode_p);
}

int STHREAD_HoldSem(R_SEMAPHORE *sem) {
	if (sem == NULL) {
		return R_FAILURE;
	}

	sem->hold_count++;

	return R_SUCCESS;
}

int STHREAD_ReleaseSem(R_SEMAPHORE *sem) {
	if (sem == NULL) {
		return R_FAILURE;
	}

	sem->hold_count--;
	if (sem->hold_count < 0) {
		sem->hold_count = 0;
	}

	return R_SUCCESS;
}

int STHREAD_DebugStep() {
	if (ScriptModule.dbg_singlestep) {
		ScriptModule.dbg_dostep = 1;
	}

	return R_SUCCESS;
}

int STHREAD_Run(R_SCRIPT_THREAD *thread, int instr_limit, int msec) {
	int instr_count;
	const byte *read_p;
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
	int result;
	int in_char;
	int i;
	int unhandled = 0;

	// Handle debug single-stepping
	if ((thread == ScriptModule.dbg_thread) && ScriptModule.dbg_singlestep) {
		if (ScriptModule.dbg_dostep) {
			debug_print = 1;
			thread->sleep_time = 0;
			instr_limit = 1;
			ScriptModule.dbg_dostep = 0;
		} else {
			return R_SUCCESS;
		}
	}

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
#if 0
		R_printf(R_STDOUT, "Executing thread offset: %lu", thread->i_offset);
#endif
		read_p = GetReadPtr(thread);

		in_char = ys_read_u8(read_p, &read_p);

		switch (in_char) {
			// Align (ALGN)
		case 0x01:
			break;

// STACK INSTRUCTIONS

			// Push nothing (PSHN)
		case 0x02:
			SSTACK_PushNull(thread->stack);
			break;
			// Pop nothing (POPN)
		case 0x03:
			SSTACK_Pop(thread->stack, NULL);
			break;
			// Push false (PSHF)
		case 0x04:
			SSTACK_Push(thread->stack, 0);
			break;
			// Push true (PSHT)
		case 0x05:
			SSTACK_Push(thread->stack, 1);
			break;
			// Push word (PUSH)
		case 0x06:
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			SSTACK_Push(thread->stack, param1);
			break;
			// Push word (PSHD) (dialogue string index)
		case 0x08:
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			SSTACK_Push(thread->stack, param1);
			break;

// DATA INSTRUCTIONS  

			// Test flag (TSTF)
		case 0x0B:
			n_buf = *read_p++;
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			SDATA_GetBit(n_buf, param1, &bitstate);
			SSTACK_Push(thread->stack, bitstate);
			break;
			// Get word (GETW)
		case 0x0C:
			n_buf = *read_p++;
			param1 = ys_read_u16_le(read_p, &read_p);
			SDATA_GetWord(n_buf, param1, &data);
			SSTACK_Push(thread->stack, data);
			break;
			// Modify flag (MODF)
		case 0x0F:
			n_buf = *read_p++;
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			bitstate = SDATA_ReadWordU(param1);
			SSTACK_Top(thread->stack, &data);
			if (bitstate) {
				SDATA_SetBit(n_buf, data, 1);
			} else {
				SDATA_SetBit(n_buf, data, 0);
			}
			break;
			// Put word (PUTW)
		case 0x10:
			n_buf = *read_p++;
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			SSTACK_Top(thread->stack, &data);
			SDATA_PutWord(n_buf, param1, data);
			break;
			// Modify flag and pop (MDFP)
		case 0x13:
			n_buf = *read_p++;
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			SSTACK_Pop(thread->stack, &param1);
			bitstate = SDATA_ReadWordU(param1);
			if (bitstate) {
				SDATA_SetBit(n_buf, param1, 1);
			} else {
				SDATA_SetBit(n_buf, param1, 0);
			}
			break;
			// Put word and pop (PTWP)
		case 0x14:
			n_buf = *read_p++;
			param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
			SSTACK_Top(thread->stack, &data);
			SDATA_PutWord(n_buf, param1, data);
			break;

// CONTROL INSTRUCTIONS    

			// (GOSB): Call subscript ?
		case 0x17:
			{
				int temp;
				int temp2;

				temp = *read_p++;
				temp2 = *read_p++;
				param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
				data = GetReadOffset(read_p);
				//SSTACK_Push(thread->stack, (SDataWord_T)temp);
				SSTACK_Push(thread->stack, data);
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

				n_args = ys_read_u8(read_p, &read_p);
				func_num = ys_read_u16_le(read_p, &read_p);
				if (func_num >= R_SFUNC_NUM) {
					CON_Print(S_ERROR_PREFIX "Invalid script function number: (%X)\n", func_num);
					thread->executing = 0;
					break;
				}

				sfunc = SFuncList[func_num].sfunc_fp;
				if (sfunc == NULL) {
					CON_Print(S_WARN_PREFIX "%X: Undefined script function number: (%X)\n",
							thread->i_offset, func_num);
					CON_Print(S_WARN_PREFIX "Removing %d operand(s) from stack.\n", n_args);
					for (i = 0; i < n_args; i++) {
						SSTACK_Pop(thread->stack, NULL);
					}
				} else {
					FIXME_SHADOWED_result = sfunc(thread);
					if (FIXME_SHADOWED_result != R_SUCCESS) {
						CON_Print(S_WARN_PREFIX "%X: Script function %d failed.\n", thread->i_offset, func_num);
					}
				}
			}
			break;
			// (ENTR) Enter the dragon
		case 0x1A:
			param1 = ys_read_u16_le(read_p, &read_p);
			break;
			// (?) Unknown
		case 0x1B:
			unhandled = 1;
			break;
			// (EXIT) End subscript
		case 0x1C:
			result = SSTACK_Pop(thread->stack, &data);
			if (result != STACK_SUCCESS) {
				CON_Print("Script execution complete.");
				thread->executing = 0;
			} else {
				thread->i_offset = data;
			}
			break;

// BRANCH INSTRUCTIONS    

			// (JMP): Unconditional jump
		case 0x1D:
			param1 = ys_read_u16_le(read_p, &read_p);
			thread->i_offset = (unsigned long)param1;
			break;
			// (JNZP): Jump if nonzero + POP
		case 0x1E:
			param1 = ys_read_u16_le(read_p, &read_p);
			SSTACK_Pop(thread->stack, &data);
			if (data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JZP): Jump if zero + POP
		case 0x1F:
			param1 = ys_read_u16_le(read_p, &read_p);
			SSTACK_Pop(thread->stack, &data);
			if (!data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JNZ): Jump if nonzero
		case 0x20:
			param1 = ys_read_u16_le(read_p, &read_p);
			SSTACK_Top(thread->stack, &data);
			if (data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JZ): Jump if zero
		case 0x21:
			param1 = ys_read_u16_le(read_p, &read_p);
			SSTACK_Top(thread->stack, &data);
			if (!data) {
				thread->i_offset = (unsigned long)param1;
			}
			break;
			// (JMPR): Relative jump
		case 0x57:
			// ignored?
			ys_read_u16_le(read_p, &read_p);
			ys_read_u16_le(read_p, &read_p);
			iparam1 = (long)*read_p++;
			thread->i_offset += iparam1;
			break;
			// (SWCH): Switch
		case 0x22:
			{
				int n_switch;
				unsigned int switch_num;
				unsigned int switch_jmp;
				unsigned int default_jmp;
				int case_found = 0;

				SSTACK_Pop(thread->stack, &data);
				n_switch = ys_read_u16_le(read_p, &read_p);
				for (i = 0; i < n_switch; i++) {
					switch_num = ys_read_u16_le(read_p, &read_p);
					switch_jmp = ys_read_u16_le(read_p, &read_p);
					// Found the specified case
					if (data == (SDataWord_T) switch_num) {
						thread->i_offset = switch_jmp;
						case_found = 1;
						break;
					}
				}

				// Jump to default case
				if (!case_found) {
					default_jmp = ys_read_u16_le(read_p, &read_p);
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
				ys_read_u16_le(read_p, &read_p);
				n_branch = ys_read_u16_le(read_p, &read_p);
				for (i = 0; i < n_branch; i++) {
					branch_wt = ys_read_u16_le(read_p, &read_p);
					branch_jmp = ys_read_u16_le(read_p, &read_p);
					if (rand_sel == i) {
						thread->i_offset = branch_jmp;
						branch_found = 1;
						break;
					}
				}
				if (!branch_found) {
					CON_Print(S_ERROR_PREFIX "%X: Random jump target out of " "bounds.", thread->i_offset);
				}
			}
			break;

// MISC. INSTRUCTIONS

			// (NEG) Negate stack by 2's complement
		case 0x25:
			SSTACK_Pop(thread->stack, &data);
			data = ~data;
			data++;
			SSTACK_Push(thread->stack, data);
			break;
			// (TSTZ) Test for zero
		case 0x26:
			SSTACK_Pop(thread->stack, &data);
			data = data ? 0 : 1;
			SSTACK_Push(thread->stack, data);
			break;
			// (NOT) Binary not
		case 0x27:
			SSTACK_Pop(thread->stack, &data);
			data = ~data;
			SSTACK_Push(thread->stack, data);
			break;
			// (?)
		case 0x28:
			unhandled = 1;
			printf("??? ");
			read_p++;
			ys_read_u16_le(read_p, &read_p);
			break;
			// (?)
		case 0x29:
			unhandled = 1;
			printf("??? ");
			read_p++;
			ys_read_u16_le(read_p, &read_p);
			break;
			// (?)
		case 0x2A:
			unhandled = 1;
			printf("??? ");
			read_p++;
			ys_read_u16_le(read_p, &read_p);
			break;
			// (?)
		case 0x2B:
			unhandled = 1;
			printf("??? ");
			read_p++;
			ys_read_u16_le(read_p, &read_p);
			break;

// ARITHMETIC INSTRUCTIONS    

			// (ADD): Addition
		case 0x2C:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 + iparam2;
			SSTACK_Push(thread->stack, (SDataWord_T) iresult);
			break;

			// (SUB): Subtraction
		case 0x2D:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 - iparam2;
			SSTACK_Push(thread->stack, (SDataWord_T) iresult);
			break;
			// (MULT): Integer multiplication
		case 0x2E:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 * iparam2;
			SSTACK_Push(thread->stack, (SDataWord_T) iresult);
			break;
			// (DIB): Integer division
		case 0x2F:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 / iparam2;
			SSTACK_Push(thread->stack, (SDataWord_T) iresult);
			break;
			// (MOD) Modulus
		case 0x30:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			iresult = iparam1 % iparam2;
			SSTACK_Push(thread->stack, (SDataWord_T) iresult);
			break;
			// (EQU) Test equality
		case 0x33:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 == iparam2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (NEQU) Test inequality
		case 0x34:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 != iparam2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (GRT) Test Greater-than
		case 0x35:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 > iparam2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (LST) Test Less-than
		case 0x36:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 < iparam2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (GRTE) Test Greater-than or Equal to
		case 0x37:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 >= iparam2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (LSTE) Test Less-than or Equal to
		case 0x38:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			iparam1 = (long)param1;
			data = (iparam1 <= iparam2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (SHR): Arithmetic binary shift right
		case 0x3F:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			iparam2 = (long)param2;
			// Preserve most significant bit
			data = (0x01 << ((sizeof param1 * CHAR_BIT) - 1)) & param1;
			for (i = 0; i < (int)iparam2; i++) {
				param1 >>= 1;
				param1 |= data;
			}
			SSTACK_Push(thread->stack, param1);
			break;

// BITWISE INSTRUCTIONS   

			// (SHL) Binary shift left
		case 0x40:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			param1 <<= param2;
			SSTACK_Push(thread->stack, param1);
			break;
			// (AND) Binary AND
		case 0x41:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			param1 &= param2;
			SSTACK_Push(thread->stack, param1);
			break;
			// (OR) Binary OR
		case 0x42:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			param1 |= param2;
			SSTACK_Push(thread->stack, param1);
			break;
			// (XOR) Binary XOR
		case 0x43:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			param1 ^= param2;
			SSTACK_Push(thread->stack, param1);
			break;

// BOOLEAN LOGIC INSTRUCTIONS     

			// (LAND): Logical AND
		case 0x44:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			data = (param1 && param2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (LOR): Logical OR
		case 0x45:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			data = (param1 || param2) ? 1 : 0;
			SSTACK_Push(thread->stack, data);
			break;
			// (LXOR): Logical XOR
		case 0x46:
			SSTACK_Pop(thread->stack, &param2);
			SSTACK_Pop(thread->stack, &param1);
			data = ((param1) ? !(param2) : !!(param2));
			SSTACK_Push(thread->stack, data);
			break;

// GAME INSTRUCTIONS  

			// (DLGP): Play Character Dialogue
		case 0x53:
			{
				int n_voices;
				int a_index;
				int voice_rn;

				n_voices = *read_p++;
				param1 = (SDataWord_T) ys_read_u16_le(read_p, &read_p);
				// ignored ?
				*read_p++;
				ys_read_u16_le(read_p, &read_p);

				a_index = ACTOR_GetActorIndex(param1);
				if (a_index < 0) {
					CON_Print(S_WARN_PREFIX "%X: DLGP Actor id not found.", thread->i_offset);
				}

				for (i = 0; i < n_voices; i++) {
					SSTACK_Pop(thread->stack, &data);
					if (a_index < 0)
						continue;
					if (!ScriptModule.voice_lut_present) {
						voice_rn = -1;
					} else {
						voice_rn = ScriptModule. current_script->voice->voices[data];
					}
					ACTOR_Speak(a_index, ScriptModule.current_script->diag-> str[data], voice_rn, &thread->sem);
				}
			}
			break;
			// (DLGS): Initialize dialogue interface
		case 0x54:
			break;
			// (DLGX): Run dialogue interface
		case 0x55:
			break;
			// (DLGO): Add a dialogue option to interface
		case 0x56:
			{
				int FIXME_SHADOWED_param1;
				int FIXME_SHADOWED_param2;
				int FIXME_SHADOWED_param3;

				printf("DLGO | ");
				FIXME_SHADOWED_param1 = *read_p++;
				FIXME_SHADOWED_param2 = *read_p++;
				printf("%02X %02X ", FIXME_SHADOWED_param1, FIXME_SHADOWED_param2);

				if (FIXME_SHADOWED_param2 > 0) {
					FIXME_SHADOWED_param3 = ys_read_u16_le(read_p, &read_p);
					printf("%04X", FIXME_SHADOWED_param3);
				}
			}
			break;

// End instruction list

		default:

			CON_Print(S_ERROR_PREFIX "%X: Invalid opcode encountered: " "(%X).\n", thread->i_offset, in_char);
			thread->executing = 0;
			break;
		}

		// Set instruction offset only if a previous instruction didn't branch
		if (saved_offset == thread->i_offset) {
			thread->i_offset = GetReadOffset(read_p);
		}
		if (unhandled) {
			CON_Print(S_ERROR_PREFIX "%X: Unhandled opcode.\n", thread->i_offset);
			thread->executing = 0;
		}
		if (thread->executing && debug_print) {
			SDEBUG_PrintInstr(thread);
		}
	}

	return R_SUCCESS;
}

} // End of namespace Saga

