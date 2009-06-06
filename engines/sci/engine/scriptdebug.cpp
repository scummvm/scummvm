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
#include "sci/debug.h"
#include "sci/engine/state.h"
#include "sci/engine/gc.h"
#include "sci/engine/kernel_types.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_gui.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxContainer, GfxPort, GfxVisual
#include "sci/resource.h"
#include "sci/vocabulary.h"
#include "sci/sfx/iterator.h"
#include "sci/sfx/sci_midi.h"

#include "common/util.h"
#include "common/savefile.h"

#include "sound/audiostream.h"

namespace Sci {

int g_debugstate_valid = 0; // Set to 1 while script_debug is running
int g_debug_step_running = 0; // Set to >0 to allow multiple stepping
static bool s_debug_commands_hooked = false; // Commands hooked to the console yet?
int g_debug_seeking = 0; // Stepping forward until some special condition is met
static int s_debug_seek_level = 0; // Used for seekers that want to check their exec stack depth
static int s_debug_seek_special = 0;  // Used for special seeks(1)

#define _DEBUG_SEEK_NOTHING 0
#define _DEBUG_SEEK_CALLK 1 // Step forward until callk is found
#define _DEBUG_SEEK_LEVEL_RET 2 // Step forward until returned from this level
#define _DEBUG_SEEK_SPECIAL_CALLK 3 // Step forward until a /special/ callk is found
#define _DEBUG_SEEK_SO 5 // Step forward until specified PC (after the send command) and stack depth
#define _DEBUG_SEEK_GLOBAL 6 // Step forward until one specified global variable is modified

static reg_t *p_pc;
static StackPtr *p_sp;
static StackPtr *p_pp;
static reg_t *p_objp;
static int *p_restadjust;
static SegmentId *p_var_segs;
static reg_t **p_vars;
static reg_t **p_var_base;
static int *p_var_max; // May be NULL even in valid state!

char inputbuf[256] = "";

union cmd_param_t {
	int32 val;
	const char *str;
	reg_t reg;
};

typedef int (*ConCommand)(EngineState *s, const Common::Array<cmd_param_t> &cmdParams);

struct cmd_mm_entry_t {
	const char *name;
	const char *description;
}; // All later structures must "extend" this

struct cmd_command_t : public cmd_mm_entry_t {
	ConCommand command;
	const char *param;
};

// Dummy function, so that it compiles
int con_hook_command(ConCommand command, const char *name, const char *param, const char *description) {

	return 0;
}

int c_step(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	g_debugstate_valid = 0;
	if (cmdParams.size() && (cmdParams[0].val > 0))
		g_debug_step_running = cmdParams[0].val - 1;

	return 0;
}

enum {
	_parse_eoi,
	_parse_token_pareno,
	_parse_token_parenc,
	_parse_token_nil,
	_parse_token_number
};

int _parse_getinp(int *i, int *nr, const Common::Array<cmd_param_t> &cmdParams) {
	const char *token;

	if ((unsigned)*i == cmdParams.size())
		return _parse_eoi;

	token = cmdParams[(*i)++].str;

	if (!strcmp(token, "("))
		return _parse_token_pareno;

	if (!strcmp(token, ")"))
		return _parse_token_parenc;

	if (!strcmp(token, "nil"))
		return _parse_token_nil;

	*nr = strtol(token, NULL, 0);

	return _parse_token_number;
}

int _parse_nodes(EngineState *s, int *i, int *pos, int type, int nr, const Common::Array<cmd_param_t> &cmdParams) {
	int nexttk, nextval, newpos, oldpos;

	if (type == _parse_token_nil)
		return 0;

	if (type == _parse_token_number) {
		s->parser_nodes[*pos += 1].type = PARSE_TREE_NODE_LEAF;
		s->parser_nodes[*pos].content.value = nr;
		return *pos;
	}
	if (type == _parse_eoi) {
		sciprintf("Unbalanced parentheses\n");
		return -1;
	}
	if (type == _parse_token_parenc) {
		sciprintf("Syntax error at token %d\n", *i);
		return -1;
	}
	s->parser_nodes[oldpos = ++(*pos)].type = PARSE_TREE_NODE_BRANCH;

	nexttk = _parse_getinp(i, &nextval, cmdParams);
	if ((newpos = s->parser_nodes[oldpos].content.branches[0] = _parse_nodes(s, i, pos, nexttk, nextval, cmdParams)) == -1)
		return -1;

	nexttk = _parse_getinp(i, &nextval, cmdParams);
	if ((newpos = s->parser_nodes[oldpos].content.branches[1] = _parse_nodes(s, i, pos, nexttk, nextval, cmdParams)) == -1)
		return -1;

	if (_parse_getinp(i, &nextval, cmdParams) != _parse_token_parenc)
		sciprintf("Expected ')' at token %d\n", *i);

	return oldpos;
}

int c_set_parse_nodes(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	int i = 0;
	int foo, bar;
	int pos = -1;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	bar = _parse_getinp(&i, &foo, cmdParams);
	if (_parse_nodes(s, &i, &pos, bar, foo, cmdParams) == -1)
		return 1;

	vocab_dump_parse_tree("debug-parse-tree", s->parser_nodes);
	return 0;
}

extern const char *selector_name(EngineState *s, int selector);

int prop_ofs_to_id(EngineState *s, int prop_ofs, reg_t objp) {
	Object *obj = obj_get(s, objp);
	byte *selectoroffset;
	int selectors;

	if (!obj) {
		sciprintf("Applied prop_ofs_to_id on non-object at %04x:%04x\n", PRINT_REG(objp));
		return -1;
	}

	selectors = obj->_variables.size();

	if (s->_version < SCI_VERSION_1_1)
		selectoroffset = ((byte *)(obj->base_obj)) + SCRIPT_SELECTOR_OFFSET + selectors * 2;
	else {
		if (!(obj->_variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS)) {
			obj = obj_get(s, obj->_variables[SCRIPT_SUPERCLASS_SELECTOR]);
			selectoroffset = (byte *)obj->base_vars;
		} else
			selectoroffset = (byte *)obj->base_vars;
	}

	if (prop_ofs < 0 || (prop_ofs >> 1) >= selectors) {
		sciprintf("Applied prop_ofs_to_id to invalid property offset %x (property #%d not in [0..%d]) on object at %04x:%04x\n",
		          prop_ofs, prop_ofs >> 1, selectors - 1, PRINT_REG(objp));
		return -1;
	}

	return READ_LE_UINT16(selectoroffset + prop_ofs);
}

reg_t disassemble(EngineState *s, reg_t pos, int print_bw_tag, int print_bytecode) {
// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
	MemObject *mobj = GET_SEGMENT(*s->seg_manager, pos.segment, MEM_OBJ_SCRIPT);
	Script *script_entity = NULL;
	byte *scr;
	int scr_size;
	reg_t retval = make_reg(pos.segment, pos.offset + 1);
	uint16 param_value;
	int opsize;
	uint opcode;
	int bytecount = 1;
	int i = 0;

	if (!mobj) {
		sciprintf("Disassembly failed: Segment %04x non-existant or not a script\n", pos.segment);
		return retval;
	} else
		script_entity = (Script *)mobj;

	scr = script_entity->buf;
	scr_size = script_entity->buf_size;

	if (pos.offset >= scr_size) {
		sciprintf("Trying to disassemble beyond end of script\n");
		return pos;
	}

	opsize = scr[pos.offset];
	opcode = opsize >> 1;

	if (!g_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return retval;
	}

	opsize &= 1; // byte if true, word if false

	sciprintf("%04x:%04x: ", PRINT_REG(pos));

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
			sciprintf("Operation arguments extend beyond end of script\n");
			return retval;
		}

