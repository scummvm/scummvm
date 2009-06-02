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

int _debugstate_valid = 0; // Set to 1 while script_debug is running
int _debug_step_running = 0; // Set to >0 to allow multiple stepping
int _debug_commands_not_hooked = 1; // Commands not hooked to the console yet?
int _debug_seeking = 0; // Stepping forward until some special condition is met
int _debug_seek_level = 0; // Used for seekers that want to check their exec stack depth
int _debug_seek_special = 0;  // Used for special seeks(1)
reg_t _debug_seek_reg = NULL_REG;  // Used for special seeks(2)

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

#if 0
// Unused
#define LOOKUP_SPECIES(species) (\
	(species >= 1000) ? species : *(s->_classtable[species].scriptposp) \
		+ s->_classtable[species].class_offset)
#endif

// Dummy function, so that it compiles
int con_hook_command(ConCommand command, const char *name, const char *param, const char *description) {

	return 0;
}

static const char *_debug_get_input() {
	char newinpbuf[256];

	printf("> ");
	if (!fgets(newinpbuf, 254, stdin))
		return NULL;

	size_t l = strlen(newinpbuf);
	if (l > 0 && newinpbuf[0] != '\n') {
		if (newinpbuf[l-1] == '\n') newinpbuf[l-1] = 0;
		memcpy(inputbuf, newinpbuf, 256);
	}

	return inputbuf;
}

static int show_node(EngineState *s, reg_t addr) {
	MemObject *mobj = GET_SEGMENT(*s->seg_manager, addr.segment, MEM_OBJ_LISTS);

	if (mobj) {
		ListTable *lt = (ListTable *)mobj;
		List *list;

		if (!lt->isValidEntry(addr.offset)) {
			sciprintf("Address does not contain a list\n");
			return 1;
		}

		list = &(lt->_table[addr.offset]);

		sciprintf("%04x:%04x : first x last = (%04x:%04x, %04x:%04x)\n", PRINT_REG(addr), PRINT_REG(list->first), PRINT_REG(list->last));
	} else {
		NodeTable *nt;
		Node *node;
		mobj = GET_SEGMENT(*s->seg_manager, addr.segment, MEM_OBJ_NODES);

		if (!mobj) {
			sciprintf("Segment #%04x is not a list or node segment\n", addr.segment);
			return 1;
		}

		nt = (NodeTable *)mobj;

		if (!nt->isValidEntry(addr.offset)) {
			sciprintf("Address does not contain a node\n");
			return 1;
		}
		node = &(nt->_table[addr.offset]);

		sciprintf("%04x:%04x : prev x next = (%04x:%04x, %04x:%04x); maps %04x:%04x -> %04x:%04x\n",
		          PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(node->succ), PRINT_REG(node->key), PRINT_REG(node->value));
	}

	return 0;
}

int objinfo(EngineState *s, reg_t pos);

static int c_vr(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t reg = cmdParams[0].reg;
	reg_t reg_end = cmdParams.size() > 1 ? cmdParams[1].reg : NULL_REG;
	int type_mask = determine_reg_type(s, reg, 1);
	int filter;
	int found = 0;

	sciprintf("%04x:%04x is of type 0x%x%s: ", PRINT_REG(reg), type_mask & ~KSIG_INVALID, type_mask & KSIG_INVALID ? " (invalid)" : "");

	type_mask &= ~KSIG_INVALID;

	if (reg.segment == 0 && reg.offset == 0) {
		sciprintf("Null.\n");
		return 0;
	}

	if (reg_end.segment != reg.segment) {
		sciprintf("Ending segment different from starting segment. Assuming no bound on dump.\n");
		reg_end = NULL_REG;
	}

	for (filter = 1; filter < 0xf000; filter <<= 1) {
		int type = type_mask & filter;

		if (found && type) {
			sciprintf("--- Alternatively, it could be a ");
		}


		switch (type) {
		case 0:
			break;

		case KSIG_LIST: {
			//List *l = lookup_list(s, reg);

			sciprintf("list\n");

			// TODO: printList has been moved to console.cpp
			/*
			if (l)
				printList(l);
			else
				sciprintf("Invalid list.\n");
			*/
		}
		break;

		case KSIG_NODE:
			sciprintf("list node\n");
			show_node(s, reg);
			break;

		case KSIG_OBJECT:
			sciprintf("object\n");
			objinfo(s, reg);
			break;

		case KSIG_REF: {
			int size;
			unsigned char *block = s->seg_manager->dereference(reg, &size);

			sciprintf("raw data\n");

			if (reg_end.segment != 0 && size < reg_end.offset - reg.offset) {
				sciprintf("Block end out of bounds (size %d). Resetting.\n", size);
				reg_end = NULL_REG;
			}

			if (reg_end.segment != 0 && (size >= reg_end.offset - reg.offset))
				size = reg_end.offset - reg.offset;

			if (reg_end.segment != 0)
				sciprintf("Block size less than or equal to %d\n", size);

			Common::hexdump(block, size, 16, 0);
		}
		break;

		case KSIG_ARITHMETIC:
			sciprintf("arithmetic value\n  %d (%04x)\n", (int16) reg.offset, reg.offset);
			break;

		default:
			sciprintf("unknown type %d.\n", type);

		}

		if (type) {
			sciprintf("\n");
			found = 1;
		}
	}

	return 0;
}

