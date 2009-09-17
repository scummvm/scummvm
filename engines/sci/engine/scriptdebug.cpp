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

// Script debugger functionality. Absolutely not threadsafe.

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/engine/state.h"

namespace Sci {

const char *opcodeNames[] = {
	   "bnot",       "add",      "sub",      "mul",      "div",
		"mod",       "shr",      "shl",      "xor",      "and",
		 "or",       "neg",      "not",       "eq",       "ne",
		"gt_",       "ge_",      "lt_",      "le_",     "ugt_",
	   "uge_",      "ult_",     "ule_",       "bt",      "bnt",
		"jmp",       "ldi",     "push",    "pushi",     "toss",
		"dup",      "link",     "call",    "callk",    "callb",
	  "calle",       "ret",     "send",    "dummy",    "dummy",
	  "class",     "dummy",     "self",    "super",     "rest",
		"lea",    "selfID",    "dummy",    "pprev",     "pToa",
	   "aTop",      "pTos",     "sTop",    "ipToa",    "dpToa",
	  "ipTos",     "dpTos",    "lofsa",    "lofss",    "push0",
	  "push1",     "push2", "pushSelf",    "dummy",      "lag",
		"lal",       "lat",      "lap",     "lagi",     "lali",
	   "lati",      "lapi",      "lsg",      "lsl",      "lst",
		"lsp",      "lsgi",     "lsli",     "lsti",     "lspi",
		"sag",       "sal",      "sat",      "sap",     "sagi",
	   "sali",      "sati",     "sapi",      "ssg",      "ssl",
		"sst",       "ssp",     "ssgi",     "ssli",     "ssti",
	   "sspi",    "plusag",   "plusal",   "plusat",   "plusap",
	"plusagi",   "plusali",  "plusati",  "plusapi",   "plussg",
	 "plussl",    "plusst",   "plussp",  "plussgi",  "plussli",
	"plussti",   "plusspi",  "minusag",  "minusal",  "minusat",
	"minusap",  "minusagi", "minusali", "minusati", "minusapi",
	"minussg",   "minussl",  "minusst",  "minussp", "minussgi",
	"minussli", "minussti", "minusspi"
};

extern const char *selector_name(EngineState *s, int selector);

DebugState g_debugState;

int propertyOffsetToId(SegManager *segMan, int prop_ofs, reg_t objp) {
	Object *obj = segMan->getObject(objp);
	byte *selectoroffset;
	int selectors;
	SciVersion version = segMan->sciVersion();	// for the selector defines

	if (!obj) {
		warning("Applied propertyOffsetToId on non-object at %04x:%04x", PRINT_REG(objp));
		return -1;
	}

	selectors = obj->_variables.size();

	if (segMan->sciVersion() < SCI_VERSION_1_1)
		selectoroffset = ((byte *)(obj->base_obj)) + SCRIPT_SELECTOR_OFFSET + selectors * 2;
	else {
		if (!(obj->getInfoSelector(version).offset & SCRIPT_INFO_CLASS)) {
			obj = segMan->getObject(obj->getSuperClassSelector(version));
			selectoroffset = (byte *)obj->base_vars;
		} else
			selectoroffset = (byte *)obj->base_vars;
	}

	if (prop_ofs < 0 || (prop_ofs >> 1) >= selectors) {
		warning("Applied propertyOffsetToId to invalid property offset %x (property #%d not in [0..%d]) on object at %04x:%04x",
		          prop_ofs, prop_ofs >> 1, selectors - 1, PRINT_REG(objp));
		return -1;
	}

	return READ_LE_UINT16(selectoroffset + prop_ofs);
}

// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
reg_t disassemble(EngineState *s, reg_t pos, int print_bw_tag, int print_bytecode) {
	SegmentObj *mobj = GET_SEGMENT(*s->segMan, pos.segment, SEG_TYPE_SCRIPT);
	Script *script_entity = NULL;
	byte *scr;
	int scr_size;
	reg_t retval = make_reg(pos.segment, pos.offset + 1);
	uint16 param_value;
	int opsize;
	uint opcode;
	int bytecount = 1;
	int i = 0;
	Kernel *kernel = ((SciEngine*)g_engine)->getKernel();

	if (!mobj) {
		warning("Disassembly failed: Segment %04x non-existant or not a script", pos.segment);
		return retval;
	} else
		script_entity = (Script *)mobj;

	scr = script_entity->_buf;
	scr_size = script_entity->_bufSize;

	if (pos.offset >= scr_size) {
		warning("Trying to disassemble beyond end of script");
		return pos;
	}

	opsize = scr[pos.offset];
	opcode = opsize >> 1;

	opsize &= 1; // byte if true, word if false

	printf("%04x:%04x: ", PRINT_REG(pos));

	if (print_bytecode) {
		while (g_opcode_formats[opcode][i]) {
			switch (g_opcode_formats[opcode][i++]) {

			case Script_SByte:
			case Script_Byte:
				bytecount++;
				break;

			case Script_Word:
			case Script_SWord:
				bytecount += 2;
				break;

			case Script_SVariable:
			case Script_Variable:
			case Script_Property:
			case Script_Global:
			case Script_Local:
			case Script_Temp:
			case Script_Param:
			case Script_SRelative:
				if (opsize)
					bytecount ++;
				else
					bytecount += 2;
				break;

			default:
				break;
			}
		}

		if (pos.offset + bytecount > scr_size) {
			warning("Operation arguments extend beyond end of script");
			return retval;
		}

		for (i = 0; i < bytecount; i++)
			printf("%02x ", scr[pos.offset + i]);

		for (i = bytecount; i < 5; i++)
			printf("   ");
	}

	if (print_bw_tag)
		printf("[%c] ", opsize ? 'B' : 'W');

	printf("%s", opcodeNames[opcode]);

	i = 0;
	while (g_opcode_formats[opcode][i]) {
		switch (g_opcode_formats[opcode][i++]) {
		case Script_Invalid:
			warning("-Invalid operation-");
			break;

		case Script_SByte:
		case Script_Byte:
			printf(" %02x", scr[retval.offset++]);
			break;

		case Script_Word:
		case Script_SWord:
			printf(" %04x", 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8)));
			retval.offset += 2;
			break;

		case Script_SVariable:
		case Script_Variable:
		case Script_Property:
		case Script_Global:
		case Script_Local:
		case Script_Temp:
		case Script_Param:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}