		for (i = 0; i < bytecount; i++)
			sciprintf("%02x ", scr[pos.offset + i]);

		for (i = bytecount; i < 5; i++)
			sciprintf("   ");
	}

	if (print_bw_tag)
		sciprintf("[%c] ", opsize ? 'B' : 'W');
	sciprintf("%s", s->_kernel->getOpcode(opcode).name.c_str());

	i = 0;
	while (g_opcode_formats[opcode][i]) {
		switch (g_opcode_formats[opcode][i++]) {
		case Script_Invalid:
			sciprintf("-Invalid operation-");
			break;

		case Script_SByte:
		case Script_Byte:
			sciprintf(" %02x", scr[retval.offset++]);
			break;

		case Script_Word:
		case Script_SWord:
			sciprintf(" %04x", 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8)));
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
				sciprintf(" %s[%x]", (param_value < s->_kernel->_kernelFuncs.size()) ?
							((param_value < s->_kernel->getKernelNamesSize()) ? s->_kernel->getKernelName(param_value).c_str() : "[Unknown(postulated)]")
							: "<invalid>", param_value);
			else
				sciprintf(opsize ? " %02x" : " %04x", param_value);

			break;

		case Script_Offset:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}
			sciprintf(opsize ? " %02x" : " %04x", param_value);
			break;

		case Script_SRelative:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = 0xffff & (scr[retval.offset] | (scr[retval.offset+1] << 8));
				retval.offset += 2;
			}
			sciprintf(opsize ? " %02x  [%04x]" : " %04x  [%04x]", param_value, (0xffff) & (retval.offset + param_value));
			break;

		case Script_End:
			retval = NULL_REG;
			break;

		default:
			sciprintf("Internal assertion failed in 'disassemble', %s, L%d\n", __FILE__, __LINE__);

		}
	}

	if (pos == *p_pc) { // Extra information if debugging the current opcode
		if ((opcode == op_pTos) || (opcode == op_sTop) || (opcode == op_pToa) || (opcode == op_aTop) ||
		        (opcode == op_dpToa) || (opcode == op_ipToa) || (opcode == op_dpTos) || (opcode == op_ipTos)) {
			int prop_ofs = scr[pos.offset + 1];
			int prop_id = prop_ofs_to_id(s, prop_ofs, *p_objp);

			sciprintf("	(%s)", selector_name(s, prop_id));
		}
	}

	sciprintf("\n");

	if (pos == *p_pc) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.offset + 2] >> 1) + (*p_restadjust);
			int argc = ((*p_sp)[- stackframe - 1]).offset;

			if (!(s->_flags & GF_SCI0_OLD))
				argc += (*p_restadjust);

			sciprintf(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				sciprintf("%04x:%04x", PRINT_REG((*p_sp)[j - stackframe]));
				if (j + 1 < argc)
					sciprintf(", ");
			}
			sciprintf(")\n");
		} else if ((opcode == op_send) || (opcode == op_self)) {
			int restmod = *p_restadjust;
			int stackframe = (scr[pos.offset + 1] >> 1) + restmod;
			reg_t *sb = *p_sp;
			uint16 selector;
			reg_t fun_ref;

			while (stackframe > 0) {
				int argc = sb[- stackframe + 1].offset;
				const char *name = NULL;
				reg_t called_obj_addr = *p_objp;

				if (opcode == op_send)
					called_obj_addr = s->r_acc;
				else if (opcode == op_self)
					called_obj_addr = *p_objp;

				selector = sb[- stackframe].offset;

				name = obj_get_name(s, called_obj_addr);

				if (!name)
					name = "<invalid>";

				sciprintf("  %s::%s[", name, (selector > s->_kernel->getSelectorNamesSize()) ? "<invalid>" : selector_name(s, selector));

				switch (lookup_selector(s, called_obj_addr, selector, 0, &fun_ref)) {
				case kSelectorMethod:
					sciprintf("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case kSelectorVariable:
					sciprintf("VAR");
					break;
				case kSelectorNone:
					sciprintf("INVALID");
					break;
				}

				sciprintf("](");

				while (argc--) {
					sciprintf("%04x:%04x", PRINT_REG(sb[- stackframe + 2]));
					if (argc)
						sciprintf(", ");
					stackframe--;
				}

				sciprintf(")\n");
				stackframe -= 2;
			} // while (stackframe > 0)
		} // Send-like opcodes
	} // (heappos == *p_pc)

	return retval;
}