int c_debuginfo(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	sciprintf("acc=%04x:%04x prev=%04x:%04x &rest=%x\n", PRINT_REG(s->r_acc), PRINT_REG(s->r_prev), *p_restadjust);

	if (!s->_executionStack.empty()) {
		sciprintf("pc=%04x:%04x obj=%04x:%04x fp=ST:%04x sp=ST:%04x\n", PRINT_REG(*p_pc), PRINT_REG(*p_objp), PRINT_STK(*p_pp), PRINT_STK(*p_sp));
	} else
		sciprintf("<no execution stack: pc,obj,fp omitted>\n");

	return 0;
}

int c_step(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	_debugstate_valid = 0;
	if (cmdParams.size() && (cmdParams[0].val > 0))
		_debug_step_running = cmdParams[0].val - 1;

	return 0;
}

#if 0
// TODO Re-implement con:so
int c_stepover(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	int opcode, opnumber;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	_debugstate_valid = 0;
	opcode = s->_heap[*p_pc];
	opnumber = opcode >> 1;
	if (opnumber == 0x22 /* callb */ || opnumber == 0x23 /* calle */ ||
	        opnumber == 0x25 /* send */ || opnumber == 0x2a /* self */ || opnumber == 0x2b /* super */) {
		_debug_seeking = _DEBUG_SEEK_SO;
		_debug_seek_level = s->_executionStack.size()-1;
		// Store in _debug_seek_special the offset of the next command after send
		switch (opcode) {
		case 0x46: // calle W
			_debug_seek_special = *p_pc + 5;
			break;

		case 0x44: // callb W
		case 0x47: // calle B
		case 0x56: // super W
			_debug_seek_special = *p_pc + 4;
			break;

		case 0x45: // callb B
		case 0x57: // super B
		case 0x4A: // send W
		case 0x54: // self W
			_debug_seek_special = *p_pc + 3;
			break;

		default:
			_debug_seek_special = *p_pc + 2;
		}
	}

	return 0;
}
#endif

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

// parses with a GNF rule set

int c_parse(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	ResultWordList words;
	char *error;
	const char *string;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	string = cmdParams[0].str;
	sciprintf("Parsing '%s'\n", string);
	bool res = s->_vocabulary->tokenizeString(words, string, &error);
	if (res && !words.empty()) {
		int syntax_fail = 0;

		vocab_synonymize_tokens(words, s->_synonyms);

		sciprintf("Parsed to the following blocks:\n");

		for (ResultWordList::const_iterator i = words.begin(); i != words.end(); ++i)
			sciprintf("   Type[%04x] Group[%04x]\n", i->_class, i->_group);

		if (s->_vocabulary->parseGNF(s->parser_nodes, words, true))
			syntax_fail = 1; // Building a tree failed

		if (syntax_fail)
			sciprintf("Building a tree failed.\n");
		else
			vocab_dump_parse_tree("debug-parse-tree", s->parser_nodes);

	} else {
		sciprintf("Unknown word: '%s'\n", error);
		free(error);
	}

	return 0;
}

const char *selector_name(EngineState *s, int selector) {
	if (selector >= 0 && selector < (int)s->_kernel->getSelectorNamesSize())
		return s->_kernel->getSelectorName(selector).c_str();
	else
		return "--INVALID--";
}

