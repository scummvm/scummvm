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

#include "common/debug.h"
#include "common/stack.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"	// for g_debug_weak_validations
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/intmap.h"
#include "sci/engine/kernel.h"
#include "sci/engine/kernel_types.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/gc.h"
#include "sci/sfx/misc.h"	// for sfx_reset_player

namespace Sci {

reg_t NULL_REG = {0, 0};

//#define VM_DEBUG_SEND
#undef STRICT_SEND // Disallows variable sends with more than one parameter
#undef STRICT_READ // Disallows reading from out-of-bounds parameters and locals


int script_abort_flag = 0; // Set to 1 to abort execution. Set to 2 to force a replay afterwards	// FIXME: Avoid non-const global vars
int script_step_counter = 0; // Counts the number of steps executed	// FIXME: Avoid non-const global vars
int script_gc_interval = GC_INTERVAL; // Number of steps in between gcs	// FIXME: Avoid non-const global vars

static bool breakpointFlag = false;	// FIXME: Avoid non-const global vars
static reg_t _dummy_register;		// FIXME: Avoid non-const global vars

// validation functionality

#ifndef DISABLE_VALIDATIONS

static reg_t &validate_property(Object *obj, int index) {
	if (!obj) {
		debugC(2, kDebugLevelVM, "[VM] Sending to disposed object!\n");
		_dummy_register = NULL_REG;
		return _dummy_register;
	}

	if (index < 0 || (uint)index >= obj->_variables.size()) {
		debugC(2, kDebugLevelVM, "[VM] Invalid property #%d (out of [0..%d]) requested!\n", 
			index, obj->_variables.size());
		_dummy_register = NULL_REG;
		return _dummy_register;
	}

	return obj->_variables[index];
}

static StackPtr validate_stack_addr(EngineState *s, StackPtr sp) {
	if (sp >= s->stack_base && sp < s->stack_top)
		return sp;

	error("[VM] Stack index %d out of valid range [%d..%d]\n", 
		(int)(sp - s->stack_base), 0, (int)(s->stack_top - s->stack_base - 1));
	return 0;
}

static int validate_arithmetic(reg_t reg) {
	if (reg.segment) {
		if (g_debug_weak_validations)
			warning("[VM] Attempt to read arithmetic value from non-zero segment [%04x]\n", reg.segment);
		else
			error("[VM] Attempt to read arithmetic value from non-zero segment [%04x]\n", reg.segment);
		return 0;
	}

	return reg.offset;
}

static int signed_validate_arithmetic(reg_t reg) {
	if (reg.segment) {
		if (g_debug_weak_validations)
			warning("[VM] Attempt to read arithmetic value from non-zero segment [%04x]\n", reg.segment);
		else
			error("[VM] Attempt to read arithmetic value from non-zero segment [%04x]\n", reg.segment);
		return 0;
	}

	if (reg.offset&0x8000)
		return (signed)(reg.offset) - 65536;
	else
		return reg.offset;
}

static int validate_variable(reg_t *r, reg_t *stack_base, int type, int max, int index, int line) {
	const char *names[4] = {"global", "local", "temp", "param"};

	if (index < 0 || index >= max) {
		char txt[200];
		char tmp[40];
		sprintf(txt, "[VM] validate_variable(): Attempt to use invalid %s variable %04x ", names[type], index);
		if (max == 0)
			strcat(txt, "(variable type invalid)");
		else {
			sprintf(tmp, "(out of range [%d..%d])", 0, max - 1);
			strcat(txt, tmp);
		}

		if (g_debug_weak_validations)
			warning("%s", txt);
		else
			error("%s", txt);

#ifdef STRICT_READ
		return 1;
#else // !STRICT_READ
		if (type == VAR_PARAM || type == VAR_TEMP) {
			int total_offset = r - stack_base;
			if (total_offset < 0 || total_offset >= VM_STACK_SIZE) {
				warning("[VM] Access would be outside even of the stack (%d); access denied", total_offset);
				return 1;
			} else {
				debugC(2, kDebugLevelVM, "[VM] Access within stack boundaries; access granted.\n");
				return 0;
			}
		}
#endif
	}

	return 0;
}

static reg_t validate_read_var(reg_t *r, reg_t *stack_base, int type, int max, int index, int line, reg_t default_value) {
	if (!validate_variable(r, stack_base, type, max, index, line))
		return r[index];
	else
		return default_value;
}

static void validate_write_var(reg_t *r, reg_t *stack_base, int type, int max, int index, int line, reg_t value) {
	if (!validate_variable(r, stack_base, type, max, index, line))
		r[index] = value;
}

#  define ASSERT_ARITHMETIC(v) validate_arithmetic(v)

#else
// Non-validating alternatives

#  define validate_stack_addr(s, sp) sp
#  define validate_arithmetic(r) ((r).offset)
#  define signed_validate_arithmetic(r) ((int) ((r).offset) & 0x8000 ? (signed) ((r).offset) - 65536 : ((r).offset))
#  define validate_variable(r, sb, t, m, i, l)
#  define validate_read_var(r, sb, t, m, i, l) ((r)[i])
#  define validate_write_var(r, sb, t, m, i, l, v) ((r)[i] = (v))
#  define validate_property(o, p) ((o)->_variables[p])
#  define ASSERT_ARITHMETIC(v) (v).offset

#endif

#define READ_VAR(type, index, def) validate_read_var(scriptState.variables[type], s->stack_base, type, scriptState.variables_max[type], index, __LINE__, def)
#define WRITE_VAR(type, index, value) validate_write_var(scriptState.variables[type], s->stack_base, type, scriptState.variables_max[type], index, __LINE__, value)
#define WRITE_VAR16(type, index, value) WRITE_VAR(type, index, make_reg(0, value));

#define ACC_ARITHMETIC_L(op) make_reg(0, (op validate_arithmetic(s->r_acc)))
#define ACC_AUX_LOAD() aux_acc = signed_validate_arithmetic(s->r_acc)
#define ACC_AUX_STORE() s->r_acc = make_reg(0, aux_acc)

#define OBJ_PROPERTY(o, p) (validate_property(o, p))

// Operating on the stack
// 16 bit:
#define PUSH(v) PUSH32(make_reg(0, v))
#define POP() (validate_arithmetic(POP32()))
// 32 bit:
#define PUSH32(a) (*(validate_stack_addr(s, (scriptState.xs->sp)++)) = (a))
#define POP32() (*(validate_stack_addr(s, --(scriptState.xs->sp))))

// Getting instruction parameters
#define GET_OP_BYTE() ((uint8)code_buf[(scriptState.xs->addr.pc.offset)++])
#define GET_OP_WORD() (READ_LE_UINT16(code_buf + ((scriptState.xs->addr.pc.offset) += 2) - 2))
#define GET_OP_FLEX() ((opcode & 1)? GET_OP_BYTE() : GET_OP_WORD())
#define GET_OP_SIGNED_BYTE() ((int8)(code_buf[(scriptState.xs->addr.pc.offset)++]))
#define GET_OP_SIGNED_WORD() (((int16)READ_LE_UINT16(code_buf + ((scriptState.xs->addr.pc.offset) += 2) - 2)))
#define GET_OP_SIGNED_FLEX() ((opcode & 1)? GET_OP_SIGNED_BYTE() : GET_OP_SIGNED_WORD())

ExecStack *execute_method(EngineState *s, uint16 script, uint16 pubfunct, StackPtr sp, reg_t calling_obj, uint16 argc, StackPtr argp) {
	int seg = s->seg_manager->segGet(script);
	Script *scr = s->seg_manager->getScriptIfLoaded(seg);

	if (!scr)  // Script not present yet?
		seg = script_instantiate(s->resmgr, s->seg_manager, s->_version, ((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader(), script);
	else
		scr->unmarkDeleted();

	int temp = s->seg_manager->validateExportFunc(pubfunct, seg);
	if (!temp) {
		error("Request for invalid exported function 0x%x of script 0x%x\n", pubfunct, script);
		return NULL;
	}

	// Check if a breakpoint is set on this method
	if (s->have_bp & BREAK_EXPORT) {
		Breakpoint *bp;
		uint32 bpaddress;

		bpaddress = (script << 16 | pubfunct);

		bp = s->bp_list;
		while (bp) {
			if (bp->type == BREAK_EXPORT && bp->data.address == bpaddress) {
				Console *con = ((SciEngine *)g_engine)->getSciDebugger();
				con->DebugPrintf("Break on script %d, export %d\n", script, pubfunct);
				scriptState.debugging = true;
				breakpointFlag = true;
				break;
			}
			bp = bp->next;
		}
	}

	return add_exec_stack_entry(s, make_reg(seg, temp), sp, calling_obj, argc, argp, -1, calling_obj, s->_executionStack.size()-1, seg);
}


static void _exec_varselectors(EngineState *s) {
	// Executes all varselector read/write ops on the TOS
	while (!s->_executionStack.empty() && s->_executionStack.back().type == EXEC_STACK_TYPE_VARSELECTOR) {
		ExecStack &xs = s->_executionStack.back();
		// varselector access?
		if (xs.argc) { // write?
			*(xs.getVarPointer(s)) = xs.variables_argp[1];

		} else // No, read
			s->r_acc = *(xs.getVarPointer(s));

		s->_executionStack.pop_back();
	}
}

ExecStack *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj, StackPtr sp, int framesize, StackPtr argp) {
// send_obj and work_obj are equal for anything but 'super'
// Returns a pointer to the TOS exec_stack element
	assert(s);

	reg_t funcp;
	int selector;
	int argc;
	int origin = s->_executionStack.size()-1; // Origin: Used for debugging
	int print_send_action = 0;
	// We return a pointer to the new active ExecStack

	// The selector calls we catch are stored below:
	Common::Stack<CallsStruct> sendCalls;

	while (framesize > 0) {
		selector = validate_arithmetic(*argp++);
		argc = validate_arithmetic(*argp);

		if (argc > 0x800) { // More arguments than the stack could possibly accomodate for
			error("send_selector(): More than 0x800 arguments to function call");
		}

		// Check if a breakpoint is set on this method
		if (s->have_bp & BREAK_SELECTOR) {
			Breakpoint *bp;
			char method_name [256];

			sprintf(method_name, "%s::%s", obj_get_name(s->seg_manager, s->_version, send_obj), ((SciEngine*)g_engine)->getKernel()->getSelectorName(selector).c_str());

			bp = s->bp_list;
			while (bp) {
				int cmplen = strlen(bp->data.name);
				if (bp->data.name[cmplen - 1] != ':')
					cmplen = 256;

				if (bp->type == BREAK_SELECTOR && !strncmp(bp->data.name, method_name, cmplen)) {
					Console *con = ((SciEngine *)g_engine)->getSciDebugger();
					con->DebugPrintf("Break on %s (in [%04x:%04x])\n", method_name, PRINT_REG(send_obj));
					print_send_action = 1;
					breakpointFlag = true;
					scriptState.debugging = true;
					break;
				}
				bp = bp->next;
			}
		}

#ifdef VM_DEBUG_SEND
		printf("Send to %04x:%04x, selector %04x (%s):", PRINT_REG(send_obj), selector, s->_selectorNames[selector].c_str());
#endif // VM_DEBUG_SEND

		ObjVarRef varp;
		switch (lookup_selector(s, send_obj, selector, &varp, &funcp)) {
		case kSelectorNone:
			// WORKAROUND: LSL6 tries to access the invalid 'keep' selector of the game object.
			// FIXME: Find out if this is a game bug.
			if ((s->_gameName == "LSL6") && (selector == 0x18c)) {
				debug("LSL6 detected, continuing...");
				break;
			}

			error("Send to invalid selector 0x%x of object at %04x:%04x\n", 0xffff & selector, PRINT_REG(send_obj));

			break;

		case kSelectorVariable:

#ifdef VM_DEBUG_SEND
			if (argc)
				printf("Varselector: Write %04x:%04x\n", PRINT_REG(argp[1]));
			else
				printf("Varselector: Read\n");
#endif // VM_DEBUG_SEND

			switch (argc) {
			case 0:   // Read selector
				if (print_send_action) {
					printf("[read selector]\n");
					print_send_action = 0;
				}
				// fallthrough
			case 1:
#ifndef STRICT_SEND
			default:
#endif
				{ // Argument is supplied -> Selector should be set
					if (print_send_action) {
						reg_t oldReg = *varp.getPointer(s);
						reg_t newReg = argp[1];

						printf("[write to selector: change %04x:%04x to %04x:%04x]\n", PRINT_REG(oldReg), PRINT_REG(newReg));
						print_send_action = 0;
					}
					CallsStruct call;
					call.address.var = varp; // register the call
					call.argp = argp;
					call.argc = argc;
					call.selector = selector;
					call.type = EXEC_STACK_TYPE_VARSELECTOR; // Register as a varselector
					sendCalls.push(call);
				}
				break;
#ifdef STRICT_SEND
			default:
				scriptState.seeking = scriptState.runningStep = 0;
				error("Send error: Variable selector %04x in %04x:%04x called with %04x params", selector, PRINT_REG(send_obj), argc);
#endif
			}
			break;

		case kSelectorMethod:

#ifdef VM_DEBUG_SEND
			printf("Funcselector(");
			for (int i = 0; i < argc; i++) {
				printf(PREG, PRINT_REG(argp[i+1]));
				if (i + 1 < argc)
					printf(", ");
			}
			printf(") at %04x:%04x\n", PRINT_REG(funcp));
#endif // VM_DEBUG_SEND
			if (print_send_action) {
				printf("[invoke selector]\n");
				print_send_action = 0;
			}

			CallsStruct call;
			call.address.func = funcp; // register call
			call.argp = argp;
			call.argc = argc;
			call.selector = selector;
			call.type = EXEC_STACK_TYPE_CALL;
			call.sp = sp;
			sp = CALL_SP_CARRY; // Destroy sp, as it will be carried over
			sendCalls.push(call);

			break;
		} // switch(lookup_selector())

		framesize -= (2 + argc);
		argp += argc + 1;
	}

	// Iterate over all registered calls in the reverse order. This way, the first call is
	// placed on the TOS; as soon as it returns, it will cause the second call to be executed.
	while (!sendCalls.empty()) {
		CallsStruct call = sendCalls.pop();
		if (call.type == EXEC_STACK_TYPE_VARSELECTOR) // Write/read variable?
			add_exec_stack_varselector(s, work_obj, call.argc, call.argp,
			                                    call.selector, call.address.var, origin);
		else
			add_exec_stack_entry(s, call.address.func, call.sp, work_obj,
			                         call.argc, call.argp,
			                         call.selector, send_obj, origin, SCI_XS_CALLEE_LOCALS);
	}

	_exec_varselectors(s);

	if (s->_executionStack.empty())
		return NULL;
	return &(s->_executionStack.back());
}

ExecStack *add_exec_stack_varselector(EngineState *s, reg_t objp, int argc, StackPtr argp, Selector selector, const ObjVarRef& address, int origin) {
	ExecStack *xstack = add_exec_stack_entry(s, NULL_REG, 0, objp, argc, argp, selector, objp, origin, SCI_XS_CALLEE_LOCALS);
	// Store selector address in sp

	xstack->addr.varp = address;
	xstack->type = EXEC_STACK_TYPE_VARSELECTOR;

	return xstack;
}

ExecStack *add_exec_stack_entry(EngineState *s, reg_t pc, StackPtr sp, reg_t objp, int argc,
								   StackPtr argp, Selector selector, reg_t sendp, int origin, SegmentId locals_segment) {
	// Returns new TOS element for the execution stack
	// locals_segment may be -1 if derived from the called object

	//printf("Exec stack: [%d/%d], origin %d, at %p\n", s->execution_stack_pos, s->_executionStack.size(), origin, s->execution_stack);

	ExecStack xstack;

	xstack.objp = objp;
	if (locals_segment != SCI_XS_CALLEE_LOCALS)
		xstack.local_segment = locals_segment;
	else
		xstack.local_segment = pc.segment;

	xstack.sendp = sendp;
	xstack.addr.pc = pc;
	xstack.fp = xstack.sp = sp;
	xstack.argc = argc;

	xstack.variables_argp = argp; // Parameters

	*argp = make_reg(0, argc);  // SCI code relies on the zeroeth argument to equal argc

	// Additional debug information
	xstack.selector = selector;
	xstack.origin = origin;

	xstack.type = EXEC_STACK_TYPE_CALL; // Normal call

	s->_executionStack.push_back(xstack);
	return &(s->_executionStack.back());
}

#ifdef DISABLE_VALIDATONS
#  define kernel_matches_signature(a, b, c, d) 1
#endif

void vm_handle_fatal_error(EngineState *s, int line, const char *file) {
	error("Fatal VM error in %s, L%d; aborting...", file, line);
}

static reg_t pointer_add(EngineState *s, reg_t base, int offset) {
	MemObject *mobj = GET_SEGMENT_ANY(*s->seg_manager, base.segment);

	if (!mobj) {
		error("[VM] Error: Attempt to add %d to invalid pointer %04x:%04x", offset, PRINT_REG(base));
		return NULL_REG;
	}

	switch (mobj->getType()) {

	case MEM_OBJ_LOCALS:
		base.offset += 2 * offset;
		return base;

	case MEM_OBJ_SCRIPT:
	case MEM_OBJ_STACK:
	case MEM_OBJ_DYNMEM:
		base.offset += offset;
		return base;
		break;

	default:
		error("[VM] Error: Attempt to add %d to pointer %04x:%04x: Pointer arithmetics of this type unsupported", offset, PRINT_REG(base));
		return NULL_REG;

	}
}

static void gc_countdown(EngineState *s) {
	if (s->gc_countdown-- <= 0) {
		s->gc_countdown = script_gc_interval;
		run_gc(s);
	}
}

static const byte _fake_return_buffer[2] = {op_ret << 1, op_ret << 1};

void run_vm(EngineState *s, int restoring) {
	assert(s);

#ifndef DISABLE_VALIDATIONS
	unsigned int code_buf_size = 0 ; // (Avoid spurious warning)
#endif
	int temp;
	int16 aux_acc; // Auxiliary 16 bit accumulator
	reg_t r_temp; // Temporary register
	StackPtr s_temp; // Temporary stack pointer
	int16 opparams[4]; // opcode parameters

	scriptState.restAdjust = s->restAdjust;
	// &rest adjusts the parameter count by this value
	// Current execution data:
	scriptState.xs = &(s->_executionStack.back());
	ExecStack *xs_new = NULL;
	Object *obj = obj_get(s->seg_manager, s->_version, scriptState.xs->objp);
	Script *local_script = s->seg_manager->getScriptIfLoaded(scriptState.xs->local_segment);
	int old_execution_stack_base = s->execution_stack_base;
	// Used to detect the stack bottom, for "physical" returns
	const byte *code_buf = NULL; // (Avoid spurious warning)

	if (!local_script) {
		error("run_vm(): program counter gone astray (local_script pointer is null)");
	}

	if (!restoring)
		s->execution_stack_base = s->_executionStack.size()-1;

#ifndef DISABLE_VALIDATIONS
	// Initialize maximum variable count
	if (s->script_000->locals_block)
		scriptState.variables_max[VAR_GLOBAL] = s->script_000->locals_block->_locals.size();
	else
		scriptState.variables_max[VAR_GLOBAL] = 0;
#endif

	scriptState.variables_seg[VAR_GLOBAL] = s->script_000->locals_segment;
	scriptState.variables_seg[VAR_TEMP] = scriptState.variables_seg[VAR_PARAM] = s->stack_segment;
	scriptState.variables_base[VAR_TEMP] = scriptState.variables_base[VAR_PARAM] = s->stack_base;

	// SCI code reads the zeroeth argument to determine argc
	if (s->script_000->locals_block)
		scriptState.variables_base[VAR_GLOBAL] = scriptState.variables[VAR_GLOBAL] = s->script_000->locals_block->_locals.begin();
	else
		scriptState.variables_base[VAR_GLOBAL] = scriptState.variables[VAR_GLOBAL] = NULL;

	s->_executionStackPosChanged = true; // Force initialization

	while (1) {
		byte opcode;
		byte opnumber;
		int var_type; // See description below
		int var_number;

		scriptState.old_pc_offset = scriptState.xs->addr.pc.offset;
		scriptState.old_sp = scriptState.xs->sp;

		if (s->_executionStackPosChanged) {
			Script *scr;
			scriptState.xs = &(s->_executionStack.back());
			s->_executionStackPosChanged = false;

			scr = s->seg_manager->getScriptIfLoaded(scriptState.xs->addr.pc.segment);
			if (!scr) {
				// No script? Implicit return via fake instruction buffer
				warning("Running on non-existant script in segment %x", scriptState.xs->addr.pc.segment);
				code_buf = _fake_return_buffer;
#ifndef DISABLE_VALIDATIONS
				code_buf_size = 2;
#endif
				scriptState.xs->addr.pc.offset = 1;

				scr = NULL;
				obj = NULL;
			} else {
				obj = obj_get(s->seg_manager, s->_version, scriptState.xs->objp);
				code_buf = scr->buf;
#ifndef DISABLE_VALIDATIONS
				code_buf_size = scr->buf_size;
#endif
				local_script = s->seg_manager->getScriptIfLoaded(scriptState.xs->local_segment);
				if (!local_script) {
					warning("Could not find local script from segment %x", scriptState.xs->local_segment);
					local_script = NULL;
					scriptState.variables_base[VAR_LOCAL] = scriptState.variables[VAR_LOCAL] = NULL;
#ifndef DISABLE_VALIDATIONS
					scriptState.variables_max[VAR_LOCAL] = 0;
#endif
				} else {

					scriptState.variables_seg[VAR_LOCAL] = local_script->locals_segment;
					if (local_script->locals_block)
						scriptState.variables_base[VAR_LOCAL] = scriptState.variables[VAR_LOCAL] = local_script->locals_block->_locals.begin();
					else
						scriptState.variables_base[VAR_LOCAL] = scriptState.variables[VAR_LOCAL] = NULL;
#ifndef DISABLE_VALIDATIONS
					if (local_script->locals_block)
						scriptState.variables_max[VAR_LOCAL] = local_script->locals_block->_locals.size();
					else
						scriptState.variables_max[VAR_LOCAL] = 0;
					scriptState.variables_max[VAR_TEMP] = scriptState.xs->sp - scriptState.xs->fp;
					scriptState.variables_max[VAR_PARAM] = scriptState.xs->argc + 1;
#endif
				}
				scriptState.variables[VAR_TEMP] = scriptState.xs->fp;
				scriptState.variables[VAR_PARAM] = scriptState.xs->variables_argp;
			}

		}

		if (script_abort_flag || g_engine->shouldQuit())
			return; // Emergency

		// Debug if this has been requested:
		// TODO: re-implement sci_debug_flags
		if (scriptState.debugging /* sci_debug_flags*/) {
			script_debug(s, breakpointFlag);
			breakpointFlag = false;
		}
		Console *con = ((Sci::SciEngine*)g_engine)->getSciDebugger();
		if (con->isAttached()) {
			con->onFrame();
		}

#ifndef DISABLE_VALIDATIONS
		if (scriptState.xs->sp < scriptState.xs->fp)
			error("run_vm(): stack underflow");

		scriptState.variables_max[VAR_TEMP] = scriptState.xs->sp - scriptState.xs->fp;

		if (scriptState.xs->addr.pc.offset >= code_buf_size)
			error("run_vm(): program counter gone astray");
#endif

		opcode = GET_OP_BYTE(); // Get opcode

		opnumber = opcode >> 1;

		for (temp = 0; g_opcode_formats[opnumber][temp]; temp++)
			switch (g_opcode_formats[opnumber][temp]) {

			case Script_Byte:
				opparams[temp] = GET_OP_BYTE();
				break;
			case Script_SByte:
				opparams[temp] = GET_OP_SIGNED_BYTE();
				break;

			case Script_Word:
				opparams[temp] = GET_OP_WORD();
				break;
			case Script_SWord:
				opparams[temp] = GET_OP_SIGNED_WORD();
				break;

			case Script_Variable:
			case Script_Property:

			case Script_Local:
			case Script_Temp:
			case Script_Global:
			case Script_Param:
				opparams[temp] = GET_OP_FLEX();
				break;

			case Script_SVariable:
			case Script_SRelative:
				opparams[temp] = GET_OP_SIGNED_FLEX();
				break;

			case Script_Offset:
				opparams[temp] = GET_OP_FLEX();
				break;

			case Script_None:
			case Script_End:
				break;

			case Script_Invalid:
			default:
				error("opcode %02x: Invalid", opcode);
			}

		// TODO: Replace the following by an opcode table, and several methods for
		// each opcode.
		switch (opnumber) {

		case 0x00: // bnot
			s->r_acc = ACC_ARITHMETIC_L(0xffff ^ /*acc*/);
			break;

		case 0x01: // add
			r_temp = POP32();
			if (r_temp.segment || s->r_acc.segment) {
				reg_t r_ptr = NULL_REG;
				int offset;
				// Pointer arithmetics!
				if (s->r_acc.segment) {
					if (r_temp.segment) {
						error("Attempt to add two pointers, stack=%04x:%04x and acc=%04x:%04x",
						          PRINT_REG(r_temp), PRINT_REG(s->r_acc));
						offset = 0;
					} else {
						r_ptr = s->r_acc;
						offset = r_temp.offset;
					}
				} else {
					r_ptr = r_temp;
					offset = s->r_acc.offset;
				}

				s->r_acc = pointer_add(s, r_ptr, offset);

			} else
				s->r_acc = make_reg(0, r_temp.offset + s->r_acc.offset);
			break;

		case 0x02: // sub
			r_temp = POP32();
			if (r_temp.segment || s->r_acc.segment) {
				reg_t r_ptr = NULL_REG;
				int offset;
				// Pointer arithmetics!
				if (s->r_acc.segment) {
					if (r_temp.segment) {
						error("Attempt to subtract two pointers, stack=%04x:%04x and acc=%04x:%04x",
						          PRINT_REG(r_temp), PRINT_REG(s->r_acc));
						offset = 0;
					} else {
						r_ptr = s->r_acc;
						offset = r_temp.offset;
					}
				} else {
					r_ptr = r_temp;
					offset = s->r_acc.offset;
				}

				s->r_acc = pointer_add(s, r_ptr, -offset);

			} else
				s->r_acc = make_reg(0, r_temp.offset - s->r_acc.offset);
			break;

		case 0x03: // mul
			s->r_acc = ACC_ARITHMETIC_L(((int16)POP()) * (int16)/*acc*/);
			break;

		case 0x04: // div
			ACC_AUX_LOAD();
			aux_acc = aux_acc != 0 ? ((int16)POP()) / aux_acc : 0;
			ACC_AUX_STORE();
			break;

		case 0x05: // mod
			ACC_AUX_LOAD();
			aux_acc = aux_acc != 0 ? ((int16)POP()) % aux_acc : 0;
			ACC_AUX_STORE();
			break;

		case 0x06: // shr
			s->r_acc = ACC_ARITHMETIC_L(((uint16) POP()) >> /*acc*/);
			break;

		case 0x07: // shl
			s->r_acc = ACC_ARITHMETIC_L(((uint16)POP()) << /*acc*/);
			break;

		case 0x08: // xor
			s->r_acc = ACC_ARITHMETIC_L(POP() ^ /*acc*/);
			break;

		case 0x09: // and
			s->r_acc = ACC_ARITHMETIC_L(POP() & /*acc*/);
			break;

		case 0x0a: // or
			s->r_acc = ACC_ARITHMETIC_L(POP() | /*acc*/);
			break;

		case 0x0b: // neg
			s->r_acc = ACC_ARITHMETIC_L(-/*acc*/);
			break;

		case 0x0c: // not
			s->r_acc = make_reg(0, !(s->r_acc.offset || s->r_acc.segment));
			// Must allow pointers to be negated, as this is used for checking whether objects exist
			break;

		case 0x0d: // eq?
			s->r_prev = s->r_acc;
			r_temp = POP32();
			s->r_acc = make_reg(0, r_temp == s->r_acc);
			// Explicitly allow pointers to be compared
			break;

		case 0x0e: // ne?
			s->r_prev = s->r_acc;
			r_temp = POP32();
			s->r_acc = make_reg(0, r_temp != s->r_acc);
			// Explicitly allow pointers to be compared
			break;

		case 0x0f: // gt?
			s->r_prev = s->r_acc;
			s->r_acc = ACC_ARITHMETIC_L((int16)POP() > (int16)/*acc*/);
			break;

		case 0x10: // ge?
			s->r_prev = s->r_acc;
			s->r_acc = ACC_ARITHMETIC_L((int16)POP() >= (int16)/*acc*/);
			break;

		case 0x11: // lt?
			s->r_prev = s->r_acc;
			s->r_acc = ACC_ARITHMETIC_L((int16)POP() < (int16)/*acc*/);
			break;

		case 0x12: // le?
			s->r_prev = s->r_acc;
			s->r_acc = ACC_ARITHMETIC_L((int16)POP() <= (int16)/*acc*/);
			break;

		case 0x13: // ugt?
			s->r_prev = s->r_acc;
			r_temp = POP32();
			s->r_acc = make_reg(0, (r_temp.segment == s->r_acc.segment) && r_temp.offset > s->r_acc.offset);
			break;

		case 0x14: // uge?
			s->r_prev = s->r_acc;
			r_temp = POP32();
			s->r_acc = make_reg(0, (r_temp.segment == s->r_acc.segment) && r_temp.offset >= s->r_acc.offset);
			break;

		case 0x15: // ult?
			s->r_prev = s->r_acc;
			r_temp = POP32();
			s->r_acc = make_reg(0, (r_temp.segment == s->r_acc.segment) && r_temp.offset < s->r_acc.offset);
			break;

		case 0x16: // ule?
			s->r_prev = s->r_acc;
			r_temp = POP32();
			s->r_acc = make_reg(0, (r_temp.segment == s->r_acc.segment) && r_temp.offset <= s->r_acc.offset);
			break;

		case 0x17: // bt
			if (s->r_acc.offset || s->r_acc.segment)
				scriptState.xs->addr.pc.offset += opparams[0];
			break;

		case 0x18: // bnt
			if (!(s->r_acc.offset || s->r_acc.segment))
				scriptState.xs->addr.pc.offset += opparams[0];
			break;

		case 0x19: // jmp
			scriptState.xs->addr.pc.offset += opparams[0];
			break;

		case 0x1a: // ldi
			s->r_acc = make_reg(0, opparams[0]);
			break;

		case 0x1b: // push
			PUSH32(s->r_acc);
			break;

		case 0x1c: // pushi
			PUSH(opparams[0]);
			break;

		case 0x1d: // toss
			scriptState.xs->sp--;
			break;

		case 0x1e: // dup
			r_temp = scriptState.xs->sp[-1];
			PUSH32(r_temp);
			break;

		case 0x1f: { // link
			int i;
			for (i = 0; i < opparams[0]; i++)
				scriptState.xs->sp[i] = NULL_REG;
			scriptState.xs->sp += opparams[0];
			break;
		}

		case 0x20: { // call
			int argc = (opparams[1] >> 1) // Given as offset, but we need count
			           + 1 + scriptState.restAdjust;
			StackPtr call_base = scriptState.xs->sp - argc;
			scriptState.xs->sp[1].offset += scriptState.restAdjust;

			xs_new = add_exec_stack_entry(s, make_reg(scriptState.xs->addr.pc.segment,
											scriptState.xs->addr.pc.offset + opparams[0]),
											scriptState.xs->sp, scriptState.xs->objp, 
											(validate_arithmetic(*call_base)) + scriptState.restAdjust,
											call_base, NULL_SELECTOR, scriptState.xs->objp,
											s->_executionStack.size()-1, scriptState.xs->local_segment);
			scriptState.restAdjust = 0; // Used up the &rest adjustment
			scriptState.xs->sp = call_base;

			s->_executionStackPosChanged = true;
			break;
		}

		case 0x21: // callk
			gc_countdown(s);

			scriptState.xs->sp -= (opparams[1] >> 1) + 1;
			if (!((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader()) {
				scriptState.xs->sp -= scriptState.restAdjust;
				s->restAdjust = 0; // We just used up the scriptState.restAdjust, remember?
			}

			if (opparams[0] >= (int)((SciEngine*)g_engine)->getKernel()->_kernelFuncs.size()) {
				error("Invalid kernel function 0x%x requested\n", opparams[0]);
			} else {
				int argc = ASSERT_ARITHMETIC(scriptState.xs->sp[0]);

				if (!((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader())
					argc += scriptState.restAdjust;

				if (((SciEngine*)g_engine)->getKernel()->_kernelFuncs[opparams[0]].signature
						&& !kernel_matches_signature(s,
						((SciEngine*)g_engine)->getKernel()->_kernelFuncs[opparams[0]].signature, argc,
						scriptState.xs->sp + 1)) {
					error("[VM] Invalid arguments to kernel call %x\n", opparams[0]);
				} else {
					s->r_acc = ((SciEngine*)g_engine)->getKernel()->_kernelFuncs[opparams[0]].fun(s, opparams[0],
														argc, scriptState.xs->sp + 1);
				}
				// Call kernel function

				// Calculate xs again: The kernel function might
				// have spawned a new VM

				xs_new = &(s->_executionStack.back());
				s->_executionStackPosChanged = true;

				if (!((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader())
					scriptState.restAdjust = s->restAdjust;

			}
			break;

		case 0x22: // callb
			temp = ((opparams[1] >> 1) + scriptState.restAdjust + 1);
			s_temp = scriptState.xs->sp;
			scriptState.xs->sp -= temp;

			scriptState.xs->sp[0].offset += scriptState.restAdjust;
			xs_new = execute_method(s, 0, opparams[0], s_temp, scriptState.xs->objp,
									scriptState.xs->sp[0].offset, scriptState.xs->sp);
			scriptState.restAdjust = 0; // Used up the &rest adjustment
			if (xs_new)    // in case of error, keep old stack
				s->_executionStackPosChanged = true;
			break;

		case 0x23: // calle
			temp = ((opparams[2] >> 1) + scriptState.restAdjust + 1);
			s_temp = scriptState.xs->sp;
			scriptState.xs->sp -= temp;

			scriptState.xs->sp[0].offset += scriptState.restAdjust;
			xs_new = execute_method(s, opparams[0], opparams[1], s_temp, scriptState.xs->objp,
									scriptState.xs->sp[0].offset, scriptState.xs->sp);
			scriptState.restAdjust = 0; // Used up the &rest adjustment

			if (xs_new)  // in case of error, keep old stack
				s->_executionStackPosChanged = true;
			break;

		case 0x24: // ret
			do {
				StackPtr old_sp2 = scriptState.xs->sp;
				StackPtr old_fp = scriptState.xs->fp;
				ExecStack *old_xs = &(s->_executionStack.back());

				if ((int)s->_executionStack.size()-1 == s->execution_stack_base) { // Have we reached the base?
					s->execution_stack_base = old_execution_stack_base; // Restore stack base

					s->_executionStack.pop_back();

					s->_executionStackPosChanged = true;
					s->restAdjust = scriptState.restAdjust; // Update &rest
					return; // "Hard" return
				}

				if (old_xs->type == EXEC_STACK_TYPE_VARSELECTOR) {
					// varselector access?
					if (old_xs->argc) // write?
						*(old_xs->getVarPointer(s)) = old_xs->variables_argp[1];
					else // No, read
						s->r_acc = *(old_xs->getVarPointer(s));
				}

				// Not reached the base, so let's do a soft return
				s->_executionStack.pop_back();
				s->_executionStackPosChanged = true;
				scriptState.xs = &(s->_executionStack.back());

				if (scriptState.xs->sp == CALL_SP_CARRY // Used in sends to 'carry' the stack pointer
				        || scriptState.xs->type != EXEC_STACK_TYPE_CALL) {
					scriptState.xs->sp = old_sp2;
					scriptState.xs->fp = old_fp;
				}

			} while (scriptState.xs->type == EXEC_STACK_TYPE_VARSELECTOR);
			// Iterate over all varselector accesses
			s->_executionStackPosChanged = true;
			xs_new = scriptState.xs;

			break;

		case 0x25: // send
			s_temp = scriptState.xs->sp;
			scriptState.xs->sp -= ((opparams[0] >> 1) + scriptState.restAdjust); // Adjust stack

			scriptState.xs->sp[1].offset += scriptState.restAdjust;
			xs_new = send_selector(s, s->r_acc, s->r_acc, s_temp,
									(int)(opparams[0] >> 1) + (uint16)scriptState.restAdjust, scriptState.xs->sp);

			if (xs_new && xs_new != scriptState.xs)
				s->_executionStackPosChanged = true;

			scriptState.restAdjust = 0;

			break;

		case 0x28: // class
			s->r_acc = s->seg_manager->get_class_address((unsigned)opparams[0], SCRIPT_GET_LOCK,
											scriptState.xs->addr.pc);
			break;

		case 0x2a: // self
			s_temp = scriptState.xs->sp;
			scriptState.xs->sp -= ((opparams[0] >> 1) + scriptState.restAdjust); // Adjust stack

			scriptState.xs->sp[1].offset += scriptState.restAdjust;
			xs_new = send_selector(s, scriptState.xs->objp, scriptState.xs->objp,
									s_temp, (int)(opparams[0] >> 1) + (uint16)scriptState.restAdjust,
									scriptState.xs->sp);

			if (xs_new && xs_new != scriptState.xs)
				s->_executionStackPosChanged = true;

			scriptState.restAdjust = 0;
			break;

		case 0x2b: // super
			r_temp = s->seg_manager->get_class_address(opparams[0], SCRIPT_GET_LOAD, scriptState.xs->addr.pc);

			if (!r_temp.segment)
				error("[VM]: Invalid superclass in object");
			else {
				s_temp = scriptState.xs->sp;
				scriptState.xs->sp -= ((opparams[1] >> 1) + scriptState.restAdjust); // Adjust stack

				scriptState.xs->sp[1].offset += scriptState.restAdjust;
				xs_new = send_selector(s, r_temp, scriptState.xs->objp, s_temp,
										(int)(opparams[1] >> 1) + (uint16)scriptState.restAdjust,
										scriptState.xs->sp);

				if (xs_new && xs_new != scriptState.xs)
					s->_executionStackPosChanged = true;

				scriptState.restAdjust = 0;
			}

			break;

		case 0x2c: // &rest
			temp = (uint16) opparams[0]; // First argument
			scriptState.restAdjust = MAX<int16>(scriptState.xs->argc - temp + 1, 0); // +1 because temp counts the paramcount while argc doesn't

			for (; temp <= scriptState.xs->argc; temp++)
				PUSH32(scriptState.xs->variables_argp[temp]);

			break;

		case 0x2d: // lea
			temp = (uint16) opparams[0] >> 1;
			var_number = temp & 0x03; // Get variable type

			// Get variable block offset
			r_temp.segment = scriptState.variables_seg[var_number];
			r_temp.offset = scriptState.variables[var_number] - scriptState.variables_base[var_number];

			if (temp & 0x08)  // Add accumulator offset if requested
				r_temp.offset += signed_validate_arithmetic(s->r_acc);

			r_temp.offset += opparams[1];  // Add index
			r_temp.offset *= sizeof(reg_t);
			// That's the immediate address now
			s->r_acc = r_temp;
			break;


		case 0x2e: // selfID
			s->r_acc = scriptState.xs->objp;
			break;

		case 0x30: // pprev
			PUSH32(s->r_prev);
			break;

		case 0x31: // pToa
			s->r_acc = OBJ_PROPERTY(obj, (opparams[0] >> 1));
			break;

		case 0x32: // aTop
			OBJ_PROPERTY(obj, (opparams[0] >> 1)) = s->r_acc;
			break;

		case 0x33: // pTos
			PUSH32(OBJ_PROPERTY(obj, opparams[0] >> 1));
			break;

		case 0x34: // sTop
			OBJ_PROPERTY(obj, (opparams[0] >> 1)) = POP32();
			break;

		case 0x35: // ipToa
			s->r_acc = OBJ_PROPERTY(obj, (opparams[0] >> 1));
			s->r_acc = OBJ_PROPERTY(obj, (opparams[0] >> 1)) = ACC_ARITHMETIC_L(1 + /*acc*/);
			break;

		case 0x36: // dpToa
			s->r_acc = OBJ_PROPERTY(obj, (opparams[0] >> 1));
			s->r_acc = OBJ_PROPERTY(obj, (opparams[0] >> 1)) = ACC_ARITHMETIC_L(-1 + /*acc*/);
			break;

		case 0x37: // ipTos
			ASSERT_ARITHMETIC(OBJ_PROPERTY(obj, (opparams[0] >> 1)));
			temp = ++OBJ_PROPERTY(obj, (opparams[0] >> 1)).offset;
			PUSH(temp);
			break;

		case 0x38: // dpTos
			ASSERT_ARITHMETIC(OBJ_PROPERTY(obj, (opparams[0] >> 1)));
			temp = --OBJ_PROPERTY(obj, (opparams[0] >> 1)).offset;
			PUSH(temp);
			break;


		case 0x39: // lofsa
			s->r_acc.segment = scriptState.xs->addr.pc.segment;

			if (s->_version >= SCI_VERSION_1_1) {
				s->r_acc.offset = opparams[0] + local_script->script_size;
			} else {
				if (((SciEngine*)g_engine)->getKernel()->hasLofsAbsolute())
					s->r_acc.offset = opparams[0];
				else
					s->r_acc.offset = scriptState.xs->addr.pc.offset + opparams[0];
			}

#ifndef DISABLE_VALIDATIONS
			if (s->r_acc.offset >= code_buf_size) {
				error("VM: lofsa operation overflowed: %04x:%04x beyond end"
				          " of script (at %04x)\n", PRINT_REG(s->r_acc), code_buf_size);
			}
#endif
			break;

		case 0x3a: // lofss
			r_temp.segment = scriptState.xs->addr.pc.segment;

			if (s->_version >= SCI_VERSION_1_1) {
				r_temp.offset = opparams[0] + local_script->script_size;
			} else {
				if (((SciEngine*)g_engine)->getKernel()->hasLofsAbsolute())
					r_temp.offset = opparams[0];
				else
					r_temp.offset = scriptState.xs->addr.pc.offset + opparams[0];
			}

#ifndef DISABLE_VALIDATIONS
			if (r_temp.offset >= code_buf_size) {
				error("VM: lofss operation overflowed: %04x:%04x beyond end"
				          " of script (at %04x)\n", PRINT_REG(r_temp), code_buf_size);
			}
#endif
			PUSH32(r_temp);
			break;

		case 0x3b: // push0
			PUSH(0);
			break;

		case 0x3c: // push1
			PUSH(1);
			break;

		case 0x3d: // push2
			PUSH(2);
			break;

		case 0x3e: // pushSelf
			PUSH32(scriptState.xs->objp);
			break;

		case 0x40: // lag
		case 0x41: // lal
		case 0x42: // lat
		case 0x43: // lap
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			s->r_acc = READ_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x44: // lsg
		case 0x45: // lsl
		case 0x46: // lst
		case 0x47: // lsp
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			PUSH32(READ_VAR(var_type, var_number, s->r_acc));
			break;

		case 0x48: // lagi
		case 0x49: // lali
		case 0x4a: // lati
		case 0x4b: // lapi
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			s->r_acc = READ_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x4c: // lsgi
		case 0x4d: // lsli
		case 0x4e: // lsti
		case 0x4f: // lspi
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			PUSH32(READ_VAR(var_type, var_number, s->r_acc));
			break;

		case 0x50: // sag
		case 0x51: // sal
		case 0x52: // sat
		case 0x53: // sap
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			WRITE_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x54: // ssg
		case 0x55: // ssl
		case 0x56: // sst
		case 0x57: // ssp
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			WRITE_VAR(var_type, var_number, POP32());
			break;

		case 0x58: // sagi
		case 0x59: // sali
		case 0x5a: // sati
		case 0x5b: // sapi
			// Special semantics because it wouldn't really make a whole lot
			// of sense otherwise, with acc being used for two things
			// simultaneously...
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			WRITE_VAR(var_type, var_number, s->r_acc = POP32());
			break;

		case 0x5c: // ssgi
		case 0x5d: // ssli
		case 0x5e: // ssti
		case 0x5f: // sspi
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			WRITE_VAR(var_type, var_number, POP32());
			break;

		case 0x60: // +ag
		case 0x61: // +al
		case 0x62: // +at
		case 0x63: // +ap
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			s->r_acc = make_reg(0, 1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			WRITE_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x64: // +sg
		case 0x65: // +sl
		case 0x66: // +st
		case 0x67: // +sp
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			r_temp = make_reg(0, 1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			PUSH32(r_temp);
			WRITE_VAR(var_type, var_number, r_temp);
			break;

		case 0x68: // +agi
		case 0x69: // +ali
		case 0x6a: // +ati
		case 0x6b: // +api
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			s->r_acc = make_reg(0, 1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			WRITE_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x6c: // +sgi
		case 0x6d: // +sli
		case 0x6e: // +sti
		case 0x6f: // +spi
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			r_temp = make_reg(0, 1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			PUSH32(r_temp);
			WRITE_VAR(var_type, var_number, r_temp);
			break;

		case 0x70: // -ag
		case 0x71: // -al
		case 0x72: // -at
		case 0x73: // -ap
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			s->r_acc = make_reg(0, -1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			WRITE_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x74: // -sg
		case 0x75: // -sl
		case 0x76: // -st
		case 0x77: // -sp
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0];
			r_temp = make_reg(0, -1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			PUSH32(r_temp);
			WRITE_VAR(var_type, var_number, r_temp);
			break;

		case 0x78: // -agi
		case 0x79: // -ali
		case 0x7a: // -ati
		case 0x7b: // -api
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			s->r_acc = make_reg(0, -1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			WRITE_VAR(var_type, var_number, s->r_acc);
			break;

		case 0x7c: // -sgi
		case 0x7d: // -sli
		case 0x7e: // -sti
		case 0x7f: // -spi
			var_type = (opcode >> 1) & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + signed_validate_arithmetic(s->r_acc);
			r_temp = make_reg(0, -1 + validate_arithmetic(READ_VAR(var_type, var_number, s->r_acc)));
			PUSH32(r_temp);
			WRITE_VAR(var_type, var_number, r_temp);
			break;

		default:
			error("run_vm(): illegal opcode %x", opnumber);

		} // switch(opcode >> 1)

		if (s->_executionStackPosChanged) // Force initialization
			scriptState.xs = xs_new;

//#ifndef DISABLE_VALIDATIONS
		if (scriptState.xs != &(s->_executionStack.back())) {
			warning("xs is stale (%p vs %p); last command was %02x",
					(void *)scriptState.xs, (void *)&(s->_executionStack.back()),
					opnumber);
		}
//#endif
		++script_step_counter;

		s->speedThrottler->postInstruction();
	}
}

static int _obj_locate_varselector(EngineState *s, Object *obj, Selector slc) {
	// Determines if obj explicitly defines slc as a varselector
	// Returns -1 if not found
	SciVersion version = s->_version;	// for the selector defines

	if (s->_version < SCI_VERSION_1_1) {
		int varnum = obj->variable_names_nr;
		int selector_name_offset = varnum * 2 + SCRIPT_SELECTOR_OFFSET;
		int i;
		byte *buf = obj->base_obj + selector_name_offset;

		obj->base_vars = (uint16 *) buf;

		for (i = 0; i < varnum; i++)
			if (READ_LE_UINT16(buf + (i << 1)) == slc) // Found it?
				return i; // report success

		return -1; // Failed
	} else {
		byte *buf = (byte *) obj->base_vars;
		int i;
		int varnum = obj->_variables[1].offset;

		if (!(obj->_variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS))
			buf = ((byte *) obj_get(s->seg_manager, s->_version, obj->_variables[SCRIPT_SUPERCLASS_SELECTOR])->base_vars);

		for (i = 0; i < varnum; i++)
			if (READ_LE_UINT16(buf + (i << 1)) == slc) // Found it?
				return i; // report success

		return -1; // Failed
	}
}

static int _class_locate_funcselector(EngineState *s, Object *obj, Selector slc) {
	// Determines if obj is a class and explicitly defines slc as a funcselector
	// Does NOT say anything about obj's superclasses, i.e. failure may be
	// returned even if one of the superclasses defines the funcselector.
	int funcnum = obj->methods_nr;
	int i;

	for (i = 0; i < funcnum; i++)
		if (VM_OBJECT_GET_FUNCSELECTOR(obj, i) == slc) // Found it?
			return i; // report success

	return -1; // Failed
}

static SelectorType _lookup_selector_function(EngineState *s, int seg_id, Object *obj, Selector selector_id, reg_t *fptr) {
	int index;
	SciVersion version = s->_version;	// for the selector defines

	// "recursive" lookup

	while (obj) {
		index = _class_locate_funcselector(s, obj, selector_id);

		if (index >= 0) {
			if (fptr) {
				*fptr = VM_OBJECT_READ_FUNCTION(obj, index);
			}

			return kSelectorMethod;
		} else {
			seg_id = obj->_variables[SCRIPT_SUPERCLASS_SELECTOR].segment;
			obj = obj_get(s->seg_manager, s->_version, obj->_variables[SCRIPT_SUPERCLASS_SELECTOR]);
		}
	}

	return kSelectorNone;
}

SelectorType lookup_selector(EngineState *s, reg_t obj_location, Selector selector_id, ObjVarRef *varp, reg_t *fptr) {
	Object *obj = obj_get(s->seg_manager, s->_version, obj_location);
	Object *species;
	int index;
	SciVersion version = s->_version;	// for the selector defines

	// Early SCI versions used the LSB in the selector ID as a read/write
	// toggle, meaning that we must remove it for selector lookup.
	if (((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader())
		selector_id &= ~1;

	if (!obj) {
		error("lookup_selector(): Attempt to send to non-object or invalid script. Address was %04x:%04x", 
				PRINT_REG(obj_location));
	}

	if (IS_CLASS(obj))
		species = obj;
	else
		species = obj_get(s->seg_manager, s->_version, obj->_variables[SCRIPT_SPECIES_SELECTOR]);


	if (!obj) {
		error("lookup_selector(): Error while looking up Species class.\nOriginal address was %04x:%04x. Species address was %04x:%04x\n", 
			PRINT_REG(obj_location), PRINT_REG(obj->_variables[SCRIPT_SPECIES_SELECTOR]));
		return kSelectorNone;
	}

	index = _obj_locate_varselector(s, obj, selector_id);

	if (index >= 0) {
		// Found it as a variable
		if (varp) {
			varp->obj = obj_location;
			varp->varindex = index;
		}
		return kSelectorVariable;
	}

	return _lookup_selector_function(s, obj_location.segment, obj, selector_id, fptr);
}

reg_t script_lookup_export(SegManager *segManager, int script_nr, int export_index) {
	SegmentId seg = segManager->getSegment(script_nr, SCRIPT_GET_DONT_LOAD);
	Script *script = segManager->getScriptIfLoaded(seg);
	return make_reg(seg, READ_LE_UINT16((byte *)(script->export_table + export_index)));
}

#define INST_LOOKUP_CLASS(id) ((id == 0xffff)? NULL_REG : segManager->get_class_address(id, SCRIPT_GET_LOCK, reg))

int script_instantiate_common(ResourceManager *resMgr, SegManager *segManager, SciVersion version, int script_nr, Resource **script, Resource **heap, int *was_new) {
	int seg_id;
	reg_t reg;

	*was_new = 1;

	*script = resMgr->findResource(ResourceId(kResourceTypeScript, script_nr), 0);
	if (version >= SCI_VERSION_1_1)
		*heap = resMgr->findResource(ResourceId(kResourceTypeHeap, script_nr), 0);

	if (!*script || (version >= SCI_VERSION_1_1 && !heap)) {
		warning("Script 0x%x requested but not found", script_nr);
		if (version >= SCI_VERSION_1_1) {
			if (*heap)
				warning("Inconsistency: heap resource WAS found");
			else if (*script)
				warning("Inconsistency: script resource WAS found");
		}
		return 0;
	}

	seg_id = segManager->segGet(script_nr);
	Script *scr = segManager->getScriptIfLoaded(seg_id);
	if (scr) {
		if (!scr->isMarkedAsDeleted()) {
			scr->incrementLockers();
			return seg_id;
		} else {
			scr->freeScript();
		}
	} else {
		scr = segManager->allocateScript(script_nr, &seg_id);
		if (!scr) {  // ALL YOUR SCRIPT BASE ARE BELONG TO US
			error("Not enough heap space for script size 0x%x of script 0x%x (Should this happen?)", (*script)->size, script_nr);
			return 0;
		}
	}

	segManager->initialiseScript(*scr, script_nr);

	reg.segment = seg_id;
	reg.offset = 0;

	// Set heap position (beyond the size word)
	scr->setLockers(1);
	scr->setExportTableOffset(0);
	scr->setSynonymsOffset(0);
	scr->setSynonymsNr(0);

	*was_new = 0;

	return seg_id;
}

int script_instantiate_sci0(ResourceManager *resMgr, SegManager *segManager, SciVersion version, bool oldScriptHeader, int script_nr) {
	int objtype;
	unsigned int objlength;
	reg_t reg;
	int seg_id;
	int relocation = -1;
	int magic_pos_adder; // Usually 0; 2 for older SCI versions
	Resource *script;
	int was_new;

	seg_id = script_instantiate_common(resMgr, segManager, version, script_nr, &script, NULL, &was_new);

	if (was_new)
		return seg_id;

	reg.segment = seg_id;
	reg.offset = 0;

	Script *scr = segManager->getScript(seg_id);

	if (oldScriptHeader) {
		//
		int locals_nr = READ_LE_UINT16(script->data);

		// Old script block
		// There won't be a localvar block in this case
		// Instead, the script starts with a 16 bit int specifying the
		// number of locals we need; these are then allocated and zeroed.

		scr->mcpyInOut(0, script->data, script->size);
		magic_pos_adder = 2;  // Step over the funny prefix

		if (locals_nr)
			segManager->scriptInitialiseLocalsZero(reg.segment, locals_nr);

	} else {
		scr->mcpyInOut(0, script->data, script->size);
		magic_pos_adder = 0;
	}

	// Now do a first pass through the script objects to find the
	// export table and local variable block

	objlength = 0;
	reg.offset = magic_pos_adder;

	do {
		reg_t data_base;
		reg_t addr;
		reg.offset += objlength; // Step over the last checked object
		objtype = scr->getHeap(reg.offset);
		if (!objtype) break;

		objlength = scr->getHeap(reg.offset + 2);

		data_base = reg;
		data_base.offset += 4;

		addr = data_base;

		switch (objtype) {
		case SCI_OBJ_EXPORTS: {
			scr->setExportTableOffset(data_base.offset);
		}
		break;

		case SCI_OBJ_SYNONYMS:
			scr->setSynonymsOffset(addr.offset);   // +4 is to step over the header
			scr->setSynonymsNr((objlength) / 4);
			break;

		case SCI_OBJ_LOCALVARS:
			segManager->scriptInitialiseLocals(data_base);
			break;

		case SCI_OBJ_CLASS: {
			int classpos = addr.offset - SCRIPT_OBJECT_MAGIC_OFFSET;
			int species;
			species = scr->getHeap(addr.offset - SCRIPT_OBJECT_MAGIC_OFFSET + SCRIPT_SPECIES_OFFSET);
			if (species < 0 || species >= (int)segManager->_classtable.size()) {
				error("Invalid species %d(0x%x) not in interval "
				          "[0,%d) while instantiating script %d\n",
				          species, species, segManager->_classtable.size(),
				          script_nr);
				return 1;
			}

			segManager->_classtable[species].script = script_nr;
			segManager->_classtable[species].reg = addr;
			segManager->_classtable[species].reg.offset = classpos;
			// Set technical class position-- into the block allocated for it
		}
		break;

		default:
			break;
		}
	} while (objtype != 0);
	// And now a second pass to adjust objects and class pointers, and the general pointers

	objlength = 0;
	reg.offset = magic_pos_adder; // Reset counter

	do {
		reg_t addr;
		reg.offset += objlength; // Step over the last checked object
		objtype = scr->getHeap(reg.offset);
		if (!objtype) break;
		objlength = scr->getHeap(reg.offset + 2);
		reg.offset += 4; // Step over header

		addr = reg;

		switch (objtype) {
		case SCI_OBJ_CODE:
			segManager->scriptAddCodeBlock(addr);
			break;
		case SCI_OBJ_OBJECT:
		case SCI_OBJ_CLASS: { // object or class?
			Object *obj = segManager->scriptObjInit(addr);
			Object *base_obj;

			// Instantiate the superclass, if neccessary
			obj->_variables[SCRIPT_SPECIES_SELECTOR] = INST_LOOKUP_CLASS(obj->_variables[SCRIPT_SPECIES_SELECTOR].offset);

			base_obj = obj_get(segManager, version, obj->_variables[SCRIPT_SPECIES_SELECTOR]);
			obj->variable_names_nr = base_obj->_variables.size();
			obj->base_obj = base_obj->base_obj;
			// Copy base from species class, as we need its selector IDs

			obj->_variables[SCRIPT_SUPERCLASS_SELECTOR] = INST_LOOKUP_CLASS(obj->_variables[SCRIPT_SUPERCLASS_SELECTOR].offset);
		} // if object or class
		break;
		case SCI_OBJ_POINTERS: // A relocation table
			relocation = addr.offset;
			break;

		default:
			break;
		}

		reg.offset -= 4; // Step back on header

	} while ((objtype != 0) && (((unsigned)reg.offset) < script->size - 2));

	if (relocation >= 0)
		segManager->scriptRelocate(make_reg(reg.segment, relocation));

	return reg.segment;		// instantiation successful
}

int script_instantiate_sci11(ResourceManager *resMgr, SegManager *segManager, SciVersion version, int script_nr) {
	Resource *script, *heap;
	int seg_id;
	int heap_start;
	reg_t reg;
	int was_new;

	seg_id = script_instantiate_common(resMgr, segManager, version, script_nr, &script, &heap, &was_new);

	if (was_new)
		return seg_id;

	Script *scr = segManager->getScript(seg_id);

	heap_start = script->size;
	if (script->size & 2)
		heap_start ++;

	scr->mcpyInOut(0, script->data, script->size);
	scr->mcpyInOut(heap_start, heap->data, heap->size);

	if (READ_LE_UINT16(script->data + 6) > 0)
		scr->setExportTableOffset(6);

	reg.segment = seg_id;
	reg.offset = heap_start + 4;
	segManager->scriptInitialiseLocals(reg);

	segManager->scriptRelocateExportsSci11(seg_id);
	segManager->scriptInitialiseObjectsSci11(seg_id);

	reg.offset = READ_LE_UINT16(heap->data);
	segManager->heapRelocate(reg);

	return seg_id;
}

int script_instantiate(ResourceManager *resMgr, SegManager *segManager, SciVersion version, bool oldScriptHeader, int script_nr) {
	if (version >= SCI_VERSION_1_1)
		return script_instantiate_sci11(resMgr, segManager, version, script_nr);
	else
		return script_instantiate_sci0(resMgr, segManager, version, oldScriptHeader, script_nr);
}

void script_uninstantiate_sci0(SegManager *segManager, SciVersion version, int script_nr, SegmentId seg) {
	reg_t reg = make_reg(seg, ((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader() ? 2 : 0);
	int objtype, objlength;
	Script *scr = segManager->getScript(seg);

	// Make a pass over the object in order uninstantiate all superclasses
	objlength = 0;

	do {
		reg.offset += objlength; // Step over the last checked object

		objtype = scr->getHeap(reg.offset);
		if (!objtype)
			break;
		objlength = scr->getHeap(reg.offset + 2);  // use SEG_UGET_HEAP ??

		reg.offset += 4; // Step over header

		if ((objtype == SCI_OBJ_OBJECT) || (objtype == SCI_OBJ_CLASS)) { // object or class?
			int superclass;

			reg.offset -= SCRIPT_OBJECT_MAGIC_OFFSET;

			superclass = scr->getHeap(reg.offset + SCRIPT_SUPERCLASS_OFFSET); // Get superclass...

			if (superclass >= 0) {
				int superclass_script = segManager->_classtable[superclass].script;

				if (superclass_script == script_nr) {
					if (scr->getLockers())
						scr->decrementLockers();  // Decrease lockers if this is us ourselves
				} else
					script_uninstantiate(segManager, version, superclass_script);
				// Recurse to assure that the superclass lockers number gets decreased
			}

			reg.offset += SCRIPT_OBJECT_MAGIC_OFFSET;
		} // if object or class

		reg.offset -= 4; // Step back on header

	} while (objtype != 0);
}

void script_uninstantiate(SegManager *segManager, SciVersion version, int script_nr) {
	SegmentId segment = segManager->segGet(script_nr);
	Script *scr = segManager->getScriptIfLoaded(segment);

	if (!scr) {   // Is it already loaded?
		//warning("unloading script 0x%x requested although not loaded", script_nr);
		// This is perfectly valid SCI behaviour
		return;
	}

	scr->decrementLockers();   // One less locker

	if (scr->getLockers() > 0)
		return;

	// Free all classtable references to this script
	for (uint i = 0; i < segManager->_classtable.size(); i++)
		if (segManager->_classtable[i].reg.segment == segment)
			segManager->_classtable[i].reg = NULL_REG;

	if (version < SCI_VERSION_1_1)
		script_uninstantiate_sci0(segManager, version, script_nr, segment);
	else
		warning("FIXME: Add proper script uninstantiation for SCI 1.1");

	if (scr->getLockers())
		return; // if xxx.lockers > 0

	// Otherwise unload it completely
	// Explanation: I'm starting to believe that this work is done by SCI itself.
	scr->markDeleted();

	debugC(kDebugLevelScripts, "Unloaded script 0x%x.\n", script_nr);

	return;
}

static void _init_stack_base_with_selector(EngineState *s, Selector selector) {
	s->stack_base[0] = make_reg(0, (uint16)selector);
	s->stack_base[1] = NULL_REG;
}

static EngineState *_game_run(EngineState *&s, int restoring) {
	EngineState *successor = NULL;
	int game_is_finished = 0;
	do {
		s->_executionStackPosChanged = false;
		run_vm(s, (successor || restoring) ? 1 : 0);
		if (s->restarting_flags & SCI_GAME_IS_RESTARTING_NOW) { // Restart was requested?
			successor = NULL;
			s->_executionStack.clear();
			s->_executionStackPosChanged = false;

			game_exit(s);
			script_free_engine(s);
			script_init_engine(s);
			game_init(s);
			sfx_reset_player();
			_init_stack_base_with_selector(s, ((SciEngine*)g_engine)->getKernel()->_selectorMap.play);

			send_selector(s, s->game_obj, s->game_obj, s->stack_base, 2, s->stack_base);

			script_abort_flag = 0;
			s->restarting_flags = SCI_GAME_WAS_RESTARTED | SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;

		} else {
			successor = s->successor;
			if (successor) {
				game_exit(s);
				script_free_vm_memory(s);
				delete s;
				s = successor;

				if (script_abort_flag == 2) {
					debugC(2, kDebugLevelVM, "Restarting with replay()\n");
					s->_executionStack.clear(); // Restart with replay

					_init_stack_base_with_selector(s, ((SciEngine*)g_engine)->getKernel()->_selectorMap.replay);

					send_selector(s, s->game_obj, s->game_obj, s->stack_base, 2, s->stack_base);
				}

				script_abort_flag = 0;

			} else
				game_is_finished = 1;
		}
	} while (!game_is_finished);

	return s;
}

int game_run(EngineState **_s) {
	EngineState *s = *_s;

	debugC(2, kDebugLevelVM, "Calling %s::play()\n", s->_gameName.c_str());
	_init_stack_base_with_selector(s, ((SciEngine*)g_engine)->getKernel()->_selectorMap.play); // Call the play selector

	// Now: Register the first element on the execution stack-
	if (!send_selector(s, s->game_obj, s->game_obj, s->stack_base, 2, s->stack_base)) {
		Console *con = ((SciEngine *)g_engine)->getSciDebugger();
		con->printObject(s->game_obj);
		warning("Failed to run the game! Aborting...");
		return 1;
	}
	// and ENGAGE!
	_game_run(*_s, 0);

	debugC(2, kDebugLevelVM, "Game::play() finished.\n");

	return 0;
}

Object *obj_get(SegManager *segManager, SciVersion version, reg_t offset) {
	MemObject *mobj = GET_OBJECT_SEGMENT(*segManager, offset.segment);
	Object *obj = NULL;
	int idx;

	if (mobj != NULL) {
		if (mobj->getType() == MEM_OBJ_CLONES) {
			CloneTable *ct = (CloneTable *)mobj;
			if (ct->isValidEntry(offset.offset))
				obj = &(ct->_table[offset.offset]);
		} else if (mobj->getType() == MEM_OBJ_SCRIPT) {
			Script *scr = (Script *)mobj;
			if (offset.offset <= scr->buf_size && offset.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET
			        && RAW_IS_OBJECT(scr->buf + offset.offset)) {
				idx = RAW_GET_CLASS_INDEX(scr, offset);
				if (idx >= 0 && (uint)idx < scr->_objects.size())
					obj = &scr->_objects[idx];
			}
		}
	}

	return obj;
}

const char *obj_get_name(SegManager *segManager, SciVersion version, reg_t pos) {
	Object *obj = obj_get(segManager, version, pos);
	if (!obj)
		return "<no such object>";

	reg_t nameReg = obj->_variables[SCRIPT_NAME_SELECTOR];
	if (nameReg.isNull())
		return "<no name>";

	const char *name = (const char*)segManager->dereference(obj->_variables[SCRIPT_NAME_SELECTOR], NULL);
	if (!name)
		return "<invalid name>";

	return name;
}


void quit_vm() {
	script_abort_flag = 1; // Terminate VM
	scriptState.seeking = kDebugSeekNothing;
	scriptState.runningStep = 0;
}

void shrink_execution_stack(EngineState *s, uint size) {
	assert(s->_executionStack.size() >= size);
	Common::List<ExecStack>::iterator iter;
	iter = s->_executionStack.begin();
	for (uint i = 0; i < size; ++i)
		++iter;
	s->_executionStack.erase(iter, s->_executionStack.end());
}

reg_t* ObjVarRef::getPointer(EngineState *s) const {
	Object *o = obj_get(s->seg_manager, s->_version, obj);
	if (!o) return 0;
	return &(o->_variables[varindex]);
}

reg_t* ExecStack::getVarPointer(EngineState *s) const {
	assert(type == EXEC_STACK_TYPE_VARSELECTOR);
	return addr.varp.getPointer(s);
}

} // End of namespace Sci