static int c_disasm_addr(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t vpc = cmdParams[0].reg;
	int op_count = 1;
	int do_bwc = 0;
	int do_bytes = 0;
	unsigned int i;
	int invalid = 0;
	int size;

	s->seg_manager->dereference(vpc, &size);
	size += vpc.offset; // total segment size

	for (i = 1; i < cmdParams.size(); i++) {
		if (!scumm_stricmp(cmdParams[i].str, "bwt"))
			do_bwc = 1;
		else if (!scumm_stricmp(cmdParams[i].str, "bc"))
			do_bytes = 1;
		else if (toupper(cmdParams[i].str[0]) == 'C')
			op_count = atoi(cmdParams[i].str + 1);
		else {
			invalid = 1;
			sciprintf("Invalid option '%s'\n", cmdParams[i].str);
		}
	}

	if (invalid || op_count < 0)
		return invalid;

	do {
		vpc = disassemble(s, vpc, do_bwc, do_bytes);

	} while ((vpc.offset > 0) && (vpc.offset + 6 < size) && (--op_count));
	return 0;
}

static int c_disasm(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	Object *obj = obj_get(s, cmdParams[0].reg);
	int selector_id = s->_kernel->findSelector(cmdParams[1].str);
	reg_t addr;

	if (!obj) {
		sciprintf("Not an object.");
		return 1;
	}

	if (selector_id < 0) {
		sciprintf("Not a valid selector name.");
		return 1;
	}

	if (lookup_selector(s, cmdParams[0].reg, selector_id, NULL, &addr) != kSelectorMethod) {
		sciprintf("Not a method.");
		return 1;
	}

	do {
		addr = disassemble(s, addr, 0, 0);
	} while (addr.offset > 0);

	return 0;
}