			if (opcode == op_callk)
				printf(" %s[%x]", (param_value < kernel->_kernelFuncs.size()) ?
							((param_value < kernel->getKernelNamesSize()) ? kernel->getKernelName(param_value).c_str() : "[Unknown(postulated)]")
							: "<invalid>", param_value);
			else
				printf(opsize ? " %02x" : " %04x", param_value);

			break;

		case Script_Offset:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}
			printf(opsize ? " %02x" : " %04x", param_value);
			break;

		case Script_SRelative:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}
			printf(opsize ? " %02x  [%04x]" : " %04x  [%04x]", param_value, (0xffff) & (retval.offset + param_value));
			break;

		case Script_End:
			retval = NULL_REG;
			break;

		default:
			error("Internal assertion failed in disassemble()");

		}
	}

	if (pos == scriptState.xs->addr.pc) { // Extra information if debugging the current opcode
		if ((opcode == op_pTos) || (opcode == op_sTop) || (opcode == op_pToa) || (opcode == op_aTop) ||
		        (opcode == op_dpToa) || (opcode == op_ipToa) || (opcode == op_dpTos) || (opcode == op_ipTos)) {
			int prop_ofs = scr[pos.offset + 1];
			int prop_id = propertyOffsetToId(s->segMan, prop_ofs, scriptState.xs->objp);

			printf("	(%s)", selector_name(s, prop_id));
		}
	}

	printf("\n");

	if (pos == scriptState.xs->addr.pc) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.offset + 2] >> 1) + (scriptState.restAdjust);
			int argc = ((scriptState.xs->sp)[- stackframe - 1]).offset;
			bool oldScriptHeader = (s->segMan->sciVersion() == SCI_VERSION_0_EARLY);

			if (!oldScriptHeader)
				argc += (scriptState.restAdjust);

			printf(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				printf("%04x:%04x", PRINT_REG((scriptState.xs->sp)[j - stackframe]));
				if (j + 1 < argc)
					printf(", ");
			}
			printf(")\n");
		} else if ((opcode == op_send) || (opcode == op_self)) {
			int restmod = scriptState.restAdjust;
			int stackframe = (scr[pos.offset + 1] >> 1) + restmod;
			reg_t *sb = scriptState.xs->sp;
			uint16 selector;
			reg_t fun_ref;

			while (stackframe > 0) {
				int argc = sb[- stackframe + 1].offset;
				const char *name = NULL;
				reg_t called_obj_addr = scriptState.xs->objp;

				if (opcode == op_send)
					called_obj_addr = s->r_acc;
				else if (opcode == op_self)
					called_obj_addr = scriptState.xs->objp;

				selector = sb[- stackframe].offset;

				name = s->segMan->getObjectName(called_obj_addr);

				if (!name)
					name = "<invalid>";

				printf("  %s::%s[", name, (selector > kernel->getSelectorNamesSize()) ? "<invalid>" : selector_name(s, selector));

				switch (lookup_selector(s->segMan, called_obj_addr, selector, 0, &fun_ref)) {
				case kSelectorMethod:
					printf("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case kSelectorVariable:
					printf("VAR");
					break;
				case kSelectorNone:
					printf("INVALID");
					break;
				}

				printf("](");

				while (argc--) {
					printf("%04x:%04x", PRINT_REG(sb[- stackframe + 2]));
					if (argc)
						printf(", ");
					stackframe--;
				}

				printf(")\n");
				stackframe -= 2;
			} // while (stackframe > 0)
		} // Send-like opcodes
	} // (heappos == *p_pc)

	return retval;
}


