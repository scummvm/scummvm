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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/glulxe/glulxe.h"

namespace Glk {
namespace Glulxe {

void Glulxe::execute_loop() {
	bool done_executing = false;
	int ix;
	uint opcode;
	const operandlist_t *oplist;
	oparg_t inst[MAX_OPERANDS];
	uint value, addr, val0, val1;
	int vals0, vals1;
	uint *arglist;
	uint arglistfix[3];
#ifdef FLOAT_SUPPORT
	gfloat32 valf, valf1, valf2;
#endif /* FLOAT_SUPPORT */

	while (!done_executing && !g_vm->shouldQuit()) {

		profile_tick();
		debugger_tick();
		/* Do OS-specific processing, if appropriate. */
		glk_tick();

		/* Stash the current opcode's address, in case the interpreter needs to serialize the VM state out-of-band. */
		prevpc = pc;

		/* Fetch the opcode number. */
		opcode = Mem1(pc);
		pc++;
		if (opcode & 0x80) {
			/* More than one-byte opcode. */
			if (opcode & 0x40) {
				/* Four-byte opcode */
				opcode &= 0x3F;
				opcode = (opcode << 8) | Mem1(pc);
				pc++;
				opcode = (opcode << 8) | Mem1(pc);
				pc++;
				opcode = (opcode << 8) | Mem1(pc);
				pc++;
			} else {
				/* Two-byte opcode */
				opcode &= 0x7F;
				opcode = (opcode << 8) | Mem1(pc);
				pc++;
			}
		}

		/* Now we have an opcode number. */

		/* Fetch the structure that describes how the operands for this
		   opcode are arranged. This is a pointer to an immutable,
		   static object. */
		if (opcode < 0x80)
			oplist = fast_operandlist[opcode];
		else
			oplist = lookup_operandlist(opcode);

		if (!oplist)
			fatal_error_i("Encountered unknown opcode.", opcode);

		/* Based on the oplist structure, load the actual operand values
		   into inst. This moves the PC up to the end of the instruction. */
		parse_operands(inst, oplist);

		/* Perform the opcode. This switch statement is split in two, based
		   on some paranoid suspicions about the ability of compilers to
		   optimize large-range switches. Ignore that. */

		if (opcode < 0x80) {

			switch (opcode) {

			case op_nop:
				break;

			case op_add:
				value = inst[0].value + inst[1].value;
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_sub:
				value = inst[0].value - inst[1].value;
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_mul:
				value = inst[0].value * inst[1].value;
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_div:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals1 == 0)
					fatal_error("Division by zero.");
				/* Since C doesn't guarantee the results of division of negative
				   numbers, we carefully convert everything to positive values
				   first. They have to be unsigned values, too, otherwise the
				   0x80000000 case goes wonky. */
				if (vals0 < 0) {
					val0 = (-vals0);
					if (vals1 < 0) {
						val1 = (-vals1);
						value = val0 / val1;
					} else {
						val1 = vals1;
						value = -(int)(val0 / val1);
					}
				} else {
					val0 = vals0;
					if (vals1 < 0) {
						val1 = (-vals1);
						value = -(int)(val0 / val1);
					} else {
						val1 = vals1;
						value = val0 / val1;
					}
				}
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_mod:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals1 == 0)
					fatal_error("Division by zero doing remainder.");
				if (vals1 < 0) {
					val1 = -vals1;
				} else {
					val1 = vals1;
				}
				if (vals0 < 0) {
					val0 = (-vals0);
					value = -(int)(val0 % val1);
				} else {
					val0 = vals0;
					value = val0 % val1;
				}
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_neg:
				vals0 = inst[0].value;
				value = (-vals0);
				store_operand(inst[1].desttype, inst[1].value, value);
				break;

			case op_bitand:
				value = (inst[0].value & inst[1].value);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_bitor:
				value = (inst[0].value | inst[1].value);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_bitxor:
				value = (inst[0].value ^ inst[1].value);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_bitnot:
				value = ~(inst[0].value);
				store_operand(inst[1].desttype, inst[1].value, value);
				break;

			case op_shiftl:
				vals0 = inst[1].value;
				if (vals0 < 0 || vals0 >= 32)
					value = 0;
				else
					value = ((uint)(inst[0].value) << (uint)vals0);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_ushiftr:
				vals0 = inst[1].value;
				if (vals0 < 0 || vals0 >= 32)
					value = 0;
				else
					value = ((uint)(inst[0].value) >> (uint)vals0);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_sshiftr:
				vals0 = inst[1].value;
				if (vals0 < 0 || vals0 >= 32) {
					if (inst[0].value & 0x80000000)
						value = 0xFFFFFFFF;
					else
						value = 0;
				} else {
					/* This is somewhat foolhardy -- C doesn't guarantee that
					   right-shifting a signed value replicates the sign bit.
					   We'll assume it for now. */
					value = ((int)(inst[0].value) >> (int)vals0);
				}
				store_operand(inst[2].desttype, inst[2].value, value);
				break;

			case op_jump:
				value = inst[0].value;
				/* fall through to PerformJump label. */

PerformJump: /* goto label for successful jumping... ironic, no? */
				if (value == 0 || value == 1) {
					/* Return from function. This is exactly what happens in
					   return_op, but it's only a few lines of code, so I won't
					   bother with a "goto". */
					leave_function();
					if (stackptr == 0) {
						done_executing = true;
						break;
					}
					pop_callstub(value); /* zero or one */
				} else {
					/* Branch to a new PC value. */
					pc = (pc + value - 2);
				}
				break;

			case op_jz:
				if (inst[0].value == 0) {
					value = inst[1].value;
					goto PerformJump;
				}
				break;
			case op_jnz:
				if (inst[0].value != 0) {
					value = inst[1].value;
					goto PerformJump;
				}
				break;
			case op_jeq:
				if (inst[0].value == inst[1].value) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jne:
				if (inst[0].value != inst[1].value) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jlt:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals0 < vals1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jgt:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals0 > vals1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jle:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals0 <= vals1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jge:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals0 >= vals1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jltu:
				val0 = inst[0].value;
				val1 = inst[1].value;
				if (val0 < val1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jgtu:
				val0 = inst[0].value;
				val1 = inst[1].value;
				if (val0 > val1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jleu:
				val0 = inst[0].value;
				val1 = inst[1].value;
				if (val0 <= val1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jgeu:
				val0 = inst[0].value;
				val1 = inst[1].value;
				if (val0 >= val1) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;

			case op_call:
				value = inst[1].value;
				arglist = pop_arguments(value, 0);
				push_callstub(inst[2].desttype, inst[2].value);
				enter_function(inst[0].value, value, arglist);
				break;
			case op_return:
				leave_function();
				if (stackptr == 0) {
					done_executing = true;
					break;
				}
				pop_callstub(inst[0].value);
				break;
			case op_tailcall:
				value = inst[1].value;
				arglist = pop_arguments(value, 0);
				leave_function();
				enter_function(inst[0].value, value, arglist);
				break;

			case op_catch:
				push_callstub(inst[0].desttype, inst[0].value);
				value = inst[1].value;
				val0 = stackptr;
				store_operand(inst[0].desttype, inst[0].value, val0);
				goto PerformJump;
				break;
			case op_throw:
				profile_fail("throw");
				value = inst[0].value;
				stackptr = inst[1].value;
				pop_callstub(value);
				break;

			case op_copy:
				value = inst[0].value;
#ifdef TOLERATE_SUPERGLUS_BUG
				if (inst[1].desttype == 1 && inst[1].value == 0)
					inst[1].desttype = 0;
#endif /* TOLERATE_SUPERGLUS_BUG */
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_copys:
				value = inst[0].value;
				store_operand_s(inst[1].desttype, inst[1].value, value);
				break;
			case op_copyb:
				value = inst[0].value;
				store_operand_b(inst[1].desttype, inst[1].value, value);
				break;

			case op_sexs:
				val0 = inst[0].value;
				if (val0 & 0x8000)
					val0 |= 0xFFFF0000;
				else
					val0 &= 0x0000FFFF;
				store_operand(inst[1].desttype, inst[1].value, val0);
				break;
			case op_sexb:
				val0 = inst[0].value;
				if (val0 & 0x80)
					val0 |= 0xFFFFFF00;
				else
					val0 &= 0x000000FF;
				store_operand(inst[1].desttype, inst[1].value, val0);
				break;

			case op_aload:
				value = inst[0].value;
				value += 4 * inst[1].value;
				val0 = Mem4(value);
				store_operand(inst[2].desttype, inst[2].value, val0);
				break;
			case op_aloads:
				value = inst[0].value;
				value += 2 * inst[1].value;
				val0 = Mem2(value);
				store_operand(inst[2].desttype, inst[2].value, val0);
				break;
			case op_aloadb:
				value = inst[0].value;
				value += inst[1].value;
				val0 = Mem1(value);
				store_operand(inst[2].desttype, inst[2].value, val0);
				break;
			case op_aloadbit:
				value = inst[0].value;
				vals0 = inst[1].value;
				val1 = (vals0 & 7);
				if (vals0 >= 0)
					value += (vals0 >> 3);
				else
					value -= (1 + ((-1 - vals0) >> 3));
				if (Mem1(value) & (1 << val1))
					val0 = 1;
				else
					val0 = 0;
				store_operand(inst[2].desttype, inst[2].value, val0);
				break;

			case op_astore:
				value = inst[0].value;
				value += 4 * inst[1].value;
				val0 = inst[2].value;
				MemW4(value, val0);
				break;
			case op_astores:
				value = inst[0].value;
				value += 2 * inst[1].value;
				val0 = inst[2].value;
				MemW2(value, val0);
				break;
			case op_astoreb:
				value = inst[0].value;
				value += inst[1].value;
				val0 = inst[2].value;
				MemW1(value, val0);
				break;
			case op_astorebit:
				value = inst[0].value;
				vals0 = inst[1].value;
				val1 = (vals0 & 7);
				if (vals0 >= 0)
					value += (vals0 >> 3);
				else
					value -= (1 + ((-1 - vals0) >> 3));
				val0 = Mem1(value);
				if (inst[2].value)
					val0 |= (1 << val1);
				else
					val0 &= ~((uint)(1 << val1));
				MemW1(value, val0);
				break;

			case op_stkcount:
				value = (stackptr - valstackbase) / 4;
				store_operand(inst[0].desttype, inst[0].value, value);
				break;
			case op_stkpeek:
				vals0 = inst[0].value * 4;
				if (vals0 < 0 || vals0 >= (int)(stackptr - valstackbase))
					fatal_error("Stkpeek outside current stack range.");
				value = Stk4(stackptr - (vals0 + 4));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_stkswap:
				if (stackptr < valstackbase + 8) {
					fatal_error("Stack underflow in stkswap.");
				}
				val0 = Stk4(stackptr - 4);
				val1 = Stk4(stackptr - 8);
				StkW4(stackptr - 4, val1);
				StkW4(stackptr - 8, val0);
				break;
			case op_stkcopy:
				vals0 = inst[0].value;
				if (vals0 < 0)
					fatal_error("Negative operand in stkcopy.");
				if (vals0 == 0)
					break;
				if (stackptr < valstackbase + vals0 * 4)
					fatal_error("Stack underflow in stkcopy.");
				if (stackptr + vals0 * 4 > stacksize)
					fatal_error("Stack overflow in stkcopy.");
				addr = stackptr - vals0 * 4;
				for (ix = 0; ix < vals0; ix++) {
					value = Stk4(addr + ix * 4);
					StkW4(stackptr + ix * 4, value);
				}
				stackptr += vals0 * 4;
				break;
			case op_stkroll:
				vals0 = inst[0].value;
				vals1 = inst[1].value;
				if (vals0 < 0)
					fatal_error("Negative operand in stkroll.");
				if (stackptr < valstackbase + vals0 * 4)
					fatal_error("Stack underflow in stkroll.");
				if (vals0 == 0)
					break;
				/* The following is a bit ugly. We want to do vals1 = vals0-vals1,
				   because rolling down is sort of easier than rolling up. But
				   we also want to take the result mod vals0. The % operator is
				   annoying for negative numbers, so we need to do this in two
				   cases. */
				if (vals1 > 0) {
					vals1 = vals1 % vals0;
					vals1 = (vals0) - vals1;
				} else {
					vals1 = (-vals1) % vals0;
				}
				if (vals1 == 0)
					break;
				addr = stackptr - vals0 * 4;
				for (ix = 0; ix < vals1; ix++) {
					value = Stk4(addr + ix * 4);
					StkW4(stackptr + ix * 4, value);
				}
				for (ix = 0; ix < vals0; ix++) {
					value = Stk4(addr + (vals1 + ix) * 4);
					StkW4(addr + ix * 4, value);
				}
				break;

			case op_streamchar:
				profile_in(0xE0000001, stackptr, false);
				value = inst[0].value & 0xFF;
				(this->*stream_char_handler)(value);
				profile_out(stackptr);
				break;
			case op_streamunichar:
				profile_in(0xE0000002, stackptr, false);
				value = inst[0].value;
				(this->*stream_unichar_handler)(value);
				profile_out(stackptr);
				break;
			case op_streamnum:
				profile_in(0xE0000003, stackptr, false);
				vals0 = inst[0].value;
				stream_num(vals0, false, 0);
				profile_out(stackptr);
				break;
			case op_streamstr:
				profile_in(0xE0000004, stackptr, false);
				stream_string(inst[0].value, 0, 0);
				profile_out(stackptr);
				break;

			default:
				fatal_error_i("Executed unknown opcode.", opcode);
			}
		} else {

			switch (opcode) {

			case op_gestalt:
				value = do_gestalt(inst[0].value, inst[1].value);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;

			case op_debugtrap:
#if VM_DEBUGGER
				/* We block and handle debug commands, but only if the
				   library has invoked debug features. (Meaning, has
				   the cycle handler ever been called.) */
				if (debugger_ever_invoked()) {
					debugger_block_and_debug("user debugtrap, pausing...");
					break;
				}
#endif /* VM_DEBUGGER */
				fatal_error_i("user debugtrap encountered.", inst[0].value);
				break;

			case op_jumpabs:
				pc = inst[0].value;
				break;

			case op_callf:
				push_callstub(inst[1].desttype, inst[1].value);
				enter_function(inst[0].value, 0, arglistfix);
				break;
			case op_callfi:
				arglistfix[0] = inst[1].value;
				push_callstub(inst[2].desttype, inst[2].value);
				enter_function(inst[0].value, 1, arglistfix);
				break;
			case op_callfii:
				arglistfix[0] = inst[1].value;
				arglistfix[1] = inst[2].value;
				push_callstub(inst[3].desttype, inst[3].value);
				enter_function(inst[0].value, 2, arglistfix);
				break;
			case op_callfiii:
				arglistfix[0] = inst[1].value;
				arglistfix[1] = inst[2].value;
				arglistfix[2] = inst[3].value;
				push_callstub(inst[4].desttype, inst[4].value);
				enter_function(inst[0].value, 3, arglistfix);
				break;

			case op_getmemsize:
				store_operand(inst[0].desttype, inst[0].value, endmem);
				break;
			case op_setmemsize:
				value = change_memsize(inst[0].value, false);
				store_operand(inst[1].desttype, inst[1].value, value);
				break;

			case op_getstringtbl:
				value = stream_get_table();
				store_operand(inst[0].desttype, inst[0].value, value);
				break;
			case op_setstringtbl:
				stream_set_table(inst[0].value);
				break;

			case op_getiosys:
				stream_get_iosys(&val0, &val1);
				store_operand(inst[0].desttype, inst[0].value, val0);
				store_operand(inst[1].desttype, inst[1].value, val1);
				break;
			case op_setiosys:
				stream_set_iosys(inst[0].value, inst[1].value);
				break;

			case op_glk:
				profile_in(0xF0000000 + inst[0].value, stackptr, false);
				value = inst[1].value;
				arglist = pop_arguments(value, 0);
				val0 = perform_glk(inst[0].value, value, arglist);
#ifdef TOLERATE_SUPERGLUS_BUG
				if (inst[2].desttype == 1 && inst[2].value == 0)
					inst[2].desttype = 0;
#endif /* TOLERATE_SUPERGLUS_BUG */
				store_operand(inst[2].desttype, inst[2].value, val0);
				profile_out(stackptr);
				break;

			case op_random:
				vals0 = inst[0].value;
				if (vals0 == 0)
					value = glulx_random();
				else if (vals0 >= 1)
					value = glulx_random() % (uint)(vals0);
				else
					value = -(int)(glulx_random() % (uint)(-vals0));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_setrandom:
				glulx_setrandom(inst[0].value);
				break;

			case op_verify:
				value = perform_verify();
				store_operand(inst[0].desttype, inst[0].value, value);
				break;

			case op_restart:
				profile_fail("restart");
				vm_restart();
				break;

			case op_protect:
				val0 = inst[0].value;
				val1 = val0 + inst[1].value;
				if (val0 == val1) {
					val0 = 0;
					val1 = 0;
				}
				protectstart = val0;
				protectend = val1;
				break;

			case op_save:
				push_callstub(inst[1].desttype, inst[1].value);
#ifdef TODO
				value = saveGameData(find_stream_by_id(inst[0].value), "Savegame").getCode() == Common::kNoError ? 0 : 1;
#else
				error("TODO");
#endif
				pop_callstub(value);
				break;

			case op_restore:
#ifdef TODO
				value = loadGameData(find_stream_by_id(inst[0].value)).getCode() == Common::kNoError ? 0 : 1;
#else
				error("TODO");
#endif
				if (value == 0) {
					/* We've succeeded, and the stack now contains the callstub
					   saved during saveundo. Ignore this opcode's operand. */
					value = (uint) - 1;
					pop_callstub(value);
				} else {
					/* We've failed, so we must store the failure in this opcode's
					   operand. */
					store_operand(inst[1].desttype, inst[1].value, value);
				}
				break;

			case op_saveundo:
				push_callstub(inst[0].desttype, inst[0].value);
				value = perform_saveundo();
				pop_callstub(value);
				break;

			case op_restoreundo:
				value = perform_restoreundo();
				if (value == 0) {
					/* We've succeeded, and the stack now contains the callstub
					   saved during saveundo. Ignore this opcode's operand. */
					value = (uint) - 1;
					pop_callstub(value);
				} else {
					/* We've failed, so we must store the failure in this opcode's
					   operand. */
					store_operand(inst[0].desttype, inst[0].value, value);
				}
				break;

			case op_quit:
				done_executing = true;
				break;

			case op_linearsearch:
				value = linear_search(inst[0].value, inst[1].value, inst[2].value,
				                      inst[3].value, inst[4].value, inst[5].value, inst[6].value);
				store_operand(inst[7].desttype, inst[7].value, value);
				break;
			case op_binarysearch:
				value = binary_search(inst[0].value, inst[1].value, inst[2].value,
				                      inst[3].value, inst[4].value, inst[5].value, inst[6].value);
				store_operand(inst[7].desttype, inst[7].value, value);
				break;
			case op_linkedsearch:
				value = linked_search(inst[0].value, inst[1].value, inst[2].value,
				                      inst[3].value, inst[4].value, inst[5].value);
				store_operand(inst[6].desttype, inst[6].value, value);
				break;

			case op_mzero: {
				uint lx;
				uint count = inst[0].value;
				addr = inst[1].value;
				for (lx = 0; lx < count; lx++, addr++) {
					MemW1(addr, 0);
				}
			}
			break;
			case op_mcopy: {
				uint lx;
				uint count = inst[0].value;
				uint addrsrc = inst[1].value;
				uint addrdest = inst[2].value;
				if (addrdest < addrsrc) {
					for (lx = 0; lx < count; lx++, addrsrc++, addrdest++) {
						value = Mem1(addrsrc);
						MemW1(addrdest, value);
					}
				} else {
					addrsrc += (count - 1);
					addrdest += (count - 1);
					for (lx = 0; lx < count; lx++, addrsrc--, addrdest--) {
						value = Mem1(addrsrc);
						MemW1(addrdest, value);
					}
				}
			}
			break;
			case op_malloc:
				value = heap_alloc(inst[0].value);
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_mfree:
				heap_free(inst[0].value);
				break;

			case op_accelfunc:
				accel_set_func(inst[0].value, inst[1].value);
				break;
			case op_accelparam:
				accel_set_param(inst[0].value, inst[1].value);
				break;

#ifdef FLOAT_SUPPORT

			case op_numtof:
				vals0 = inst[0].value;
				value = encode_float((gfloat32)vals0);
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_ftonumz:
				valf = decode_float(inst[0].value);
				if (!signbit(valf)) {
					if (isnan(valf) || isinf(valf) || (valf > 2147483647.0))
						vals0 = 0x7FFFFFFF;
					else
						vals0 = (int)(truncf(valf));
				} else {
					if (isnan(valf) || isinf(valf) || (valf < -2147483647.0))
						vals0 = 0x80000000;
					else
						vals0 = (int)(truncf(valf));
				}
				store_operand(inst[1].desttype, inst[1].value, vals0);
				break;
			case op_ftonumn:
				valf = decode_float(inst[0].value);
				if (!signbit(valf)) {
					if (isnan(valf) || isinf(valf) || (valf > 2147483647.0))
						vals0 = 0x7FFFFFFF;
					else
						vals0 = (int)(roundf(valf));
				} else {
					if (isnan(valf) || isinf(valf) || (valf < -2147483647.0))
						vals0 = 0x80000000;
					else
						vals0 = (int)(roundf(valf));
				}
				store_operand(inst[1].desttype, inst[1].value, vals0);
				break;

			case op_fadd:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				value = encode_float(valf1 + valf2);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_fsub:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				value = encode_float(valf1 - valf2);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_fmul:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				value = encode_float(valf1 * valf2);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;
			case op_fdiv:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				value = encode_float(valf1 / valf2);
				store_operand(inst[2].desttype, inst[2].value, value);
				break;

			case op_fmod:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				valf = fmodf(valf1, valf2);
				val0 = encode_float(valf);
				val1 = encode_float((valf1 - valf) / valf2);
				if (val1 == 0x0 || val1 == 0x80000000) {
					/* When the quotient is zero, the sign has been lost in the
					   shuffle. We'll set that by hand, based on the original
					   arguments. */
					val1 = (inst[0].value ^ inst[1].value) & 0x80000000;
				}
				store_operand(inst[2].desttype, inst[2].value, val0);
				store_operand(inst[3].desttype, inst[3].value, val1);
				break;

			case op_floor:
				valf = decode_float(inst[0].value);
				value = encode_float(floorf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_ceil:
				valf = decode_float(inst[0].value);
				value = encode_float(ceilf(valf));
				if (value == 0x0 || value == 0x80000000) {
					/* When the result is zero, the sign may have been lost in the
					   shuffle. (This is a bug in some C libraries.) We'll set the
					   sign by hand, based on the original argument. */
					value = inst[0].value & 0x80000000;
				}
				store_operand(inst[1].desttype, inst[1].value, value);
				break;

			case op_sqrt:
				valf = decode_float(inst[0].value);
				value = encode_float(sqrtf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_log:
				valf = decode_float(inst[0].value);
				value = encode_float(logf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_exp:
				valf = decode_float(inst[0].value);
				value = encode_float(expf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_pow:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				value = encode_float(glulx_powf(valf1, valf2));
				store_operand(inst[2].desttype, inst[2].value, value);
				break;

			case op_sin:
				valf = decode_float(inst[0].value);
				value = encode_float(sinf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_cos:
				valf = decode_float(inst[0].value);
				value = encode_float(cosf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_tan:
				valf = decode_float(inst[0].value);
				value = encode_float(tanf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_asin:
				valf = decode_float(inst[0].value);
				value = encode_float(asinf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_acos:
				valf = decode_float(inst[0].value);
				value = encode_float(acosf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_atan:
				valf = decode_float(inst[0].value);
				value = encode_float(atanf(valf));
				store_operand(inst[1].desttype, inst[1].value, value);
				break;
			case op_atan2:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				value = encode_float(atan2f(valf1, valf2));
				store_operand(inst[2].desttype, inst[2].value, value);
				break;

			case op_jisinf:
				/* Infinity is well-defined, so we don't bother to convert to
				   float. */
				val0 = inst[0].value;
				if (val0 == 0x7F800000 || val0 == 0xFF800000) {
					value = inst[1].value;
					goto PerformJump;
				}
				break;
			case op_jisnan:
				/* NaN is well-defined, so we don't bother to convert to
				   float. */
				val0 = inst[0].value;
				if ((val0 & 0x7F800000) == 0x7F800000 && (val0 & 0x007FFFFF) != 0) {
					value = inst[1].value;
					goto PerformJump;
				}
				break;

			case op_jfeq:
				if ((inst[2].value & 0x7F800000) == 0x7F800000 && (inst[2].value & 0x007FFFFF) != 0) {
					/* The delta is NaN, which can never match. */
					val0 = 0;
				} else if ((inst[0].value == 0x7F800000 || inst[0].value == 0xFF800000)
				           && (inst[1].value == 0x7F800000 || inst[1].value == 0xFF800000)) {
					/* Both are infinite. Opposite infinities are never equal,
					   even if the difference is infinite, so this is easy. */
					val0 = (inst[0].value == inst[1].value);
				} else {
					valf1 = decode_float(inst[1].value) - decode_float(inst[0].value);
					valf2 = fabs(decode_float(inst[2].value));
					val0 = (valf1 <= valf2 && valf1 >= -valf2);
				}
				if (val0) {
					value = inst[3].value;
					goto PerformJump;
				}
				break;
			case op_jfne:
				if ((inst[2].value & 0x7F800000) == 0x7F800000 && (inst[2].value & 0x007FFFFF) != 0) {
					/* The delta is NaN, which can never match. */
					val0 = 0;
				} else if ((inst[0].value == 0x7F800000 || inst[0].value == 0xFF800000)
				           && (inst[1].value == 0x7F800000 || inst[1].value == 0xFF800000)) {
					/* Both are infinite. Opposite infinities are never equal,
					   even if the difference is infinite, so this is easy. */
					val0 = (inst[0].value == inst[1].value);
				} else {
					valf1 = decode_float(inst[1].value) - decode_float(inst[0].value);
					valf2 = fabs(decode_float(inst[2].value));
					val0 = (valf1 <= valf2 && valf1 >= -valf2);
				}
				if (!val0) {
					value = inst[3].value;
					goto PerformJump;
				}
				break;

			case op_jflt:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				if (valf1 < valf2) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jfgt:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				if (valf1 > valf2) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jfle:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				if (valf1 <= valf2) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;
			case op_jfge:
				valf1 = decode_float(inst[0].value);
				valf2 = decode_float(inst[1].value);
				if (valf1 >= valf2) {
					value = inst[2].value;
					goto PerformJump;
				}
				break;

#endif /* FLOAT_SUPPORT */

#ifdef GLULX_EXTEND_OPCODES
				GLULX_EXTEND_OPCODES
#endif /* GLULX_EXTEND_OPCODES */

			default:
				fatal_error_i("Executed unknown opcode.", opcode);
			}
		}
	}
	/* done executing */
#if VM_DEBUGGER
	debugger_handle_quit();
#endif /* VM_DEBUGGER */
}

} // End of namespace Glulxe
} // End of namespace Glk