static int c_sg(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	g_debug_seeking = _DEBUG_SEEK_GLOBAL;
	s_debug_seek_special = cmdParams[0].val;
	g_debugstate_valid = 0;

	return 0;
}

static int c_snk(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	int callk_index;
	char *endptr;

	if (!g_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmdParams.size() > 0) {
		/* Try to convert the parameter to a number. If the conversion stops
		   before end of string, assume that the parameter is a function name
		   and scan the function table to find out the index. */
		callk_index = strtoul(cmdParams [0].str, &endptr, 0);
		if (*endptr != '\0') {
			callk_index = -1;
			for (uint i = 0; i < s->_kernel->getKernelNamesSize(); i++)
				if (cmdParams [0].str == s->_kernel->getKernelName(i)) {
					callk_index = i;
					break;
				}

			if (callk_index == -1) {
				sciprintf("Unknown kernel function '%s'\n", cmdParams[0].str);
				return 1;
			}
		}

		g_debug_seeking = _DEBUG_SEEK_SPECIAL_CALLK;
		s_debug_seek_special = callk_index;
		g_debugstate_valid = 0;
	} else {
		g_debug_seeking = _DEBUG_SEEK_CALLK;
		g_debugstate_valid = 0;
	}

	return 0;
}

static int c_sret(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	g_debug_seeking = _DEBUG_SEEK_LEVEL_RET;
	s_debug_seek_level = s->_executionStack.size()-1;
	g_debugstate_valid = 0;
	return 0;
}

static int c_go(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	g_debug_seeking = 0;
	g_debugstate_valid = 0;
	return 0;
}

static int c_send(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t object = cmdParams[0].reg;
	const char *selector_name = cmdParams[1].str;
	StackPtr stackframe = s->_executionStack.front().sp;
	int selector_id;
	unsigned int i;
	ExecStack *xstack;
	Object *o;
	reg_t fptr;

	selector_id = s->_kernel->findSelector(selector_name);

	if (selector_id < 0) {
		sciprintf("Unknown selector: \"%s\"\n", selector_name);
		return 1;
	}

	o = obj_get(s, object);
	if (o == NULL) {
		sciprintf("Address \"%04x:%04x\" is not an object\n", PRINT_REG(object));
		return 1;
	}

	SelectorType selector_type = lookup_selector(s, object, selector_id, 0, &fptr);

	if (selector_type == kSelectorNone) {
		sciprintf("Object does not support selector: \"%s\"\n", selector_name);
		return 1;
	}

	stackframe[0] = make_reg(0, selector_id);
	stackframe[1] = make_reg(0, cmdParams.size() - 2);

	for (i = 2; i < cmdParams.size(); i++)
		stackframe[i] = cmdParams[i].reg;

	xstack = add_exec_stack_entry(s, fptr,
	                 s->_executionStack.front().sp + cmdParams.size(),
	                 object, cmdParams.size() - 2,
	                 s->_executionStack.front().sp - 1, 0, object,
	                 s->_executionStack.size()-1, SCI_XS_CALLEE_LOCALS);
	xstack->selector = selector_id;
	xstack->type = selector_type == kSelectorVariable ? EXEC_STACK_TYPE_VARSELECTOR : EXEC_STACK_TYPE_CALL;

	// Now commit the actual function:
	xstack = send_selector(s, object, object, stackframe, cmdParams.size() - 2, stackframe);

	xstack->sp += cmdParams.size();
	xstack->fp += cmdParams.size();

	s->_executionStackPosChanged = true;

	return 0;
}

// Breakpoint commands

int c_se(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	g_stop_on_event = 1;
	g_debugstate_valid = 0;

	return 0;
}