int prop_ofs_to_id(EngineState *s, int prop_ofs, reg_t objp) {
	Object *obj = obj_get(s, objp);
	byte *selectoroffset;
	int selectors;

	if (!obj) {
		sciprintf("Applied prop_ofs_to_id on non-object at %04x:%04x\n", PRINT_REG(objp));
		return -1;
	}

	selectors = obj->_variables.size();

	if (s->version < SCI_VERSION_1_1)
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

	if (!_debugstate_valid) {
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
				sciprintf(" %s[%x]", (param_value < s->_kernel->_kfuncTable.size()) ?
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

			if (!(s->flags & GF_SCI0_OLD))
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
			reg_t *val_ref;
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

				switch (lookup_selector(s, called_obj_addr, selector, &val_ref, &fun_ref)) {
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

int c_vmvars(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	const char *varnames[] = {"global", "local", "temp", "param"};
	const char *varabbrev = "gltp";
	const char *vartype_pre = strchr(varabbrev, *cmdParams[0].str);
	int vartype;
	int idx = cmdParams[1].val;

	if (!vartype_pre) {
		sciprintf("Invalid variable type '%c'\n", *cmdParams[0].str);
		return 1;
	}
	vartype = vartype_pre - varabbrev;

	if (idx < 0) {
		sciprintf("Invalid: negative index\n");
		return 1;
	}
	if ((p_var_max) && (p_var_max[vartype] <= idx)) {
		sciprintf("Max. index is %d (0x%x)\n", p_var_max[vartype], p_var_max[vartype]);
		return 1;
	}

	switch (cmdParams.size()) {
	case 2:
		sciprintf("%s var %d == %04x:%04x\n", varnames[vartype], idx, PRINT_REG(p_vars[vartype][idx]));
		break;

	case 3:
		p_vars[vartype][idx] = cmdParams[2].reg;
		break;

	default:
		sciprintf("Too many arguments\n");
	}

	return 0;
}

static int c_backtrace(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	sciprintf("Call stack (current base: 0x%x):\n", s->execution_stack_base);
	Common::List<ExecStack>::iterator iter;
	uint i = 0;
	for (iter = s->_executionStack.begin();
	     iter != s->_executionStack.end(); ++iter, ++i) {
		ExecStack &call = *iter;
		const char *objname = obj_get_name(s, call.sendp);
		int paramc, totalparamc;

		switch (call.type) {

		case EXEC_STACK_TYPE_CALL: {// Normal function
			sciprintf(" %x:[%x]  %s::%s(", i, call.origin, objname, (call.selector == -1) ? "<call[be]?>" :
			          selector_name(s, call.selector));
		}
		break;

		case EXEC_STACK_TYPE_KERNEL: // Kernel function
			sciprintf(" %x:[%x]  k%s(", i, call.origin, s->_kernel->getKernelName(-(call.selector) - 42).c_str());
			break;

		case EXEC_STACK_TYPE_VARSELECTOR:
			sciprintf(" %x:[%x] vs%s %s::%s (", i, call.origin, (call.argc) ? "write" : "read",
			          objname,s->_kernel->getSelectorName(call.selector).c_str());
			break;
		}

		totalparamc = call.argc;

		if (totalparamc > 16)
			totalparamc = 16;

		for (paramc = 1; paramc <= totalparamc; paramc++) {
			sciprintf("%04x:%04x", PRINT_REG(call.variables_argp[paramc]));

			if (paramc < call.argc)
				sciprintf(", ");
		}

		if (call.argc > 16)
			sciprintf("...");

		sciprintf(")\n    obj@%04x:%04x", PRINT_REG(call.objp));
		if (call.type == EXEC_STACK_TYPE_CALL) {
			sciprintf(" pc=%04x:%04x", PRINT_REG(call.addr.pc));
			if (call.sp == CALL_SP_CARRY)
				sciprintf(" sp,fp:carry");
			else {
				sciprintf(" sp=ST:%04x", PRINT_STK(call.sp));
				sciprintf(" fp=ST:%04x", PRINT_STK(call.fp));
			}
		} else
			sciprintf(" pc:none");

		sciprintf(" argp:ST:%04x", PRINT_STK(call.variables_argp));
		if (call.type == EXEC_STACK_TYPE_CALL)
			sciprintf(" script: %d", (*(Script *)s->seg_manager->_heap[call.addr.pc.segment]).nr);
		sciprintf("\n");
	}

	return 0;
}

#ifdef GFXW_DEBUG_WIDGETS
extern GfxWidget *debug_widgets[];
extern int debug_widget_pos;

static int c_gfx_print_widget(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmdParams.size()) {
		unsigned int i;
		for (i = 0; i < cmdParams.size() ; i++) {
			int widget_nr = cmdParams[i].val;

			sciprintf("===== Widget #%d:\n", widget_nr);
			debug_widgets[widget_nr]->print(0);
		}

	} else if (debug_widget_pos > 1)
		sciprintf("Widgets 0-%d are active\n", debug_widget_pos - 1);
	else if (debug_widget_pos == 1)
		sciprintf("Widget 0 is active\n");
	else
		sciprintf("No widgets are active\n");

	return 0;
}
#endif

#define GETRECT(ll, rr, tt, bb) \
	ll = GET_SELECTOR(pos, ll); \
	rr = GET_SELECTOR(pos, rr); \
	tt = GET_SELECTOR(pos, tt); \
	bb = GET_SELECTOR(pos, bb);

#if 0
// Unreferenced - removed
static int c_gfx_draw_viewobj(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
#ifdef __GNUC__
#warning "Re-implement con:gfx_draw_viewobj"
#endif
#if 0
	HeapPtr pos = (HeapPtr)(cmdParams[0].val);
	int is_view;
	int x, y, priority;
	int nsLeft, nsRight, nsBottom, nsTop;
	int brLeft, brRight, brBottom, brTop;

	if (!s) {
		sciprintf("Not in debug state!\n");
		return 1;
	}

	if ((pos < 4) || (pos > 0xfff0)) {
		sciprintf("Invalid address.\n");
		return 1;
	}

	if (((int16)READ_LE_UINT16(s->heap + pos + SCRIPT_OBJECT_MAGIC_OFFSET)) != SCRIPT_OBJECT_MAGIC_NUMBER) {
		sciprintf("Not an object.\n");
		return 0;
	}


	is_view = (lookup_selector(s, pos, s->_kernel->_selectorMap.x, NULL) == kSelectorVariable) &&
	    (lookup_selector(s, pos, s->_kernel->_selectorMap.brLeft, NULL) == kSelectorVariable) &&
	    (lookup_selector(s, pos, s->_kernel->_selectorMap.signal, NULL) == kSelectorVariable) &&
	    (lookup_selector(s, pos, s->_kernel->_selectorMap.nsTop, NULL) == kSelectorVariable);

	if (!is_view) {
		sciprintf("Not a dynamic View object.\n");
		return 0;
	}

	x = GET_SELECTOR(pos, x);
	y = GET_SELECTOR(pos, y);
	priority = GET_SELECTOR(pos, priority);
	GETRECT(brLeft, brRight, brBottom, brTop);
	GETRECT(nsLeft, nsRight, nsBottom, nsTop);
	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	brTop += 10;
	brBottom += 10;
	nsTop += 10;
	nsBottom += 10;

	gfxop_fill_box(s->gfx_state, gfx_rect(nsLeft, nsTop, nsRight - nsLeft + 1, nsBottom - nsTop + 1), s->ega_colors[2]);

	gfxop_fill_box(s->gfx_state, gfx_rect(brLeft, brTop, brRight - brLeft + 1, brBottom - brTop + 1), s->ega_colors[1]);

	gfxop_fill_box(s->gfx_state, gfx_rect(x - 1, y - 1, 3, 3), s->ega_colors[0]);

	gfxop_fill_box(s->gfx_state, gfx_rect(x - 1, y, 3, 1), s->ega_colors[priority]);

	gfxop_fill_box(s->gfx_state, gfx_rect(x, y - 1, 1, 3), s->ega_colors[priority]);

	gfxop_update(s->gfx_state);

	return 0;
#endif
}
#endif

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
	_debug_seeking = _DEBUG_SEEK_GLOBAL;
	_debug_seek_special = cmdParams[0].val;
	_debugstate_valid = 0;

	return 0;
}

static int c_snk(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	int callk_index;
	char *endptr;

	if (!_debugstate_valid) {
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

		_debug_seeking = _DEBUG_SEEK_SPECIAL_CALLK;
		_debug_seek_special = callk_index;
		_debugstate_valid = 0;
	} else {
		_debug_seeking = _DEBUG_SEEK_CALLK;
		_debugstate_valid = 0;
	}

	return 0;
}

static int c_sret(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	_debug_seeking = _DEBUG_SEEK_LEVEL_RET;
	_debug_seek_level = s->_executionStack.size()-1;
	_debugstate_valid = 0;
	return 0;
}

static int c_go(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	_debug_seeking = 0;
	_debugstate_valid = 0;
	return 0;
}

static int c_set_acc(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	s->r_acc = cmdParams[0].reg;
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
	reg_t *vptr;
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

	SelectorType selector_type = lookup_selector(s, object, selector_id, &vptr, &fptr);

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


struct generic_config_flag_t {
	const char *name;
	const char option;
	unsigned int flag;
};

#define SFX_DEBUG_MODES 2
#define FROBNICATE_HANDLE(reg) ((reg).segment << 16 | (reg).offset)

static int c_sfx_remove(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t id = cmdParams[0].reg;
	int handle = FROBNICATE_HANDLE(id);

	if (id.segment) {
		s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		s->_sound.sfx_remove_song(handle);
		PUT_SEL32V(id, signal, -1);
		PUT_SEL32V(id, nodePtr, 0);
		PUT_SEL32V(id, handle, 0);
	}

	return 0;
}

#define GETRECT(ll, rr, tt, bb) \
	ll = GET_SELECTOR(pos, ll); \
	rr = GET_SELECTOR(pos, rr); \
	tt = GET_SELECTOR(pos, tt); \
	bb = GET_SELECTOR(pos, bb);

#if 0
#ifdef __GNUC__
#warning "Re-implement viewobjinfo"
#endif
static void viewobjinfo(EngineState *s, HeapPtr pos) {
	char *signals[16] = {
		"stop_update",
		"updated",
		"no_update",
		"hidden",
		"fixed_priority",
		"always_update",
		"force_update",
		"remove",
		"frozen",
		"is_extra",
		"hit_obstacle",
		"doesnt_turn",
		"no_cycler",
		"ignore_horizon",
		"ignore_actor",
		"dispose!"
	};

	int x, y, z, priority;
	int cel, loop, view, signal;
	int nsLeft, nsRight, nsBottom, nsTop;
	int lsLeft, lsRight, lsBottom, lsTop;
	int brLeft, brRight, brBottom, brTop;
	int i;
	int have_rects = 0;
	Common::Rect nsrect, nsrect_clipped, brrect;

	if (lookup_selector(s, pos, s->_kernel->_selectorMap.nsBottom, NULL) == kSelectorVariable) {
		GETRECT(nsLeft, nsRight, nsBottom, nsTop);
		GETRECT(lsLeft, lsRight, lsBottom, lsTop);
		GETRECT(brLeft, brRight, brBottom, brTop);
		have_rects = 1;
	}

	GETRECT(view, loop, signal, cel);

	sciprintf("\n-- View information:\ncel %d/%d/%d at ", view, loop, cel);

	x = GET_SELECTOR(pos, x);
	y = GET_SELECTOR(pos, y);
	priority = GET_SELECTOR(pos, priority);
	if (s->_kernel->_selectorMap.z > 0) {
		z = GET_SELECTOR(pos, z);
		sciprintf("(%d,%d,%d)\n", x, y, z);
	} else
		sciprintf("(%d,%d)\n", x, y);

	if (priority == -1)
		sciprintf("No priority.\n\n");
	else
		sciprintf("Priority = %d (band starts at %d)\n\n", priority, PRIORITY_BAND_FIRST(priority));

	if (have_rects) {
		sciprintf("nsRect: [%d..%d]x[%d..%d]\n", nsLeft, nsRight, nsTop, nsBottom);
		sciprintf("lsRect: [%d..%d]x[%d..%d]\n", lsLeft, lsRight, lsTop, lsBottom);
		sciprintf("brRect: [%d..%d]x[%d..%d]\n", brLeft, brRight, brTop, brBottom);
	}

	nsrect = get_nsrect(s, pos, 0);
	nsrect_clipped = get_nsrect(s, pos, 1);
	brrect = set_base(s, pos);
	sciprintf("new nsRect: [%d..%d]x[%d..%d]\n", nsrect.x, nsrect.xend, nsrect.y, nsrect.yend);
	sciprintf("new clipped nsRect: [%d..%d]x[%d..%d]\n", nsrect_clipped.x, nsrect_clipped.xend, nsrect_clipped.y, nsrect_clipped.yend);
	sciprintf("new brRect: [%d..%d]x[%d..%d]\n", brrect.x, brrect.xend, brrect.y, brrect.yend);
	sciprintf("\n signals = %04x:\n", signal);

	for (i = 0; i < 16; i++)
		if (signal & (1 << i))
			sciprintf("  %04x: %s\n", 1 << i, signals[i]);
}
#endif
#undef GETRECT

int objinfo(EngineState *s, reg_t pos) {
	Object *obj = obj_get(s, pos);
	Object *var_container = obj;
	int i;

	if (!obj) {
		sciprintf("[%04x:%04x]: Not an object.", PRINT_REG(pos));
		return 1;
	}

	// Object header
	sciprintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(pos), obj_get_name(s, pos),
				obj->_variables.size(), obj->methods_nr);

	if (!(obj->_variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS))
		var_container = obj_get(s, obj->_variables[SCRIPT_SUPERCLASS_SELECTOR]);
	sciprintf("  -- member variables:\n");
	for (i = 0; (uint)i < obj->_variables.size(); i++) {
		sciprintf("    ");
		if (i < var_container->variable_names_nr) {
			sciprintf("[%03x] %s = ", VM_OBJECT_GET_VARSELECTOR(var_container, i), selector_name(s, VM_OBJECT_GET_VARSELECTOR(var_container, i)));
		} else
			sciprintf("p#%x = ", i);

		reg_t val = obj->_variables[i];
		sciprintf("%04x:%04x", PRINT_REG(val));

		Object *ref = obj_get(s, val);
		if (ref)
			sciprintf(" (%s)", obj_get_name(s, val));

		sciprintf("\n");
	}
	sciprintf("  -- methods:\n");
	for (i = 0; i < obj->methods_nr; i++) {
		reg_t fptr = VM_OBJECT_READ_FUNCTION(obj, i);
		sciprintf("    [%03x] %s = %04x:%04x\n", VM_OBJECT_GET_FUNCSELECTOR(obj, i), selector_name(s, VM_OBJECT_GET_FUNCSELECTOR(obj, i)), PRINT_REG(fptr));
	}
	if (s->seg_manager->_heap[pos.segment]->getType() == MEM_OBJ_SCRIPT)
		sciprintf("\nOwner script:\t%d\n", s->seg_manager->getScript(pos.segment)->nr);

	return 0;
}

int c_vo(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	return objinfo(s, cmdParams[0].reg);
}

int c_obj(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	return objinfo(s, *p_objp);
}

int c_accobj(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	return objinfo(s, s->r_acc);
}

int c_shownode(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t addr = cmdParams[0].reg;

	return show_node(s, addr);
}

// Breakpoint commands

static Breakpoint *bp_alloc(EngineState *s) {
	Breakpoint *bp;

	if (s->bp_list) {
		bp = s->bp_list;
		while (bp->next)
			bp = bp->next;
		bp->next = (Breakpoint *)malloc(sizeof(Breakpoint));
		bp = bp->next;
	} else {
		s->bp_list = (Breakpoint *)malloc(sizeof(Breakpoint));
		bp = s->bp_list;
	}

	bp->next = NULL;

	return bp;
}

int c_bpx(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	Breakpoint *bp;

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */

	bp = bp_alloc(s);

	bp->type = BREAK_SELECTOR;
	bp->data.name = (char *)malloc(strlen(cmdParams [0].str) + 1);
	strcpy(bp->data.name, cmdParams [0].str);
	s->have_bp |= BREAK_SELECTOR;

	return 0;
}

int c_bpe(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	Breakpoint *bp;

	bp = bp_alloc(s);

	bp->type = BREAK_EXPORT;
	bp->data.address = (cmdParams [0].val << 16 | cmdParams [1].val);
	s->have_bp |= BREAK_EXPORT;

	return 0;
}

int c_bplist(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	Breakpoint *bp;
	int i = 0;
	int bpdata;

	bp = s->bp_list;
	while (bp) {
		sciprintf("  #%i: ", i);
		switch (bp->type) {
		case BREAK_SELECTOR:
			sciprintf("Execute %s\n", bp->data.name);
			break;
		case BREAK_EXPORT:
			bpdata = bp->data.address;
			sciprintf("Execute script %d, export %d\n", bpdata >> 16, bpdata & 0xFFFF);
			break;
		}

		bp = bp->next;
		i++;
	}

	return 0;
}

int c_bpdel(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	Breakpoint *bp, *bp_next, *bp_prev;
	int i = 0, found = 0;
	int type;

	// Find breakpoint with given index
	bp_prev = NULL;
	bp = s->bp_list;
	while (bp && i < cmdParams [0].val) {
		bp_prev = bp;
		bp = bp->next;
		i++;
	}
	if (!bp) {
		sciprintf("Invalid breakpoint index %i\n", cmdParams [0].val);
		return 1;
	}

	// Delete it
	bp_next = bp->next;
	type = bp->type;
	if (type == BREAK_SELECTOR) free(bp->data.name);
	free(bp);
	if (bp_prev)
		bp_prev->next = bp_next;
	else
		s->bp_list = bp_next;

	// Check if there are more breakpoints of the same type. If not, clear
	// the respective bit in s->have_bp.
	for (bp = s->bp_list; bp; bp = bp->next) {
		if (bp->type == type) {
			found = 1;
			break;
		}
	}

	if (!found)
		s->have_bp &= ~type;

	return 0;
}

int c_se(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	stop_on_event = 1;
	_debugstate_valid = 0;

	return 0;
}

int c_type(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	int t = determine_reg_type(s, cmdParams[0].reg, 1);
	int invalid = t & KSIG_INVALID;

	switch (t & ~KSIG_INVALID) {
	case 0:
		sciprintf("Invalid");
		break;

	case KSIG_LIST:
		sciprintf("List");
		break;

	case KSIG_OBJECT:
		sciprintf("Object");
		break;

	case KSIG_REF:
		sciprintf("Reference");
		break;

	case KSIG_ARITHMETIC:
		sciprintf("Arithmetic");
		break;

	default:
		sciprintf("Erroneous unknown type %02x(%d decimal)\n", t, t);
	}

	sciprintf("%s\n", invalid ? " (invalid)" : "");

	return 0;
}

static void _print_address(void * _, reg_t addr) {
	if (addr.segment)
		sciprintf("  %04x:%04x\n", PRINT_REG(addr));
}

static int c_gc_show_reachable(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t addr = cmdParams[0].reg;

	MemObject *mobj = GET_SEGMENT_ANY(*s->seg_manager, addr.segment);
	if (!mobj) {
		sciprintf("Unknown segment : %x\n", addr.segment);
		return 1;
	}

	sciprintf("Reachable from %04x:%04x:\n", PRINT_REG(addr));
	mobj->listAllOutgoingReferences(s, addr, NULL, _print_address);

	return 0;
}

static int c_gc_show_freeable(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t addr = cmdParams[0].reg;

	MemObject *mobj = GET_SEGMENT_ANY(*s->seg_manager, addr.segment);
	if (!mobj) {
		sciprintf("Unknown segment : %x\n", addr.segment);
		return 1;
	}

	sciprintf("Freeable in segment %04x:\n", addr.segment);
	mobj->listAllDeallocatable(addr.segment, NULL, _print_address);

	return 0;
}

static int c_gc_normalise(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	reg_t addr = cmdParams[0].reg;

	MemObject *mobj = GET_SEGMENT_ANY(*s->seg_manager, addr.segment);
	if (!mobj) {
		sciprintf("Unknown segment : %x\n", addr.segment);
		return 1;
	}

	addr = mobj->findCanonicAddress(s->seg_manager, addr);
	sciprintf(" %04x:%04x\n", PRINT_REG(addr));

	return 0;
}

void script_debug(EngineState *s, reg_t *pc, StackPtr *sp, StackPtr *pp, reg_t *objp, int *restadjust,
	SegmentId *segids, reg_t **variables, reg_t **variables_base, int *variables_nr, int bp) {
	// Do we support a separate console?

	int old_debugstate = _debugstate_valid;

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
	_debugstate_valid = 1;
	disassemble(s, *pc, 0, 1);
	if (_debug_seeking == _DEBUG_SEEK_GLOBAL)
		sciprintf("Global %d (0x%x) = %04x:%04x\n", _debug_seek_special,
		          _debug_seek_special, PRINT_REG(s->script_000->locals_block->_locals[_debug_seek_special]));

	_debugstate_valid = old_debugstate;

	if (_debug_seeking && !bp) { // Are we looking for something special?
		MemObject *mobj = GET_SEGMENT(*s->seg_manager, pc->segment, MEM_OBJ_SCRIPT);

		if (mobj) {
			Script *scr = (Script *)mobj;
			byte *code_buf = scr->buf;
			int code_buf_size = scr->buf_size;
			int opcode = pc->offset >= code_buf_size ? 0 : code_buf[pc->offset];
			int op = opcode >> 1;
			int paramb1 = pc->offset + 1 >= code_buf_size ? 0 : code_buf[pc->offset + 1];
			int paramf1 = (opcode & 1) ? paramb1 : (pc->offset + 2 >= code_buf_size ? 0 : (int16)READ_LE_UINT16(code_buf + pc->offset + 1));

			switch (_debug_seeking) {
			case _DEBUG_SEEK_SPECIAL_CALLK:
				if (paramb1 != _debug_seek_special)
					return;

			case _DEBUG_SEEK_CALLK: {
				if (op != op_callk)
					return;
				break;
			}

			case _DEBUG_SEEK_LEVEL_RET: {
				if ((op != op_ret) || (_debug_seek_level < (int)s->_executionStack.size()-1))
					return;
				break;
			}

			case _DEBUG_SEEK_SO:
				if ((*pc != _debug_seek_reg) || (int)s->_executionStack.size()-1 != _debug_seek_level)
					return;
				break;

			case _DEBUG_SEEK_GLOBAL:

				if (op < op_sag)
					return;
				if ((op & 0x3) > 1)
					return; // param or temp
				if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
					return; // locals and not running in script.000
				if (paramf1 != _debug_seek_special)
					return; // CORRECT global?
				break;

			}

			_debug_seeking = _DEBUG_SEEK_NOTHING;
			// OK, found whatever we were looking for
		}
	}

	_debugstate_valid = (_debug_step_running == 0);

	if (_debugstate_valid) {
		p_pc = pc;
		p_sp = sp;
		p_pp = pp;
		p_objp = objp;
		p_restadjust = restadjust;
		p_var_segs = segids;
		p_vars = variables;
		p_var_max = variables_nr;
		p_var_base = variables_base;

		c_debuginfo(s);
		sciprintf("Step #%d\n", script_step_counter);
		disassemble(s, *pc, 0, 1);

		if (_debug_commands_not_hooked) {
			_debug_commands_not_hooked = 0;

			con_hook_command(c_sfx_remove, "sfx_remove", "!a", "Kills a playing sound.");
			con_hook_command(c_vmvars, "vmvars", "!sia*", "Displays or changes variables in the VM\n\nFirst parameter is either g(lobal), l(ocal), t(emp) or p(aram).\nSecond parameter is the var number\nThird parameter (if specified) is the value to set the variable to");
			con_hook_command(c_step, "s", "i*", "Executes one or several operations\n\nEXAMPLES\n\n"
			                 "    s 4\n\n  Execute 4 commands\n\n    s\n\n  Execute next command");
#if 0
			// TODO Re-implement con:so
			con_hook_command(c_stepover, "so", "", "Executes one operation skipping over sends");
#endif
			con_hook_command(c_disasm_addr, "disasm-addr", "!as*", "Disassembles one or more commands\n\n"
			                 "USAGE\n\n  disasm-addr [startaddr] <options>\n\n"
			                 "  Valid options are:\n"
			                 "  bwt  : Print byte/word tag\n"
			                 "  c<x> : Disassemble <x> bytes\n"
			                 "  bc   : Print bytecode\n\n");
			con_hook_command(c_disasm, "disasm", "!as", "Disassembles a method by name\n\nUSAGE\n\n  disasm <obj> <method>\n\n");
			con_hook_command(c_obj, "obj", "!", "Displays information about the\n  currently active object/class.\n"
			                 "\n\nSEE ALSO\n\n  vo.1, accobj.1");
			con_hook_command(c_accobj, "accobj", "!", "Displays information about an\n  object or class at the\n"
			                 "address indexed by acc.\n\nSEE ALSO\n\n  obj.1, vo.1");
			con_hook_command(c_backtrace, "bt", "", "Dumps the send/self/super/call/calle/callb stack");
			con_hook_command(c_snk, "snk", "s*", "Steps forward until it hits the next\n  callk operation.\n"
			                 "  If invoked with a parameter, it will\n  look for that specific callk.\n");
			con_hook_command(c_se, "se", "", "Steps forward until an SCI event is received.\n");
			con_hook_command(c_set_acc, "set_acc", "!a", "Sets the accumulator");
			con_hook_command(c_send, "send", "!asa*", "Sends a message to an object\nExample: send ?fooScript cue");
			con_hook_command(c_sret, "sret", "", "Steps forward until ret is called\n  on the current execution stack\n  level.");
			con_hook_command(c_bpx, "bpx", "s", "Sets a breakpoint on the execution of\n  the specified method.\n\n  EXAMPLE:\n"
			                 "  bpx ego::doit\n\n  May also be used to set a breakpoint\n  that applies whenever an object\n"
			                 "  of a specific type is touched:\n  bpx foo::\n");
			con_hook_command(c_bpe, "bpe", "ii", "Sets a breakpoint on the execution of specified exported function.\n");
			con_hook_command(c_bplist, "bplist", "", "Lists all breakpoints.\n");
			con_hook_command(c_bpdel, "bpdel", "i", "Deletes a breakpoint with specified index.");
			con_hook_command(c_go, "go", "", "Executes the script.\n");
			con_hook_command(c_parse, "parse", "s", "Parses a sequence of words and prints\n  the resulting parse tree.\n"
			                 "  The word sequence must be provided as a\n  single string.");
			con_hook_command(c_set_parse_nodes, "set_parse_nodes", "s*", "Sets the contents of all parse nodes.\n"
			                 "  Input token must be separated by\n  blanks.");

#ifdef GFXW_DEBUG_WIDGETS
			con_hook_command(c_gfx_print_widget, "gfx_print_widget", "i*", "If called with no parameters, it\n  shows which widgets are active.\n"
			                 "  With parameters, it lists the\n  widget corresponding to the\n  numerical index specified (for\n  each parameter).");
#endif

#if 0
			// TODO: Re-enable con:draw_viewobj
			con_hook_command(c_gfx_draw_viewobj, "draw_viewobj", "i", "Draws the nsRect and brRect of a\n  dynview object.\n\n  nsRect is green, brRect\n"
			                 "  is blue.\n");
#endif
			con_hook_command(c_vo, "vo", "!a",
			                 "Examines an object\n\n"
			                 "SEE ALSO\n\n"
			                 "  addresses.3, type.1");
			con_hook_command(c_vr, "vr", "!aa*",
			                 "Examines an arbitrary reference\n\n");
			con_hook_command(c_sg, "sg", "!i",
			                 "Steps until the global variable with the\n"
			                 "specified index is modified.\n\nSEE ALSO\n\n"
			                 "  s.1, snk.1, so.1, bpx.1");
			con_hook_command(c_type, "type", "!a",
			                 "Determines the type of a value\n\n"
			                 "SEE ALSO\n\n  addresses.3, vo.1");
			con_hook_command(c_shownode, "shownode", "!a",
			                 "Prints information about a list node\n"
			                 "  or list base.\n\n");
			con_hook_command(c_gc_show_reachable, "gc-list-reachable", "!a",
			                 "Prints all addresses directly reachable from\n"
			                 "  the memory object specified as parameter.\n\n"
			                 "SEE ALSO\n\n"
			                 "  gc-list-freeable.1, gc-normalise.1, gc.1,\n"
			                 "  gc-all-reachable.1");
			con_hook_command(c_gc_show_freeable, "gc-list-freeable", "!a",
			                 "Prints all addresses freeable in the segment\n"
			                 "  associated with the address (offset is ignored).\n\n"
			                 "SEE ALSO\n\n"
			                 "  gc-list-freeable.1, gc-normalise.1, gc.1,\n"
			                 "  gc-all-reachable.1");
			con_hook_command(c_gc_normalise, "gc-normalise", "!a",
			                 "Prints the \"normal\" address of a given address,\n"
			                 "  i.e. the address we would free in order to free\n"
			                 "  the object associated with the original address.\n\n"
			                 "SEE ALSO\n\n"
			                 "  gc-list-freeable.1, gc-list-reachable.1, gc.1,\n"
			                 "  gc-all-reachable.1");

/*
			con_hook_int(&script_abort_flag, "script_abort_flag", "Set != 0 to abort execution\n");
*/
		} // If commands were not hooked up
	}

	if (_debug_step_running)
		_debug_step_running--;

	while (_debugstate_valid) {
		int skipfirst = 0;
		const char *commandstring;

		// Suspend music playing
		s->_sound.sfx_suspend(true);

		commandstring = _debug_get_input();

		// Check if a specific destination has been given
		if (commandstring && (commandstring[0] == '.' || commandstring[0] == ':'))
			skipfirst = 1;

		//if (commandstring && commandstring[0] != ':')
		//	con_parse(s, commandstring + skipfirst);
		sciprintf("\n");

		// Resume music playing
		s->_sound.sfx_suspend(false);
	}
}

} // End of namespace Sci