void script_debug(EngineState *s, bool bp) {
	// Do we support a separate console?

#if 0
	if (sci_debug_flags & _DEBUG_FLAG_LOGGING) {
		printf("%d: acc=%04x:%04x  ", script_step_counter, PRINT_REG(s->r_acc));
		disassemble(s, scriptState.xs->addr.pc, 0, 1);
		if (scriptState.seeking == kDebugSeekGlobal)
			printf("Global %d (0x%x) = %04x:%04x\n", scriptState.seekSpecial,
			          scriptState.seekSpecial, PRINT_REG(s->script_000->_localsBlock->_locals[scriptState.seekSpecial]));
	}
#endif

#if 0
	if (!g_debugState.debugging)
		return;
#endif

	if (g_debugState.seeking && !bp) { // Are we looking for something special?
		SegmentObj *mobj = GET_SEGMENT(*s->segMan, scriptState.xs->addr.pc.segment, SEG_TYPE_SCRIPT);

		if (mobj) {
			Script *scr = (Script *)mobj;
			byte *code_buf = scr->_buf;
			int code_buf_size = scr->_bufSize;
			int opcode = scriptState.xs->addr.pc.offset >= code_buf_size ? 0 : code_buf[scriptState.xs->addr.pc.offset];
			int op = opcode >> 1;
			int paramb1 = scriptState.xs->addr.pc.offset + 1 >= code_buf_size ? 0 : code_buf[scriptState.xs->addr.pc.offset + 1];
			int paramf1 = (opcode & 1) ? paramb1 : (scriptState.xs->addr.pc.offset + 2 >= code_buf_size ? 0 : (int16)READ_LE_UINT16(code_buf + scriptState.xs->addr.pc.offset + 1));

			switch (g_debugState.seeking) {
			case kDebugSeekSpecialCallk:
				if (paramb1 != g_debugState.seekSpecial)
					return;

			case kDebugSeekCallk: {
				if (op != op_callk)
					return;
				break;
			}

			case kDebugSeekLevelRet: {
				if ((op != op_ret) || (g_debugState.seekLevel < (int)s->_executionStack.size()-1))
					return;
				break;
			}

			case kDebugSeekGlobal:
				if (op < op_sag)
					return;
				if ((op & 0x3) > 1)
					return; // param or temp
				if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
					return; // locals and not running in script.000
				if (paramf1 != g_debugState.seekSpecial)
					return; // CORRECT global?
				break;

			case kDebugSeekSO:
				// FIXME: Unhandled?
				break;

			case kDebugSeekNothing:
				// We seek nothing, so just continue
				break;
			}

			g_debugState.seeking = kDebugSeekNothing;
			// OK, found whatever we were looking for
		}
	}

	printf("Step #%d\n", script_step_counter);
	disassemble(s, scriptState.xs->addr.pc, 0, 1);

	if (g_debugState.runningStep) {
		g_debugState.runningStep--;
		return;
	}

	g_debugState.debugging = false;

	Console *con = ((Sci::SciEngine*)g_engine)->getSciDebugger();
	con->attach();
}

} // End of namespace Sci