void script_debug(EngineState *s, reg_t *pc, StackPtr *sp, StackPtr *pp, reg_t *objp, int *restadjust,
	SegmentId *segids, reg_t **variables, reg_t **variables_base, int *variables_nr, int bp) {
	// Do we support a separate console?

	int old_debugstate = g_debugstate_valid;

	p_var_segs = segids;
	p_vars = variables;
	p_var_max = variables_nr;
	p_var_base = variables_base;
	p_pc = pc;
	p_sp = sp;
	p_pp = pp;
	p_objp = objp;
	p_restadjust = restadjust;
	sciprintf("%d: acc=%04x:%04x  ", script_step_counter, PRINT_REG(s->r_acc));
	g_debugstate_valid = 1;
	disassemble(s, *pc, 0, 1);
	if (g_debug_seeking == _DEBUG_SEEK_GLOBAL)
		sciprintf("Global %d (0x%x) = %04x:%04x\n", s_debug_seek_special,
		          s_debug_seek_special, PRINT_REG(s->script_000->locals_block->_locals[s_debug_seek_special]));

	g_debugstate_valid = old_debugstate;

	if (g_debug_seeking && !bp) { // Are we looking for something special?
		MemObject *mobj = GET_SEGMENT(*s->seg_manager, pc->segment, MEM_OBJ_SCRIPT);

		if (mobj) {
			Script *scr = (Script *)mobj;
			byte *code_buf = scr->buf;
			int code_buf_size = scr->buf_size;
			int opcode = pc->offset >= code_buf_size ? 0 : code_buf[pc->offset];
			int op = opcode >> 1;
			int paramb1 = pc->offset + 1 >= code_buf_size ? 0 : code_buf[pc->offset + 1];
			int paramf1 = (opcode & 1) ? paramb1 : (pc->offset + 2 >= code_buf_size ? 0 : (int16)READ_LE_UINT16(code_buf + pc->offset + 1));

			switch (g_debug_seeking) {
			case _DEBUG_SEEK_SPECIAL_CALLK:
				if (paramb1 != s_debug_seek_special)
					return;

			case _DEBUG_SEEK_CALLK: {
				if (op != op_callk)
					return;
				break;
			}

			case _DEBUG_SEEK_LEVEL_RET: {
				if ((op != op_ret) || (s_debug_seek_level < (int)s->_executionStack.size()-1))
					return;
				break;
			}

			case _DEBUG_SEEK_GLOBAL:
				if (op < op_sag)
					return;
				if ((op & 0x3) > 1)
					return; // param or temp
				if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
					return; // locals and not running in script.000
				if (paramf1 != s_debug_seek_special)
					return; // CORRECT global?
				break;

			}

			g_debug_seeking = _DEBUG_SEEK_NOTHING;
			// OK, found whatever we were looking for
		}
	}

	g_debugstate_valid = (g_debug_step_running == 0);

	if (g_debugstate_valid) {
		p_pc = pc;
		p_sp = sp;
		p_pp = pp;
		p_objp = objp;
		p_restadjust = restadjust;
		p_var_segs = segids;
		p_vars = variables;
		p_var_max = variables_nr;
		p_var_base = variables_base;
		
		sciprintf("Step #%d\n", script_step_counter);
		disassemble(s, *pc, 0, 1);

		if (!s_debug_commands_hooked) {
			s_debug_commands_hooked = true;

			con_hook_command(c_step, "s", "i*", "Executes one or several operations\n\nEXAMPLES\n\n"
			                 "    s 4\n\n  Execute 4 commands\n\n    s\n\n  Execute next command");
			con_hook_command(c_disasm_addr, "disasm-addr", "!as*", "Disassembles one or more commands\n\n"
			                 "USAGE\n\n  disasm-addr [startaddr] <options>\n\n"
			                 "  Valid options are:\n"
			                 "  bwt  : Print byte/word tag\n"
			                 "  c<x> : Disassemble <x> bytes\n"
			                 "  bc   : Print bytecode\n\n");
			con_hook_command(c_disasm, "disasm", "!as", "Disassembles a method by name\n\nUSAGE\n\n  disasm <obj> <method>\n\n");
			con_hook_command(c_snk, "snk", "s*", "Steps forward until it hits the next\n  callk operation.\n"
			                 "  If invoked with a parameter, it will\n  look for that specific callk.\n");
			con_hook_command(c_se, "se", "", "Steps forward until an SCI event is received.\n");
			con_hook_command(c_send, "send", "!asa*", "Sends a message to an object\nExample: send ?fooScript cue");
			con_hook_command(c_sret, "sret", "", "Steps forward until ret is called\n  on the current execution stack\n  level.");
			con_hook_command(c_go, "go", "", "Executes the script.\n");
			con_hook_command(c_set_parse_nodes, "set_parse_nodes", "s*", "Sets the contents of all parse nodes.\n"
			                 "  Input token must be separated by\n  blanks.");
			con_hook_command(c_sg, "sg", "!i",
			                 "Steps until the global variable with the\n"
			                 "specified index is modified.\n\nSEE ALSO\n\n"
			                 "  s.1, snk.1, so.1, bpx.1");
		} // If commands were not hooked up
	}

	if (g_debug_step_running)
		g_debug_step_running--;
}

} // End of namespace Sci
