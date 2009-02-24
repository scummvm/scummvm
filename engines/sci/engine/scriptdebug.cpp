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

#include "sci/engine/gc.h"
#include "sci/include/sciresource.h"
#include "sci/include/engine.h"
#include "sci/scicore/sciconsole.h"
#include "sci/engine/kdebug.h"
#include "sci/include/vocabulary.h"
#include "sci/engine/kernel.h"
#include "sci/engine/kernel_types.h"
#include "sci/include/sci_midi.h"
#include "sci/include/sci_widgets.h"
#include "sci/sci.h"
#include "sci/gfx/gfx_widgets.h"

#include "common/util.h"
#include "common/savefile.h"

namespace Sci {

extern int debug_sleeptime_factor;
int _debugstate_valid = 0; // Set to 1 while script_debug is running
int _debug_step_running = 0; // Set to >0 to allow multiple stepping
int _debug_commands_not_hooked = 1; // Commands not hooked to the console yet?
int _debug_seeking = 0; // Stepping forward until some special condition is met
int _debug_seek_level = 0; // Used for seekers that want to check their exec stack depth
int _debug_seek_special = 0;  // Used for special seeks(1)
int _weak_validations = 1; // Some validation errors are reduced to warnings if non-0
reg_t _debug_seek_reg = NULL_REG;  // Used for special seeks(2)

#define _DEBUG_SEEK_NOTHING 0
#define _DEBUG_SEEK_CALLK 1 // Step forward until callk is found
#define _DEBUG_SEEK_LEVEL_RET 2 // Step forward until returned from this level
#define _DEBUG_SEEK_SPECIAL_CALLK 3 // Step forward until a /special/ callk is found
#define _DEBUG_SEEK_SO 5 // Step forward until specified PC (after the send command) and stack depth
#define _DEBUG_SEEK_GLOBAL 6 // Step forward until one specified global variable is modified

static reg_t *p_pc;
static stack_ptr_t *p_sp;
static stack_ptr_t *p_pp;
static reg_t *p_objp;
static int *p_restadjust;
static seg_id_t *p_var_segs;
static reg_t **p_vars;
static reg_t **p_var_base;
static int *p_var_max; // May be NULL even in valid state!

static const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0};

int _kdebug_cheap_event_hack = 0;
int _kdebug_cheap_soundcue_hack = -1;

char inputbuf[256] = "";

#define LOOKUP_SPECIES(species) (\
	(species >= 1000) ? species : *(s->classtable[species].scriptposp) \
		+ s->classtable[species].class_offset)

const char *_debug_get_input_default() {
	char newinpbuf[256];

	printf("> ");
	fgets(newinpbuf, 254, stdin);

	if (strlen(newinpbuf) != 0)
		memcpy(inputbuf, newinpbuf, 256);

	return inputbuf;
}

static inline int _parse_ticks(byte *data, int *offset_p, int size) {
	int ticks = 0;
	int tempticks;
	int offset = 0;

	do {
		tempticks = data[offset++];
		ticks += (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX) ? SCI_MIDI_TIME_EXPANSION_LENGTH : tempticks;
	} while (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX && offset < size);

	if (offset_p)
		*offset_p = offset;

	return ticks;
}

static void midi_hexdump(byte *data, int size, int notational_offset) { // Specialised for SCI01 tracks (this affects the way cumulative cues are treated )
	int offset = 0;
	int prev = 0;

	if (*data == 0xf0) // SCI1 priority spec
		offset = 8;

	while (offset < size) {
		int old_offset = offset;
		int offset_mod;
		int time = _parse_ticks(data + offset, &offset_mod, size);
		int cmd;
		int pleft;
		int firstarg = 0;
		int i;
		int blanks = 0;

		offset += offset_mod;
		fprintf(stderr, "  [%04x] %d\t",
		        old_offset + notational_offset, time);

		cmd = data[offset];
		if (!(cmd & 0x80)) {
			cmd = prev;
			if (prev < 0x80) {
				fprintf(stderr, "Track broken at %x after"
				        " offset mod of %d\n",
				        offset + notational_offset, offset_mod);
				sci_hexdump(data, size, notational_offset);
				return;
			}
			fprintf(stderr, "(rs %02x) ", cmd);
			blanks += 8;
		} else {
			++offset;
			fprintf(stderr, "%02x ", cmd);
			blanks += 3;
		}
		prev = cmd;

		pleft = MIDI_cmdlen[cmd >> 4];
		if (SCI_MIDI_CONTROLLER(cmd) && data[offset] == SCI_MIDI_CUMULATIVE_CUE)
			--pleft; // This is SCI(0)1 specific

		for (i = 0; i < pleft; i++) {
			if (i == 0)
				firstarg = data[offset];
			fprintf(stderr, "%02x ", data[offset++]);
			blanks += 3;
		}

		while (blanks < 16) {
			blanks += 4;
			fprintf(stderr, "    ");
		}

		while (blanks < 20) {
			++blanks;
			fprintf(stderr, " ");
		}

		if (cmd == SCI_MIDI_EOT)
			fprintf(stderr, ";; EOT");
		else if (cmd == SCI_MIDI_SET_SIGNAL) {
			if (firstarg == SCI_MIDI_SET_SIGNAL_LOOP)
				fprintf(stderr, ";; LOOP point");
			else
				fprintf(stderr, ";; CUE (%d)", firstarg);
		} else if (SCI_MIDI_CONTROLLER(cmd)) {
			if (firstarg == SCI_MIDI_CUMULATIVE_CUE)
				fprintf(stderr, ";; CUE (cumulative)");
			else if (firstarg == SCI_MIDI_RESET_ON_SUSPEND)
				fprintf(stderr, ";; RESET-ON-SUSPEND flag");
		}
		fprintf(stderr, "\n");

		if (old_offset >= offset) {
			fprintf(stderr, "-- Not moving forward anymore,"
			        " aborting (%x/%x)\n", offset, old_offset);
			return;
		}
	}
}

#define SONGDATA(x) data[offset + (x)]
#define CHECK_FOR_END_ABSOLUTE(off) if ((off) >= size) return;

static void sci01_song_header_dump(byte *data, int size) {
	int offset = 0;
	int smallest_start = 10000;

	sciprintf("SCI01 song track mappings:\n");

	if (*data == 0xf0) // SCI1 priority spec
		offset = 8;

	CHECK_FOR_END_ABSOLUTE(0);
	while (SONGDATA(0) != 0xff) {
		byte device_id = data[offset];
		sciprintf("* Device %02x:\n", device_id);
		offset++;
		CHECK_FOR_END_ABSOLUTE(offset + 1);
		while (SONGDATA(0) != 0xff) {
			int track_offset;
			int end;
			byte header1, header2;

			CHECK_FOR_END_ABSOLUTE(offset + 7);

			offset += 2;

			track_offset = getUInt16(data + offset);
			header1 = data[track_offset];
			header2 = data[track_offset+1];
			track_offset += 2;

			if (track_offset < smallest_start)
				smallest_start = track_offset;

			end = getUInt16(data + offset + 2);
			sciprintf("  - %04x -- %04x", track_offset, track_offset + end);

			if (track_offset == 0xfe)
				sciprintf(" (PCM data)\n");
			else
				sciprintf(" (channel %d, special %d, %d playing notes, %d foo)\n",
				          header1 & 0xf, header1 >> 4, header2 & 0xf, header2 >> 4);
			offset += 4;
		}
		offset++;
	}
}
#undef CHECK_FOR_END_ABSOLUTE
#undef SONGDATA

int c_sfx_01_header(EngineState *s) {
	resource_t *song = scir_find_resource(s->resmgr, sci_sound, cmd_params[0].val, 0);

	if (!song) {
		sciprintf("Doesn't exist\n");
		return 1;
	}

	sci01_song_header_dump(song->data, song->size);

	return 0;
}

int c_sfx_01_track(EngineState *s) {
	resource_t *song = scir_find_resource(s->resmgr, sci_sound, cmd_params[0].val, 0);

	int offset = cmd_params[1].val;

	if (!song) {
		sciprintf("Doesn't exist\n");
		return 1;
	}

	midi_hexdump(song->data + offset, song->size, offset);

	return 0;
}

const char *(*_debug_get_input)(void) = _debug_get_input_default;

int c_segtable(EngineState *s) {
	int i;

	sciprintf("  ---- segment table ----\n");
	for (i = 0; i < s->seg_manager->heap_size; i++) {
		mem_obj_t *mobj = s->seg_manager->heap[i];
		if (mobj && mobj->type) {
			sciprintf(" [%04x] ", i);

			switch (mobj->type) {
			case MEM_OBJ_SCRIPT:
				sciprintf("S  script.%03d l:%d ", mobj->data.script.nr, mobj->data.script.lockers);
				break;

			case MEM_OBJ_CLONES:
				sciprintf("C  clones (%d allocd)", mobj->data.clones.entries_used);
				break;

			case MEM_OBJ_LOCALS:
				sciprintf("V  locals %03d", mobj->data.locals.script_id);
				break;

			case MEM_OBJ_STACK:
				sciprintf("D  data stack (%d)", mobj->data.stack.nr);
				break;

			case MEM_OBJ_SYS_STRINGS:
				sciprintf("Y  system string table");
				break;

			case MEM_OBJ_LISTS:
				sciprintf("L  lists (%d)", mobj->data.lists.entries_used);
				break;

			case MEM_OBJ_NODES:
				sciprintf("N  nodes (%d)", mobj->data.nodes.entries_used);
				break;

			case MEM_OBJ_HUNK:
				sciprintf("H  hunk (%d)", mobj->data.hunks.entries_used);
				break;

			case MEM_OBJ_DYNMEM:
				sciprintf("M  dynmem: %d bytes", mobj->data.dynmem.size);
				break;

			default:
				sciprintf("I  Invalid (type = %x)", mobj->type);
				break;
			}

			sciprintf("  seg_ID = %d \n", mobj->segmgr_id);
		}
	}
	sciprintf("\n");

	return 0;
}

static void print_obj_head(EngineState *s, object_t *obj) {
	sciprintf(PREG" %s : %3d vars, %3d methods\n", PRINT_REG(obj->pos), obj_get_name(s, obj->pos),
				obj->variables_nr, obj->methods_nr);
}

static void print_list(EngineState *s, list_t *l) {
	reg_t pos = l->first;
	reg_t my_prev = NULL_REG;

	sciprintf("\t<\n");

	while (!IS_NULL_REG(pos)) {
		node_t *node;
		mem_obj_t *mobj = GET_SEGMENT(*s->seg_manager, pos.segment, MEM_OBJ_NODES);

		if (!mobj || !ENTRY_IS_VALID(&(mobj->data.nodes), pos.offset)) {
			sciprintf("   WARNING: "PREG": Doesn't contain list node!\n",
			          PRINT_REG(pos));
			return;
		}

		node = &(mobj->data.nodes.table[pos.offset].entry);

		sciprintf("\t"PREG"  : "PREG" -> "PREG"\n", PRINT_REG(pos), PRINT_REG(node->key), PRINT_REG(node->value));

		if (!REG_EQ(my_prev, node->pred))
			sciprintf("   WARNING: current node gives "PREG" as predecessor!\n",
			          PRINT_REG(node->pred));

		my_prev = pos;
		pos = node->succ;
	}

	if (!REG_EQ(my_prev, l->last))
		sciprintf("   WARNING: Last node was expected to be "PREG", was "PREG"!\n",
		          PRINT_REG(l->last), PRINT_REG(my_prev));
	sciprintf("\t>\n");
}

static void _c_single_seg_info(EngineState *s, mem_obj_t *mobj) {
	switch (mobj->type) {

	case MEM_OBJ_SCRIPT: {
		int i;
		script_t *scr = &(mobj->data.script);
		sciprintf("script.%03d locked by %d, bufsize=%d (%x)\n", scr->nr, scr->lockers, (uint)scr->buf_size, (uint)scr->buf_size);
		if (scr->export_table)
			sciprintf("  Exports: %4d at %d\n", scr->exports_nr, (int)(((byte *)scr->export_table) - ((byte *)scr->buf)));
		else
			sciprintf("  Exports: none\n");

		sciprintf("  Synynms: %4d\n", scr->synonyms_nr);

		if (scr->locals_block)
			sciprintf("  Locals : %4d in segment 0x%x\n", scr->locals_block->nr, scr->locals_segment);
		else
			sciprintf("  Locals : none\n");

		sciprintf("  Objects: %4d\n", scr->objects_nr);
		for (i = 0; i < scr->objects_nr; i++) {
			sciprintf("    ");
			print_obj_head(s, scr->objects + i);
		}
	}
	break;

	case MEM_OBJ_LOCALS: {
		local_variables_t *locals = &(mobj->data.locals);
		sciprintf("locals for script.%03d\n", locals->script_id);
		sciprintf("  %d (0x%x) locals\n", locals->nr, locals->nr);
	}
	break;

	case MEM_OBJ_STACK: {
		dstack_t *stack = &(mobj->data.stack);
		sciprintf("stack\n");
		sciprintf("  %d (0x%x) entries\n", stack->nr, stack->nr);
	}
	break;

	case MEM_OBJ_SYS_STRINGS: {
		sys_strings_t *strings = &(mobj->data.sys_strings);
		int i;

		sciprintf("system string table\n");
		for (i = 0; i < SYS_STRINGS_MAX; i++)
			if (strings->strings[i].name)
				sciprintf("  %s[%d]=\"%s\"\n", strings->strings[i].name, strings->strings[i].max_size, strings->strings[i].value);
	}
	break;

	case MEM_OBJ_CLONES: {
		int i = 0;
		clone_table_t *ct = &(mobj->data.clones);

		sciprintf("clones\n");

		for (i = 0; i < ct->max_entry; i++)
			if (ENTRY_IS_VALID(ct, i)) {
				sciprintf("  [%04x] ", i);
				print_obj_head(s, &(ct->table[i].entry));
			}
	}
	break;

	case MEM_OBJ_LISTS: {
		int i = 0;
		list_table_t *lt = &(mobj->data.lists);

		sciprintf("lists\n");
		for (i = 0; i < lt->max_entry; i++)
			if (ENTRY_IS_VALID(lt, i)) {
				sciprintf("  [%04x]: ", i);
				print_list(s, &(lt->table[i].entry));
			}
	}
	break;

	case MEM_OBJ_NODES: {
		sciprintf("nodes (total %d)\n", mobj->data.nodes.entries_used);
		break;
	}

	case MEM_OBJ_HUNK: {
		int i;
		hunk_table_t *ht = &(mobj->data.hunks);

		sciprintf("hunk  (total %d)\n", mobj->data.hunks.entries_used);
		for (i = 0; i < ht->max_entry; i++)
			if (ENTRY_IS_VALID(ht, i)) {
				sciprintf("    [%04x] %d bytes at %p, type=%s\n",
				          i, ht->table[i].entry.size, ht->table[i].entry.mem, ht->table[i].entry.type);
			}
	}

	case MEM_OBJ_DYNMEM: {
		sciprintf("dynmem (%s): %d bytes\n",
		          mobj->data.dynmem.description ? mobj->data.dynmem.description : "no description", mobj->data.dynmem.size);

		sci_hexdump(mobj->data.dynmem.buf, mobj->data.dynmem.size, 0);
	}
	break;

	default :
		sciprintf("Invalid type %d\n", mobj->type);
		break;
	}
}

static int show_node(EngineState *s, reg_t addr) {
	mem_obj_t *mobj = GET_SEGMENT(*s->seg_manager, addr.segment, MEM_OBJ_LISTS);

	if (mobj) {
		list_table_t *lt = &(mobj->data.lists);
		list_t *list;

		if (!ENTRY_IS_VALID(lt, addr.offset)) {
			sciprintf("Address does not contain a list\n");
			return 1;
		}

		list = &(lt->table[addr.offset].entry);

		sciprintf(PREG" : first x last = ("PREG", "PREG")\n", PRINT_REG(addr), PRINT_REG(list->first), PRINT_REG(list->last));
	} else {
		node_table_t *nt;
		node_t *node;
		mobj = GET_SEGMENT(*s->seg_manager, addr.segment, MEM_OBJ_NODES);

		if (!mobj) {
			sciprintf("Segment #%04x is not a list or node segment\n", addr.segment);
			return 1;
		}

		nt = &(mobj->data.nodes);

		if (!ENTRY_IS_VALID(nt, addr.offset)) {
			sciprintf("Address does not contain a node\n");
			return 1;
		}
		node = &(nt->table[addr.offset].entry);

		sciprintf(PREG" : prev x next = ("PREG", "PREG"); maps "PREG" -> "PREG"\n",
		          PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(node->succ), PRINT_REG(node->key), PRINT_REG(node->value));
	}

	return 0;
}

int objinfo(EngineState *s, reg_t pos);

void song_lib_dump(songlib_t songlib, int line);

static int c_songlib_print(EngineState *s) {
	song_lib_dump(s->sound.songlib, __LINE__);

	return 0;
}

static int c_vr(EngineState *s) {
	reg_t reg = cmd_params[0].reg;
	reg_t reg_end = cmd_paramlength > 1 ? cmd_params[1].reg : NULL_REG;
	int type_mask = determine_reg_type(s, reg, 1);
	int filter;
	int found = 0;

	sciprintf(PREG" is of type 0x%x%s: ", PRINT_REG(reg), type_mask & ~KSIG_INVALID, type_mask & KSIG_INVALID ? " (invalid)" : "");

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
			list_t *l = LOOKUP_LIST(reg);

			sciprintf("list\n");

			if (l)
				print_list(s, l);
			else
				sciprintf("Invalid list.\n");
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

			sci_hexdump(block, size, 0);
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

int c_segkill(EngineState *s) {
	unsigned int i = 0;

	while (i < cmd_paramlength) {
		int nr = cmd_params[i++].val;

		s->seg_manager->setLockers(nr, 0, SEG_ID);
	}
	return 0;
}

static int c_mousepos(EngineState *s) {
	sci_event_t event;

	sciprintf("Click somewhere in the game window...\n");

	while (event = gfxop_get_event(s->gfx_state, SCI_EVT_MOUSE_RELEASE), event.type != SCI_EVT_MOUSE_RELEASE) {};

	sciprintf("Mouse pointer at (%d, %d)\n", s->gfx_state->pointer_pos.x, s->gfx_state->pointer_pos.y);

	return 0;
}

int c_seginfo(EngineState *s) {
	unsigned int i = 0;

	if (cmd_paramlength) {
		while (i < cmd_paramlength) {
			int nr = cmd_params[i++].val;
			if (nr < 0 || nr >= s->seg_manager->heap_size || !s->seg_manager->heap[nr]) {
				sciprintf("Segment %04x does not exist\n", nr);
				return 1;
			}
			sciprintf("[%04x] ", nr);
			_c_single_seg_info(s, s->seg_manager->heap[nr]);
		}
	} else
		for (i = 0; i < (unsigned int)s->seg_manager->heap_size; i++) {
			if (s->seg_manager->heap[i]) {
				sciprintf("[%04x] ", i);
				_c_single_seg_info(s, s->seg_manager->heap[i]);
				sciprintf("\n");
			}
		}

	return 0;
}

int c_debuginfo(EngineState *s) {
	exec_stack_t *eframe = NULL;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (s->execution_stack && s->execution_stack_pos >= 0)
		eframe = s->execution_stack + s->execution_stack_pos;

	sciprintf("acc="PREG" prev="PREG" &rest=%x\n", PRINT_REG(s->r_acc), PRINT_REG(s->r_prev), *p_restadjust);

	if (eframe)
		sciprintf("pc="PREG" obj="PREG" fp="PSTK" sp="PSTK"\n", PRINT_REG(*p_pc), PRINT_REG(*p_objp), PRINT_STK(*p_pp), PRINT_STK(*p_sp));
	else
		sciprintf("<no execution stack: pc,obj,fp omitted>\n");

	return 0;
}

int c_step(EngineState *s) {
	_debugstate_valid = 0;
	if (cmd_paramlength && (cmd_params[0].val > 0))
		_debug_step_running = cmd_params[0].val - 1;

	return 0;
}

#if 0
// TODO Re-implement con:so
int c_stepover(EngineState *s) {
	int opcode, opnumber;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	_debugstate_valid = 0;
	opcode = s->heap [*p_pc];
	opnumber = opcode >> 1;
	if (opnumber == 0x22 /* callb */ || opnumber == 0x23 /* calle */ ||
	        opnumber == 0x25 /* send */ || opnumber == 0x2a /* self */ || opnumber == 0x2b /* super */) {
		_debug_seeking = _DEBUG_SEEK_SO;
		_debug_seek_level = s->execution_stack_pos;
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

int c_sim_parse(EngineState *s) {
	unsigned int i;
	const char *operators = ",&/()[]#<>";

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmd_paramlength == 0) {
		s->parser_valid = 0;
		return 0;
	}

	for (i = 0; i < cmd_paramlength; i++) {
		int flag = 0;
		char *token = cmd_params[i].str;

		if (strlen(token) == 1) {// could be an operator
			int j = 0;
			while (operators[j] && (operators[j] != token[0]))
				j++;
			if (operators[j]) {
				s->parser_nodes[i].type = 1;
				s->parser_nodes[i].content.value = j + 0xf0;
				flag = 1; // found an operator
			}
		}

		if (!flag) {
			char *openb = strchr(token, '['); // look for opening braces
			result_word_t *result;

			if (openb)
				*openb = 0; // remove them and the rest

			result = vocab_lookup_word(token, strlen(token), s->parser_words, s->parser_words_nr, s->parser_suffices, s->parser_suffices_nr);

			if (result) {
				s->parser_nodes[i].type = 0;
				s->parser_nodes[i].content.value = result->group;
				free(result);
			} else { // group name was specified directly?
				int val = strtol(token, NULL, 0);
				if (val) {
					s->parser_nodes[i].type = 0;
					s->parser_nodes[i].content.value = val;
				} else { // invalid and not matched
					sciprintf("Couldn't interpret '%s'\n", token);
					s->parser_valid = 0;
					return 1;
				}
			}
		}

	}

	s->parser_nodes[cmd_paramlength].type = -1; // terminate

	s->parser_valid = 2;

	return 0;
}

int c_classtable(EngineState *s) {
	int i;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	sciprintf("Available classes:\n");
	for (i = 0; i < s->classtable_size; i++)
		if (s->classtable[i].reg.segment)
			sciprintf(" Class 0x%x at "PREG" (script 0x%x)\n", i, PRINT_REG(s->classtable[i].reg), s->classtable[i].script);

	return 0;
}

int c_viewinfo(EngineState *s) {
	int view = cmd_params[0].val;
	int palette = cmd_params[1].val;
	int loops, i;
	gfxr_view_t *view_pixmaps = NULL;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}
	sciprintf("Resource view.%d ", view);

	loops = gfxop_lookup_view_get_loops(s->gfx_state, view);

	if (loops < 0)
		sciprintf("does not exist.\n");
	else {
		sciprintf("has %d loops:\n", loops);

		for (i = 0; i < loops; i++) {
			int j, cels;

			sciprintf("Loop %d: %d cels.\n", i, cels = gfxop_lookup_view_get_cels(s->gfx_state, view, i));
			for (j = 0; j < cels; j++) {
				int width;
				int height;
				Common::Point mod;

				if (con_can_handle_pixmaps()) {
					view_pixmaps = gfxr_get_view(s->gfx_state->resstate, view, &i, &j, palette);
					con_insert_pixmap(gfx_clone_pixmap(view_pixmaps->loops[i].cels[j], s->gfx_state->driver->mode));
				}

				gfxop_get_cel_parameters(s->gfx_state, view, i, j, &width, &height, &mod);

				sciprintf("   cel %d: size %dx%d, adj+(%d,%d)\n", j, width, height, mod.x, mod.y);
			}
		}
	}

	return 0;
}

int c_list_sentence_fragments(EngineState *s) {
	int i;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	for (i = 0; i < s->parser_branches_nr; i++) {
		int j = 0;

		sciprintf("R%02d: [%x] ->", i, s->parser_branches[i].id);
		while ((j < 10) && s->parser_branches[i].data[j]) {
			int dat = s->parser_branches[i].data[j++];

			switch (dat) {
			case VOCAB_TREE_NODE_COMPARE_TYPE:
				dat = s->parser_branches[i].data[j++];
				sciprintf(" C(%x)", dat);
				break;

			case VOCAB_TREE_NODE_COMPARE_GROUP:
				dat = s->parser_branches[i].data[j++];
				sciprintf(" WG(%x)", dat);
				break;

			case VOCAB_TREE_NODE_FORCE_STORAGE:
				dat = s->parser_branches[i].data[j++];
				sciprintf(" FORCE(%x)", dat);
				break;

			default:
				if (dat > VOCAB_TREE_NODE_LAST_WORD_STORAGE) {
					int dat2 = s->parser_branches[i].data[j++];
					sciprintf(" %x[%x]", dat, dat2);
				} else
					sciprintf(" ?%x?", dat);
			}
		}
		sciprintf("\n");
	}

	sciprintf("%d rules.\n", s->parser_branches_nr);

	return 0;
}

enum {
	_parse_eoi,
	_parse_token_pareno,
	_parse_token_parenc,
	_parse_token_nil,
	_parse_token_number
};

int _parse_getinp(int *i, int *nr) {
	char *token;

	if ((unsigned)*i == cmd_paramlength)
		return _parse_eoi;

	token = cmd_params[(*i)++].str;

	if (!strcmp(token, "("))
		return _parse_token_pareno;

	if (!strcmp(token, ")"))
		return _parse_token_parenc;

	if (!strcmp(token, "nil"))
		return _parse_token_nil;

	*nr = strtol(token, NULL, 0);

	return _parse_token_number;
}

int _parse_nodes(EngineState *s, int *i, int *pos, int type, int nr) {
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

	nexttk = _parse_getinp(i, &nextval);
	if ((newpos = s->parser_nodes[oldpos].content.branches[0] = _parse_nodes(s, i, pos, nexttk, nextval)) == -1)
		return -1;

	nexttk = _parse_getinp(i, &nextval);
	if ((newpos = s->parser_nodes[oldpos].content.branches[1] = _parse_nodes(s, i, pos, nexttk, nextval)) == -1)
		return -1;

	if (_parse_getinp(i, &nextval) != _parse_token_parenc)
		sciprintf("Expected ')' at token %d\n", *i);

	return oldpos;
}

int c_set_parse_nodes(EngineState *s) {
	int i = 0;
	int foo, bar;
	int pos = -1;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	bar = _parse_getinp(&i, &foo);
	if (_parse_nodes(s, &i, &pos, bar, foo) == -1)
		return 1;

	vocab_dump_parse_tree("debug-parse-tree", s->parser_nodes);
	return 0;
}

int vocab_gnf_parse(parse_tree_node_t *nodes, result_word_t *words, int words_nr,
					parse_tree_branch_t *branch0, parse_rule_list_t *tlist, int verbose);
// parses with a GNF rule set

int c_parse(EngineState *s) {
	result_word_t *words;
	int words_nr;
	char *error;
	char *string;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	string = cmd_params[0].str;
	sciprintf("Parsing '%s'\n", string);
	words = vocab_tokenize_string(string, &words_nr, s->parser_words, s->parser_words_nr,
	                              s->parser_suffices, s->parser_suffices_nr, &error);
	if (words) {
		int i, syntax_fail = 0;

		vocab_synonymize_tokens(words, words_nr, s->synonyms, s->synonyms_nr);

		sciprintf("Parsed to the following blocks:\n");

		for (i = 0; i < words_nr; i++)
			sciprintf("   Type[%04x] Group[%04x]\n", words[i].w_class, words[i].group);

		if (vocab_gnf_parse(&(s->parser_nodes[0]), words, words_nr, s->parser_branches, s->parser_rules, 1))
			syntax_fail = 1; // Building a tree failed

		free(words);

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

int c_save_game(EngineState *s) {
	int omit_check = cmd_params[0].str[0] == '_';

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (!omit_check) {
		int result = 0;
		for (uint i = 0; i < s->_fileHandles.size(); i++)
			if (s->_fileHandles[i]._file)
				result++;

		if (result) {
			sciprintf("Game state has %d open file handles.\n", result);
			sciprintf("Save to '_%s' to ignore this check.\nGame was NOT saved.\n", cmd_params[0].str);
			return 1;
		}
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out;
	if (!(out = saveFileMan->openForSaving(cmd_params[0].str))) {
		sciprintf("Error opening savegame \"%s\" for writing\n", cmd_params[0].str);
		return 0;
	}

	// TODO: enable custom descriptions? force filename into a specific format?
	if (gamestate_save(s, out, "debugging")) {
		sciprintf("Saving the game state to '%s' failed\n", cmd_params[0].str);
	}

	return 0;
}

int c_restore_game(EngineState *s) {
	EngineState *newstate = NULL;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in;
	if (!(in = saveFileMan->openForLoading(cmd_params[0].str))) {
		// found a savegame file
		newstate = gamestate_restore(s, in);
		delete in;
	}

	if (newstate) {
		s->successor = newstate; // Set successor

		script_abort_flag = SCRIPT_ABORT_WITH_REPLAY; // Abort current game
		_debugstate_valid = 0;
		s->execution_stack_pos = s->execution_stack_base;
		return 0;
	} else {
		sciprintf("Restoring gamestate '%s' failed.\n", cmd_params[0].str);
		return 1;
	}
}

extern char *old_save_dir;

int c_restart_game(EngineState *s) {
	unsigned int i;
	char *deref_save_dir = (char*)kernel_dereference_bulk_pointer(s, s->save_dir_copy, 1);

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	old_save_dir = strdup(deref_save_dir);
	for (i = 0; i < cmd_paramlength; i++) {
		if ((strcmp(cmd_params[0].str, "-r") == 0) || (strcmp(cmd_params[0].str, "--replay") == 0))
			s->restarting_flags |= SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
		else
			if ((strcmp(cmd_params[0].str, "-p") == 0) || (strcmp(cmd_params[0].str, "--play") == 0))
				s->restarting_flags &= ~SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
			else {
				sciprintf("Invalid parameter '%s'\n", cmd_params[0].str);
				return 1;
			}
	}

	sciprintf("Restarting\n");

	s->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;

	script_abort_flag = 1;
	_debugstate_valid = 0;

	return 0;
}

int c_stack(EngineState *s) {
	int i;
	exec_stack_t *xs;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (s->execution_stack_pos >= 0)
		xs = s->execution_stack + s->execution_stack_pos;
	else {
		sciprintf("No exec stack!");
		return 1;
	}

	for (i = cmd_params[0].val ; i > 0; i--) {
		if ((xs->sp - xs->fp - i) == 0)
			sciprintf("-- temp variables --\n");
		if (xs->sp - i >= s->stack_base)
			sciprintf(PSTK" = "PREG"\n", PRINT_STK(xs->sp - i), PRINT_REG(xs->sp[-i]));
	}

	return 0;
}

const char *selector_name(EngineState *s, int selector) {
	if (selector >= 0 && selector < (int)s->_selectorNames.size())
		return s->_selectorNames[selector].c_str();
	else
		return "--INVALID--";
}

int prop_ofs_to_id(EngineState *s, int prop_ofs, reg_t objp) {
	object_t *obj = obj_get(s, objp);
	byte *selectoroffset;
	int selectors;

	if (!obj) {
		sciprintf("Applied prop_ofs_to_id on non-object at "PREG"\n", PRINT_REG(objp));
		return -1;
	}

	selectors = obj->variables_nr;

	if (s->version < SCI_VERSION(1, 001, 000))
		selectoroffset = ((byte *)(obj->base_obj)) + SCRIPT_SELECTOR_OFFSET + selectors * 2;
	else {
		if (!(obj->variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS)) {
			obj = obj_get(s, obj->variables[SCRIPT_SUPERCLASS_SELECTOR]);
			selectoroffset = (byte *)obj->base_vars;
		} else
			selectoroffset = (byte *)obj->base_vars;
	}

	if (prop_ofs < 0 || (prop_ofs >> 1) >= selectors) {
		sciprintf("Applied prop_ofs_to_id to invalid property offset %x (property #%d not in [0..%d]) on object at "PREG"\n",
		          prop_ofs, prop_ofs >> 1, selectors - 1, PRINT_REG(objp));
		return -1;
	}

	return getUInt16(selectoroffset + prop_ofs);
}

reg_t disassemble(EngineState *s, reg_t pos, int print_bw_tag, int print_bytecode) {
// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
	mem_obj_t *memobj = GET_SEGMENT(*s->seg_manager, pos.segment, MEM_OBJ_SCRIPT);
	script_t *script_entity = NULL;
	byte *scr;
	int scr_size;
	reg_t retval = make_reg(pos.segment, pos.offset + 1);
	uint16 param_value;
	int opsize;
	int opcode;
	int bytecount = 1;
	int i = 0;

	if (!memobj) {
		sciprintf("Disassembly failed: Segment %04x non-existant or not a script\n", pos.segment);
		return retval;
	} else
		script_entity = &(memobj->data.script);

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

	sciprintf(PREG": ", PRINT_REG(pos));

	if (print_bytecode) {
		while (formats[opcode][i]) {
			switch (formats[opcode][i++]) {

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
	sciprintf("%s", s->opcodes[opcode].name);

	i = 0;
	while (formats[opcode][i]) {
		switch (formats[opcode][i++]) {
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
				sciprintf(" %s[%x]", (param_value < s->kfunct_nr) ?
							((param_value < s->kernel_names_nr) ? s->kernel_names[param_value] : "[Unknown(postulated)]")
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

	if (REG_EQ(pos, *p_pc)) { // Extra information if debugging the current opcode
		if ((opcode == op_pTos) || (opcode == op_sTop) || (opcode == op_pToa) || (opcode == op_aTop) ||
		        (opcode == op_dpToa) || (opcode == op_ipToa) || (opcode == op_dpTos) || (opcode == op_ipTos)) {
			int prop_ofs = scr[pos.offset + 1];
			int prop_id = prop_ofs_to_id(s, prop_ofs, *p_objp);

			sciprintf("	(%s)", selector_name(s, prop_id));
		}
	}

	sciprintf("\n");

	if (REG_EQ(pos, *p_pc)) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.offset + 2] >> 1) + (*p_restadjust);
			int argc = ((*p_sp)[- stackframe - 1]).offset;

			if (s->version >= SCI_VERSION_FTU_NEW_SCRIPT_HEADER)
				argc += (*p_restadjust);

			sciprintf(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				sciprintf(PREG, PRINT_REG((*p_sp)[j - stackframe]));
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

				sciprintf("  %s::%s[", name, (selector > s->_selectorNames.size()) ? "<invalid>" : selector_name(s, selector));

				switch (lookup_selector(s, called_obj_addr, selector, &val_ref, &fun_ref)) {
				case SELECTOR_METHOD:
					sciprintf("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case SELECTOR_VARIABLE:
					sciprintf("VAR");
					break;
				case SELECTOR_NONE:
					sciprintf("INVALID");
					break;
				}

				sciprintf("](");

				while (argc--) {
					sciprintf(PREG, PRINT_REG(sb[- stackframe + 2]));
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

int c_dumpnodes(EngineState *s) {
	int end = MIN<int>(cmd_params[0].val, VOCAB_TREE_NODES);
	int i;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	for (i = 0; i < end; i++) {
		sciprintf(" Node %03x: ", i);
		if (s->parser_nodes[i].type == PARSE_TREE_NODE_LEAF)
			sciprintf("Leaf: %04x\n", s->parser_nodes[i].content.value);
		else
			sciprintf("Branch: ->%04x, ->%04x\n", s->parser_nodes[i].content.branches[0],
			          s->parser_nodes[i].content.branches[1]);
	}

	return 0;
}

static const char *varnames[] = {"global", "local", "temp", "param"};
static const char *varabbrev = "gltp";

int c_vmvarlist(EngineState *s) {
	int i;

	for (i = 0;i < 4;i++) {
		sciprintf("%s vars at "PREG" ", varnames[i], PRINT_REG(make_reg(p_var_segs[i], p_vars[i] - p_var_base[i])));
		if (p_var_max)
			sciprintf("  total %d", p_var_max[i]);
		sciprintf("\n");
	}

	return 0;
}

int c_vmvars(EngineState *s) {
	const char *vartype_pre = strchr(varabbrev, *cmd_params[0].str);
	int vartype;
	int idx = cmd_params[1].val;

	if (!vartype_pre) {
		sciprintf("Invalid variable type '%c'\n", *cmd_params[0].str);
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

	switch (cmd_paramlength) {
	case 2:
		sciprintf("%s var %d == "PREG"\n", varnames[vartype], idx, PRINT_REG(p_vars[vartype][idx]));
		break;

	case 3:
		p_vars[vartype][idx] = cmd_params[2].reg;
		break;

	default:
		sciprintf("Too many arguments\n");
	}

	return 0;
}

static int c_backtrace(EngineState *s) {
	int i;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	sciprintf("Call stack (current base: 0x%x):\n", s->execution_stack_base);
	for (i = 0; i <= s->execution_stack_pos; i++) {
		exec_stack_t *call = &(s->execution_stack[i]);
		const char *objname = obj_get_name(s, call->sendp);
		int paramc, totalparamc;

		switch (call->type) {

		case EXEC_STACK_TYPE_CALL: {// Normal function
			sciprintf(" %x:[%x]  %s::%s(", i, call->origin, objname, (call->selector == -1) ? "<call[be]?>" :
			          selector_name(s, call->selector));
		}
		break;

		case EXEC_STACK_TYPE_KERNEL: // Kernel function
			sciprintf(" %x:[%x]  k%s(", i, call->origin, s->kernel_names[-(call->selector)-42]);
			break;

		case EXEC_STACK_TYPE_VARSELECTOR:
			sciprintf(" %x:[%x] vs%s %s::%s (", i, call->origin, (call->argc) ? "write" : "read",
			          objname, s->_selectorNames[call->selector].c_str());
			break;
		}

		totalparamc = call->argc;

		if (totalparamc > 16)
			totalparamc = 16;

		for (paramc = 1; paramc <= totalparamc; paramc++) {
			sciprintf(PREG, PRINT_REG(call->variables_argp[paramc]));

			if (paramc < call->argc)
				sciprintf(", ");
		}

		if (call->argc > 16)
			sciprintf("...");

		sciprintf(")\n    obj@"PREG, PRINT_REG(call->objp));
		if (call->type == EXEC_STACK_TYPE_CALL) {
			sciprintf(" pc="PREG, PRINT_REG(call->addr.pc));
			if (call->sp == CALL_SP_CARRY)
				sciprintf(" sp,fp:carry");
			else {
				sciprintf(" sp="PSTK, PRINT_STK(call->sp));
				sciprintf(" fp="PSTK, PRINT_STK(call->fp));
			}
		} else
			sciprintf(" pc:none");

		sciprintf(" argp:"PSTK, PRINT_STK(call->variables_argp));
		if (call->type == EXEC_STACK_TYPE_CALL)
			sciprintf(" script: %d", s->seg_manager->heap[call->addr.pc.segment]->data.script.nr);
		sciprintf("\n");
	}

	return 0;
}

static int c_redraw_screen(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	s->visual->draw(GFXW(s->visual), Common::Point(0, 0));
	gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(s->gfx_state);
	gfxop_usleep(s->gfx_state, 10);

	return 0;
}

static int c_clear_screen(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	gfxop_clear_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update_box(s->gfx_state, gfx_rect(0, 0, 320, 200));

	return 0;
}

static int c_visible_map(EngineState *s) {
	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	//WARNING(fixme!)
#if 0
	if (s->onscreen_console)
		con_restore_screen(s, s->osc_backup);

	if (cmd_params[0].val <= 3)
		s->pic_visible_map = cmd_params[0].val;
	c_redraw_screen(s);

	if (s->onscreen_console)
		s->osc_backup = con_backup_screen(s);
#endif
	return 0;
}

static int c_gfx_current_port(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (!s->port)
		sciprintf("none.\n");
	else
		sciprintf("%d\n", s->port->ID);

	return 0;
}

static int c_gfx_print_port(EngineState *s) {
	gfxw_port_t *port;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	port = s->port;

	if (cmd_paramlength > 0) {
		if (s->visual) {
			port = gfxw_find_port(s->visual, cmd_params[0].val);
		} else {
			sciprintf("visual is uninitialized.\n");
			return 1;
		}
	}

	if (port)
		port->print(GFXW(port), 0);
	else
		sciprintf("No such port.\n");

	return 0;
}

static int c_gfx_priority(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmd_paramlength) {
		int zone = cmd_params[0].val;
		if (zone < 0)
			zone = 0;
		if (zone > 15) zone = 15;

		sciprintf("Zone %x starts at y=%d\n", zone, PRIORITY_BAND_FIRST(zone));
	} else {
		sciprintf("Priority bands start at y=%d\nThey end at y=%d\n", s->priority_first, s->priority_last);
	}

	return 0;
}

static int c_gfx_print_visual(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (s->visual)
		s->visual->print(GFXW(s->visual), 0);
	else
		sciprintf("visual is uninitialized.\n");

	return 0;
}

static int c_gfx_print_dynviews(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (!s->dyn_views)
		sciprintf("No dynview list active.\n");
	else
		s->dyn_views->print(GFXW(s->dyn_views), 0);

	return 0;
}

static int c_gfx_print_dropviews(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (!s->drop_views)
		sciprintf("No dropped dynview list active.\n");
	else
		s->drop_views->print(GFXW(s->drop_views), 0);

	return 0;
}

static int c_gfx_drawpic(EngineState *s) {
	int flags = 1, default_palette = 0;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmd_paramlength > 1) {
		default_palette = cmd_params[1].val;

		if (cmd_paramlength > 2)
			flags = cmd_params[2].val;
	}

	gfxop_new_pic(s->gfx_state, cmd_params[0].val, flags, default_palette);
	gfxop_clear_box(s->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(s->gfx_state);
	gfxop_usleep(s->gfx_state, 0);

	return 0;
}

#ifdef GFXW_DEBUG_WIDGETS
extern gfxw_widget_t *debug_widgets[];
extern int debug_widget_pos;

static int c_gfx_print_widget(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmd_paramlength) {
		unsigned int i;
		for (i = 0; i < cmd_paramlength ; i++) {
			int widget_nr = cmd_params[i].val;

			sciprintf("===== Widget #%d:\n", widget_nr);
			debug_widgets[widget_nr]->print(debug_widgets[widget_nr], 0);
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

static int c_gfx_show_map(EngineState *s) {
	int map = cmd_params[0].val;
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	switch (map) {
	case 0:
		s->visual->add_dirty_abs(GFXWC(s->visual), gfx_rect(0, 0, 320, 200), 0);
		s->visual->draw(GFXW(s->visual), Common::Point(0, 0));
		break;

	case 1:
		gfx_xlate_pixmap(s->gfx_state->pic->priority_map, s->gfx_state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(s->gfx_state, s->gfx_state->pic->priority_map, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		break;

	case 2:
		gfx_xlate_pixmap(s->gfx_state->control_map, s->gfx_state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(s->gfx_state, s->gfx_state->control_map, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		break;

	default:
		sciprintf("Map %d is not available.\n", map);
		return 1;
	}

	gfxop_update(s->gfx_state);

	return 0;
}

static int c_gfx_draw_cel(EngineState *s) {
	int view = cmd_params[0].val;
	int loop = cmd_params[1].val;
	int cel = cmd_params[2].val;
	int palette = cmd_params[3].val;

	if (!s) {
		sciprintf("Not in debug state!\n");
		return 1;
	}

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);
	gfxop_draw_cel(s->gfx_state, view, loop, cel, Common::Point(160, 100), s->ega_colors[0], palette);
	gfxop_update(s->gfx_state);

	return 0;
}

static int c_gfx_fill_screen(EngineState *s) {
	int col = cmd_params[0].val;

	if (!s) {
		sciprintf("Not in debug state!\n");
		return 1;
	}

	if (col < 0 || col > 15)
		col = 0;

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);
	gfxop_fill_box(s->gfx_state, gfx_rect_fullscreen, s->ega_colors[col]);
	gfxop_update(s->gfx_state);

	return 0;
}

static int c_gfx_draw_rect(EngineState *s) {
	int col = cmd_params[4].val;

	if (!s) {
		sciprintf("Not in debug state!\n");
		return 1;
	}

	if (col < 0 || col > 15)
		col = 0;

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);
	gfxop_fill_box(s->gfx_state, gfx_rect(cmd_params[0].val, cmd_params[1].val, cmd_params[2].val, cmd_params[3].val), s->ega_colors[col]);
	gfxop_update(s->gfx_state);

	return 0;
}

static int c_gfx_propagate_rect(EngineState *s) {
	int map = cmd_params[4].val;
	rect_t rect;

	if (!s) {
		sciprintf("Not in debug state!\n");
		return 1;
	}

	if (map < 0 || map > 1)
		map = 0;

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	rect = gfx_rect(cmd_params[0].val, cmd_params[1].val, cmd_params[2].val, cmd_params[3].val);

	if (map == 1)
		gfxop_clear_box(s->gfx_state, rect);
	else
		gfxop_update_box(s->gfx_state, rect);
	gfxop_update(s->gfx_state);
	gfxop_usleep(s->gfx_state, 10);

	return 0;
}

#define GETRECT(ll, rr, tt, bb) \
ll = GET_SELECTOR(pos, ll); \
rr = GET_SELECTOR(pos, rr); \
tt = GET_SELECTOR(pos, tt); \
bb = GET_SELECTOR(pos, bb);

#if 0
// Unreferenced - removed
static int c_gfx_draw_viewobj(EngineState *s) {
#ifdef __GNUC__
#warning "Re-implement con:gfx_draw_viewobj"
#endif
#if 0
	heap_ptr pos = (heap_ptr)(cmd_params[0].val);
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

	if ((getInt16(s->heap + pos + SCRIPT_OBJECT_MAGIC_OFFSET)) != SCRIPT_OBJECT_MAGIC_NUMBER) {
		sciprintf("Not an object.\n");
		return 0;
	}


	is_view = (lookup_selector(s, pos, s->selector_map.x, NULL) == SELECTOR_VARIABLE) &&
	    (lookup_selector(s, pos, s->selector_map.brLeft, NULL) == SELECTOR_VARIABLE) &&
	    (lookup_selector(s, pos, s->selector_map.signal, NULL) == SELECTOR_VARIABLE) &&
	    (lookup_selector(s, pos, s->selector_map.nsTop, NULL) == SELECTOR_VARIABLE);

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

static int c_gfx_flush_resources(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	gfxop_set_pointer_cursor(s->gfx_state, GFXOP_NO_POINTER);
	sciprintf("Flushing resources...\n");
	s->visual->widfree(GFXW(s->visual));
	gfxr_free_all_resources(s->gfx_state->driver, s->gfx_state->resstate);
	s->visual = NULL;

	return 0;
}

static int c_gfx_update_zone(EngineState *s) {
	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	return s->gfx_state->driver->update(s->gfx_state->driver, gfx_rect(cmd_params[0].val, cmd_params[1].val, cmd_params[2].val, cmd_params[3].val),
										Common::Point(cmd_params[0].val, cmd_params[1].val), GFX_BUFFER_FRONT);
}

static int c_disasm_addr(EngineState *s) {
	reg_t vpc = cmd_params[0].reg;
	int op_count = 1;
	int do_bwc = 0;
	int do_bytes = 0;
	unsigned int i;
	int invalid = 0;
	int size;

	s->seg_manager->dereference(vpc, &size);
	size += vpc.offset; // total segment size

	for (i = 1; i < cmd_paramlength; i++) {
		if (!scumm_stricmp(cmd_params[i].str, "bwt"))
			do_bwc = 1;
		else if (!scumm_stricmp(cmd_params[i].str, "bc"))
			do_bytes = 1;
		else if (toupper(cmd_params[i].str[0]) == 'C')
			op_count = atoi(cmd_params[i].str + 1);
		else {
			invalid = 1;
			sciprintf("Invalid option '%s'\n", cmd_params[i].str);
		}
	}

	if (invalid || op_count < 0)
		return invalid;

	do {
		vpc = disassemble(s, vpc, do_bwc, do_bytes);

	} while ((vpc.offset > 0) && (vpc.offset + 6 < size) && (--op_count));
	return 0;
}

static int c_disasm(EngineState *s) {
	object_t *obj = obj_get(s, cmd_params[0].reg);
	int selector_id = script_find_selector(s, cmd_params[1].str);
	reg_t addr;

	if (!obj) {
		sciprintf("Not an object.");
		return 1;
	}

	if (selector_id < 0) {
		sciprintf("Not a valid selector name.");
		return 1;
	}

	if (lookup_selector(s, cmd_params[0].reg, selector_id, NULL, &addr) != SELECTOR_METHOD) {
		sciprintf("Not a method.");
		return 1;
	}

	do {
		addr = disassemble(s, addr, 0, 0);
	} while (addr.offset > 0);

	return 0;
}

static int c_sg(EngineState *s) {
	_debug_seeking = _DEBUG_SEEK_GLOBAL;
	_debug_seek_special = cmd_params[0].val;
	_debugstate_valid = 0;

	return 0;
}

static int c_snk(EngineState *s) {
	int callk_index;
	char *endptr;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmd_paramlength > 0) {
		/* Try to convert the parameter to a number. If the conversion stops
		   before end of string, assume that the parameter is a function name
		   and scan the function table to find out the index. */
		callk_index = strtoul(cmd_params [0].str, &endptr, 0);
		if (*endptr != '\0') {
			int i;

			callk_index = -1;
			for (i = 0; i < s->kernel_names_nr; i++)
				if (!strcmp(cmd_params [0].str, s->kernel_names [i])) {
					callk_index = i;
					break;
				}

			if (callk_index == -1) {
				sciprintf("Unknown kernel function '%s'\n", cmd_params [0].str);
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

static int c_sret(EngineState *s) {
	_debug_seeking = _DEBUG_SEEK_LEVEL_RET;
	_debug_seek_level = s->execution_stack_pos;
	_debugstate_valid = 0;
	return 0;
}

static int c_go(EngineState *s) {
	_debug_seeking = 0;
	_debugstate_valid = 0;
	script_debug_flag = 0;
	return 0;
}

static int c_set_acc(EngineState *s) {
	s->r_acc = cmd_params[0].reg;
	return 0;
}

static int c_send(EngineState *s) {
	reg_t object = cmd_params[0].reg;
	char *selector_name = cmd_params[1].str;
	stack_ptr_t stackframe = s->execution_stack->sp;
	int selector_id;
	unsigned int i, selector_type;
	exec_stack_t *xstack;
	object_t *o;
	reg_t *vptr;
	reg_t fptr;

	selector_id = vocabulary_lookup_sname(s->_selectorNames, selector_name);

	if (selector_id < 0) {
		sciprintf("Unknown selector: \"%s\"\n", selector_name);
		return 1;
	}

	o = obj_get(s, object);
	if (o == NULL) {
		sciprintf("Address \""PREG"\" is not an object\n", PRINT_REG(object));
		return 1;
	}

	selector_type = lookup_selector(s, object, selector_id, &vptr, &fptr);

	if (selector_type == SELECTOR_NONE) {
		sciprintf("Object does not support selector: \"%s\"\n", selector_name);
		return 1;
	}

	stackframe[0] = make_reg(0, selector_id);
	stackframe[1] = make_reg(0, cmd_paramlength - 2);

	for (i = 2; i < cmd_paramlength; i++)
		stackframe[i] = cmd_params[i].reg;

	xstack = add_exec_stack_entry(s, fptr, s->execution_stack->sp + cmd_paramlength, object, cmd_paramlength - 2,
									s->execution_stack->sp - 1, 0, object, s->execution_stack_pos, SCI_XS_CALLEE_LOCALS);
	xstack->selector = selector_id;
	xstack->type = selector_type == SELECTOR_VARIABLE ? EXEC_STACK_TYPE_VARSELECTOR : EXEC_STACK_TYPE_CALL;

	// Now commit the actual function:
	xstack = send_selector(s, object, object, stackframe, cmd_paramlength - 2, stackframe);

	xstack->sp += cmd_paramlength;
	xstack->fp += cmd_paramlength;

	s->execution_stack_pos_changed = 1;

	return 0;
}

static int c_resource_id(EngineState *s) {
	int id = cmd_params[0].val;

	sciprintf("%s.%d (0x%x)\n", sci_resource_types[id >> 11], id &0x7ff, id & 0x7ff);

	return 0;
}

static int c_listclones(EngineState *s) {
/*	int i, j = 0;

	sciprintf("Listing all logged clones:\n");

	for (i = 0; i < SCRIPT_MAX_CLONES; i++) {
		if (s->clone_list[i]) {
			sciprintf("  Clone at %04x\n", s->clone_list[i]);
			j++;
		}
	}

	sciprintf("Total of %d clones.\n", j);*/
	sciprintf("This function is temporarily disabled.\n");

	return 0;
}

struct generic_config_flag_t {
	const char *name;
	const char option;
	unsigned int flag;
};

static void handle_config_update(const generic_config_flag_t *flags_list, int flags_nr, const char *subsystem,
								 int *active_options_p, char *changestring /* or NULL to display*/) {
	if (!changestring) {
		int j;

		sciprintf("Logging in %s:\n", subsystem);
		if (!(*active_options_p))
			sciprintf("  (nothing)\n");

		for (j = 0; j < flags_nr; j++)
			if (*active_options_p & flags_list[j].flag) {
				sciprintf("  - %s (%c)\n", flags_list[j].name, flags_list[j].option);
			}
	} else {
		int mode;
		int j = 0;
		int flags = 0;

		if (changestring[0] == '-')
			mode = 0;
		else if (changestring[0] == '+')
			mode = 1;
		else {
			sciprintf("Mode spec must start with '+' or '-' in '%s'\n", changestring);
			return;
		}

		while (changestring[++j]) {
			int k;
			int flag = 0;

			if (changestring[j] == '*')
				flags = ~0; // Everything
			else
				for (k = 0; !flag && k < flags_nr; k++)
					if (flags_list[k].option == changestring[j])
						flag = flags_list[k].flag;

			if (!flag) {
				sciprintf("Invalid/unknown mode flag '%c'\n", changestring[j]);
				return;
			}
			flags |= flag;
		}

		if (mode) // +
			*active_options_p |= flags;
		else // -
			*active_options_p &= ~flags;
	}
}

static int c_handle_config_update(const generic_config_flag_t *flags, int flags_nr, const char *subsystem, int *active_options_p) {
	unsigned int i;

	if (!_debugstate_valid) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (cmd_paramlength == 0)
		handle_config_update(flags, flags_nr, subsystem, active_options_p, 0);

	for (i = 0; i < cmd_paramlength; i++)
		handle_config_update(flags, flags_nr, subsystem, active_options_p, cmd_params[i].str);

	return 0;
}

const generic_config_flag_t SCIk_Debug_Names[SCIk_DEBUG_MODES] = {
	{"Lists and nodes", 'l', (1 << 1)},
	{"Graphics", 'g', (1 << 2)},
	{"Character handling", 'c', (1 << 3)},
	{"Memory management", 'm', (1 << 4)},
	{"Function parameter checks", 'f', (1 << SCIkFUNCCHK_NR)},
	{"Bresenham algorithms", 'b', (1 << 6)},
	{"Audio subsystem", 'a', (1 << SCIkSOUNDCHK_NR)},
	{"System graphics driver", 'd', (1 << SCIkGFXDRIVER_NR)},
	{"Base setter results", 's', (1 << SCIkBASESETTER_NR)},
	{"Parser", 'p', (1 << SCIkPARSER_NR)},
	{"Menu handling", 'M', (1 << 11)},
	{"Said specs", 'S', (1 << 12)},
	{"File I/O", 'F', (1 << 13)},
	{"Time", 't', (1 << 14)},
	{"Room numbers", 'r', (1 << 15)},
	{"FreeSCI 0.3.3 kernel emulation", 'e', (1 << 16)},
	{"Pathfinding", 'P', (1 << SCIkAVOIDPATH_NR)}
} ;

void set_debug_mode(EngineState *s, int mode, const char *areas) {
	char *param = (char*)sci_malloc(strlen(areas) + 2);

	param[0] = (mode) ? '+' : '-';
	strcpy(param + 1, areas);

	handle_config_update(SCIk_Debug_Names, SCIk_DEBUG_MODES, "VM and kernel", (int *)&(s->debug_mode), param);

	free(param);
}

int c_debuglog(EngineState *s) {
	return c_handle_config_update(SCIk_Debug_Names, SCIk_DEBUG_MODES, "VM and kernel", (int *)&(s->debug_mode));
}

#define SFX_DEBUG_MODES 2
#define FROBNICATE_HANDLE(reg) ((reg).segment << 16 | (reg).offset)

static int c_sfx_debuglog(EngineState *s) {
	const generic_config_flag_t sfx_debug_modes[SFX_DEBUG_MODES] = {
		{"Song activation/deactivation", 's', SFX_DEBUG_SONGS},
		{"Song cue polling and delivery", 'c', SFX_DEBUG_CUES}
	};

	return c_handle_config_update(sfx_debug_modes, SFX_DEBUG_MODES, "sound subsystem", (int *)&(s->sound.debug));
}

static int c_sfx_remove(EngineState *s) {
	reg_t id = cmd_params[0].reg;
	int handle = FROBNICATE_HANDLE(id);

	if (id.segment) {
		sfx_song_set_status(&s->sound, handle, SOUND_STATUS_STOPPED);
		sfx_remove_song(&s->sound, handle);
		PUT_SEL32V(id, signal, -1);
		PUT_SEL32V(id, nodePtr, 0);
		PUT_SEL32V(id, handle, 0);
	}

	return 0;
}

#define GFX_DEBUG_MODES 4

int c_gfx_debuglog(EngineState *s) {
	gfx_driver_t *drv = s->gfx_state->driver;
	const generic_config_flag_t gfx_debug_modes[GFX_DEBUG_MODES] = {
		{ "Mouse Pointer", 'p', GFX_DEBUG_POINTER},
		{ "Updates", 'u', GFX_DEBUG_UPDATES},
		{ "Pixmap operations", 'x', GFX_DEBUG_PIXMAPS},
		{ "Basic operations", 'b', GFX_DEBUG_BASIC},
	};

	return c_handle_config_update(gfx_debug_modes, GFX_DEBUG_MODES, "graphics subsystem", (int *)&(drv->debug_flags));
}

int c_dump_words(EngineState *s) {
	int i;

	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	if (!s->parser_words) {
		sciprintf("No words.\n");
		return 0;
	}

	for (i = 0; i < s->parser_words_nr; i++) {
		word_t *word = s->parser_words[i];
		sciprintf("%s: C %03x G %03x\n", word->word, word->w_class, word->group);
	}
	sciprintf("%d words\n", s->parser_words_nr);

	return 0;
}

int c_simkey(EngineState *s) {
	_kdebug_cheap_event_hack = cmd_params[0].val;

	return 0;
}

static int c_is_sample(EngineState *s) {
	resource_t *song = scir_find_resource(s->resmgr, sci_sound, cmd_params[0].val, 0);
	song_iterator_t *songit;
	sfx_pcm_feed_t *data;

	if (!song) {
		sciprintf("Not a sound resource.\n");
		return 1;
	}

	songit = songit_new(song->data, song->size, SCI_SONG_ITERATOR_TYPE_SCI0, 0xcaffe /* What do I care about the ID? */);

	if (!songit) {
		sciprintf("Error-- Could not convert to song iterator\n");
		return 1;
	}

	if ((data = songit->get_pcm_feed(songit))) {
		sciprintf("\nIs sample (encoding %dHz/%s/%04x).\n", data->conf.rate, (data->conf.stereo) ?
		          ((data->conf.stereo == SFX_PCM_STEREO_LR) ? "stereo-LR" : "stereo-RL") : "mono", data->conf.format);
		data->destroy(data);
	} else
		sciprintf("Valid song, but not a sample.\n");

	songit_free(songit);

	return 0;
}

int c_simsoundcue(EngineState *s) {
	_kdebug_cheap_soundcue_hack = cmd_params[0].val;

	return 0;
}

#define ASSERT_PARAMS(number) \
	if (cmd_paramlength <= number) {\
		sciprintf("Operation '%s' needs %d parameters\n", op, number); \
		return 1;\
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
static void viewobjinfo(EngineState *s, heap_ptr pos) {
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
	abs_rect_t nsrect, nsrect_clipped, brrect;

	if (lookup_selector(s, pos, s->selector_map.nsBottom, NULL) == SELECTOR_VARIABLE) {
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
	if (s->selector_map.z > 0) {
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
	object_t *obj = obj_get(s, pos);
	object_t *var_container = obj;
	int i;

	sciprintf("["PREG"]: ", PRINT_REG(pos));
	if (!obj) {
		sciprintf("Not an object.");
		return 1;
	}

	print_obj_head(s, obj);

	if (!(obj->variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS))
		var_container = obj_get(s, obj->variables[SCRIPT_SUPERCLASS_SELECTOR]);
	sciprintf("  -- member variables:\n");
	for (i = 0; i < obj->variables_nr; i++) {
		sciprintf("    ");
		if (i < var_container->variable_names_nr)
			sciprintf("[%03x] %s = ", VM_OBJECT_GET_VARSELECTOR(var_container, i), selector_name(s, VM_OBJECT_GET_VARSELECTOR(var_container, i)));
		else
			sciprintf("p#%x = ", i);

		sciprintf(PREG"\n", PRINT_REG(obj->variables[i]));
	}
	sciprintf("  -- methods:\n");
	for (i = 0; i < obj->methods_nr; i++) {
		reg_t fptr = VM_OBJECT_READ_FUNCTION(obj, i);
		sciprintf("    [%03x] %s = "PREG"\n", VM_OBJECT_GET_FUNCSELECTOR(obj, i), selector_name(s, VM_OBJECT_GET_FUNCSELECTOR(obj, i)), PRINT_REG(fptr));
	}
	if (s->seg_manager->heap[pos.segment]->type == MEM_OBJ_SCRIPT)
		sciprintf("\nOwner script:\t%d\n", s->seg_manager->heap[pos.segment]->data.script.nr);

	return 0;
}

int c_vo(EngineState *s) {
	return objinfo(s, cmd_params[0].reg);
}

int c_obj(EngineState *s) {
	return objinfo(s, *p_objp);
}

int c_accobj(EngineState *s) {
	return objinfo(s, s->r_acc);
}

int c_shownode(EngineState *s) {
	reg_t addr = cmd_params[0].reg;

	return show_node(s, addr);
}

// Breakpoint commands

static breakpoint_t *bp_alloc(EngineState *s) {
	breakpoint_t *bp;

	if (s->bp_list) {
		bp = s->bp_list;
		while (bp->next)
			bp = bp->next;
		bp->next = (breakpoint_t *)sci_malloc(sizeof(breakpoint_t));
		bp = bp->next;
	} else {
		s->bp_list = (breakpoint_t *)sci_malloc(sizeof(breakpoint_t));
		bp = s->bp_list;
	}

	bp->next = NULL;

	return bp;
}

int c_bpx(EngineState *s) {
	breakpoint_t *bp;

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */

	bp = bp_alloc(s);

	bp->type = BREAK_SELECTOR;
	bp->data.name = (char *)sci_malloc(strlen(cmd_params [0].str) + 1);
	strcpy(bp->data.name, cmd_params [0].str);
	s->have_bp |= BREAK_SELECTOR;

	return 0;
}

int c_bpe(EngineState *s) {
	breakpoint_t *bp;

	bp = bp_alloc(s);

	bp->type = BREAK_EXPORT;
	bp->data.address = (cmd_params [0].val << 16 | cmd_params [1].val);
	s->have_bp |= BREAK_EXPORT;

	return 0;
}

int c_bplist(EngineState *s) {
	breakpoint_t *bp;
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

int c_bpdel(EngineState *s) {
	breakpoint_t *bp, *bp_next, *bp_prev;
	int i = 0, found = 0;
	int type;

	// Find breakpoint with given index
	bp_prev = NULL;
	bp = s->bp_list;
	while (bp && i < cmd_params [0].val) {
		bp_prev = bp;
		bp = bp->next;
		i++;
	}
	if (!bp) {
		sciprintf("Invalid breakpoint index %i\n", cmd_params [0].val);
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

int c_gnf(EngineState *s) {
	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	vocab_gnf_dump(s->parser_branches, s->parser_branches_nr);

	return 0;
}

int c_se(EngineState *s) {
	stop_on_event = 1;
	_debugstate_valid = script_debug_flag = script_error_flag = 0;

	return 0;
}

int c_type(EngineState *s) {
	int t = determine_reg_type(s, cmd_params[0].reg, 1);
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

int c_statusbar(EngineState *s) {
	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	s->titlebar_port->color = s->ega_colors[cmd_params[0].val];
	s->titlebar_port->bgcolor = s->ega_colors[cmd_params[1].val];

	s->status_bar_foreground = cmd_params[0].val;
	s->status_bar_background = cmd_params[1].val;

	sciw_set_status_bar(s, s->titlebar_port, s->status_bar_text, s->status_bar_foreground, s->status_bar_background);
	gfxop_update(s->gfx_state);

	return 0;
}

int c_sci_version(EngineState *s) {
	if (!s) {
		sciprintf("Not in debug state\n");
		return 1;
	}

	sciprintf("Emulating SCI version %d.%03d.%03d\n", SCI_VERSION_MAJOR(s->version), SCI_VERSION_MINOR(s->version),
	          SCI_VERSION_PATCHLEVEL(s->version));

	return 0;
}

// int c_sleep(EngineState *s) {
//	sleep(cmd_params[0].val);
//
//	return 0;
// }

static void _print_address(void * _, reg_t addr) {
	if (addr.segment)
		sciprintf("  "PREG"\n", PRINT_REG(addr));
}

#define GET_SEG_INTERFACE(seg_id) \
	SegInterface *seg_interface = s->seg_manager->getSegInterface(seg_id);	\
	if (!seg_interface) {								\
		sciprintf("Unknown segment : %x\n", seg_id);				\
		return 1;								\
	}

static int c_gc_show_reachable(EngineState *s) {
	reg_t addr = cmd_params[0].reg;

	GET_SEG_INTERFACE(addr.segment);

	sciprintf("Reachable from "PREG":\n", PRINT_REG(addr));
	seg_interface->listAllOutgoingReferences(s, addr, NULL, _print_address);

	delete seg_interface;

	return 0;
}

static int c_gc_show_freeable(EngineState *s) {
	reg_t addr = cmd_params[0].reg;

	GET_SEG_INTERFACE(addr.segment);

	sciprintf("Freeable in segment %04x:\n", addr.segment);
	seg_interface->listAllDeallocatable(NULL, _print_address);

	delete seg_interface;

	return 0;
}

static int c_gc_normalise(EngineState *s) {
	reg_t addr = cmd_params[0].reg;

	GET_SEG_INTERFACE(addr.segment);

	addr = seg_interface->findCanonicAddress(addr);
	sciprintf(" "PREG"\n", PRINT_REG(addr));

	delete seg_interface;

	return 0;
}

static int c_gc(EngineState *s) {
	run_gc(s);

	return 0;
}

static int c_gc_list_reachable(EngineState *s) {
	reg_t_hash_map *use_map = find_all_used_references(s);

	sciprintf("Reachable references (normalised):\n");
	for (reg_t_hash_map::iterator i = use_map->begin(); i != use_map->end(); ++i) {
		sciprintf(" - "PREG"\n", PRINT_REG(i->_key));
	}

	delete use_map;

	return 0;
}

void script_debug(EngineState *s, reg_t *pc, stack_ptr_t *sp, stack_ptr_t *pp, reg_t *objp, int *restadjust,
	seg_id_t *segids, reg_t **variables, reg_t **variables_base, int *variables_nr, int bp) {
	// Do we support a separate console?

	if (sci_debug_flags & _DEBUG_FLAG_LOGGING) {
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
		sciprintf("%d: acc="PREG"  ", script_step_counter, PRINT_REG(s->r_acc));
		_debugstate_valid = 1;
		disassemble(s, *pc, 0, 1);
		if (_debug_seeking == _DEBUG_SEEK_GLOBAL)
			sciprintf("Global %d (0x%x) = "PREG"\n", _debug_seek_special,
			          _debug_seek_special, PRINT_REG(s->script_000->locals_block->locals[_debug_seek_special]));

		_debugstate_valid = old_debugstate;

		if (!script_debug_flag)
			return;
	}

	if (_debug_seeking && !bp) { // Are we looking for something special?
		mem_obj_t *memobj = GET_SEGMENT(*s->seg_manager, pc->segment, MEM_OBJ_SCRIPT);

		if (memobj) {
			script_t *scr = &(memobj->data.script);
			byte *code_buf = scr->buf;
			int code_buf_size = scr->buf_size;
			int opcode = pc->offset >= code_buf_size ? 0 : code_buf[pc->offset];
			int op = opcode >> 1;
			int paramb1 = pc->offset + 1 >= code_buf_size ? 0 : code_buf[pc->offset + 1];
			int paramf1 = (opcode & 1) ? paramb1 : (pc->offset + 2 >= code_buf_size ? 0 : getInt16(code_buf + pc->offset + 1));

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
				if ((op != op_ret) || (_debug_seek_level < s->execution_stack_pos))
					return;
				break;
			}

			case _DEBUG_SEEK_SO:
				if (!REG_EQ(*pc, _debug_seek_reg) || s->execution_stack_pos != _debug_seek_level)
					return;
				break;

			case _DEBUG_SEEK_GLOBAL:

				if (op < op_sag)
					return;
				if ((op & 0x3) > 1)
					return; // param or temp
				if ((op & 0x3) && s->execution_stack[s->execution_stack_pos].local_segment > 0)
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
			con_hook_command(c_debuginfo, "registers", "", "Displays all current register values");
			con_hook_command(c_vmvars, "vmvars", "!sia*", "Displays or changes variables in the VM\n\nFirst parameter is either g(lobal), l(ocal), t(emp) or p(aram).\nSecond parameter is the var number\nThird parameter (if specified) is the value to set the variable to");
			con_hook_command(c_sci_version, "sci_version", "", "Prints the SCI version currently being emulated");
			con_hook_command(c_vmvarlist, "vmvarlist", "!", "Displays the addresses of variables in the VM");
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
			con_hook_command(c_classtable, "classtable", "", "Lists all available classes");
			con_hook_command(c_stack, "stack", "i", "Dumps the specified number of stack elements");
			con_hook_command(c_backtrace, "bt", "", "Dumps the send/self/super/call/calle/callb stack");
			con_hook_command(c_snk, "snk", "s*", "Steps forward until it hits the next\n  callk operation.\n"
			                 "  If invoked with a parameter, it will\n  look for that specific callk.\n");
			con_hook_command(c_se, "se", "", "Steps forward until an SCI event is received.\n");
			con_hook_command(c_listclones, "clonetable", "", "Lists all registered clones");
			con_hook_command(c_set_acc, "set_acc", "!a", "Sets the accumulator");
			con_hook_command(c_send, "send", "!asa*", "Sends a message to an object\nExample: send ?fooScript cue");
			con_hook_command(c_sret, "sret", "", "Steps forward until ret is called\n  on the current execution stack\n  level.");
			con_hook_command(c_resource_id, "resource_id", "i", "Identifies a resource number by\n"
			                 "  splitting it up in resource type\n  and resource number.");
			con_hook_command(c_clear_screen, "clear_screen", "", "Clears the screen, shows the\n  background pic and picviews");
			con_hook_command(c_redraw_screen, "redraw_screen", "", "Redraws the screen");
			con_hook_command(c_debuglog, "debuglog", "!s*", "Sets the debug log modes.\n  Possible parameters:\n"
			                 "  +x (sets debugging for x)\n  -x (unsets debugging for x)\n\nPossible values"
			                 " for x:\n  u: Unimpl'd/stubbed stuff\n  l: Lists and nodes\n  g: Graphics\n"
			                 "  c: Character handling\n  m: Memory management\n  f: Function call checks\n"
			                 "  b: Bresenham details\n  a: Audio\n  d: System gfx management\n  s: Base setter\n"
			                 "  p: Parser\n  M: The menu system\n  S: Said specs\n  F: File I/O\n  t: GetTime\n"
			                 "  e: 0.3.3 kernel emulation\n  r: Room numbers\n  P: Pathfinding\n"
			                 "  *: Everything\n\n"
			                 "  If invoked withour parameters,\n  it will list all activated\n  debug options.\n\n"
			                 "SEE ALSO\n"
			                 "  gfx_debuglog.1, sfx_debuglog.1\n");
			con_hook_command(c_visible_map, "set_vismap", "i", "Sets the visible map.\n  Default is 0 (visual).\n"
			                 "  Other useful values are:\n  1: Priority\n  2: Control\n  3: Auxiliary\n");
			con_hook_command(c_simkey, "simkey", "i", "Simulates a keypress with the\n  specified scancode.\n");
			con_hook_command(c_statusbar, "statusbar", "ii", "Sets the colors of the status bar. Also controllable from the script.\n");
			con_hook_command(c_bpx, "bpx", "s", "Sets a breakpoint on the execution of\n  the specified method.\n\n  EXAMPLE:\n"
			                 "  bpx ego::doit\n\n  May also be used to set a breakpoint\n  that applies whenever an object\n"
			                 "  of a specific type is touched:\n  bpx foo::\n");
			con_hook_command(c_bpe, "bpe", "ii", "Sets a breakpoint on the execution of specified exported function.\n");
			con_hook_command(c_bplist, "bplist", "", "Lists all breakpoints.\n");
			con_hook_command(c_bpdel, "bpdel", "i", "Deletes a breakpoint with specified index.");
			con_hook_command(c_go, "go", "", "Executes the script.\n");
			con_hook_command(c_dumpnodes, "dumpnodes", "i", "shows the specified number of nodes\nfrom the parse node tree");
			con_hook_command(c_save_game, "save_game", "s", "Saves the current game state to\n  the hard disk");
			con_hook_command(c_restore_game, "restore_game", "s", "Restores a saved game from the\n  hard disk");
			con_hook_command(c_restart_game, "restart", "s*", "Restarts the game.\n\nUSAGE\n\n  restart [-r] [-p]"
			                 " [--play] [--replay]\n\n  There are two ways to restart an SCI\n  game:\n"
			                 "  play (-p) calls the game object's play()\n    method\n  replay (-r) calls the replay() method");
			con_hook_command(c_mousepos, "mousepos", "", "Reveal the location of a mouse click.\n\n");
			con_hook_command(c_viewinfo, "viewinfo", "ii", "Displays the number of loops\n  and cels of each loop"
			                 " for the\n  specified view resource and palette.");
			con_hook_command(c_list_sentence_fragments, "list_sentence_fragments", "", "Lists all sentence fragments (which\n"
			                 "  are used to build Parse trees).");
			con_hook_command(c_parse, "parse", "s", "Parses a sequence of words and prints\n  the resulting parse tree.\n"
			                 "  The word sequence must be provided as a\n  single string.");
			con_hook_command(c_gnf, "gnf", "", "Displays the Parse grammar\n  in strict GNF");
			con_hook_command(c_set_parse_nodes, "set_parse_nodes", "s*", "Sets the contents of all parse nodes.\n"
			                 "  Input token must be separated by\n  blanks.");
			con_hook_command(c_sfx_debuglog, "sfx_debuglog", "s*",
			                 "Sets or prints the sound subsystem debug\n"
			                 "settings\n\n"
			                 "USAGE\n\n"
			                 "  sfx_debuglog {[+|-][p|u|x|b]+}*\n\n"
			                 "  sfx_debuglog\n\n"
			                 "    Prints current settings\n\n"
			                 "  sfx_debuglog +X\n\n"
			                 "    Activates all debug features listed in X\n\n"
			                 "  sfx_debuglog -X\n\n"
			                 "    Deactivates the debug features listed in X\n\n"
			                 "  Debug features:\n"
			                 "    s: Active song changes\n"
			                 "    c: Song cues\n"
			                 "SEE ALSO\n"
			                 "  debuglog.1, gfx_debuglog.1\n");
			con_hook_command(c_gfx_debuglog, "gfx_debuglog", "s*",
			                 "Sets or prints the gfx driver's debug\n"
			                 "settings\n\n"
			                 "USAGE\n\n"
			                 "  gfx_debuglog {[+|-][p|u|x|b]+}*\n\n"
			                 "  gfx_debuglog\n\n"
			                 "    Prints current settings\n\n"
			                 "  gfx_debuglog +X\n\n"
			                 "    Activates all debug features listed in X\n\n"
			                 "  gfx_debuglog -X\n\n"
			                 "    Deactivates the debug features listed in X\n\n"
			                 "  Debug features:\n"
			                 "    p: Pointer\n"
			                 "    u: Updates\n"
			                 "    x: Pixmaps\n"
			                 "    b: Basic features\n\n"
			                 "SEE ALSO\n"
			                 "  debuglog.1, sfx_debuglog.1\n");

#ifdef SCI_SIMPLE_SAID_CODE
			con_hook_command(c_sim_parse, "simparse", "s*", "Simulates a parsed entity.\n\nUSAGE\n  Call this"
			                 " function with a list of\n  Said operators, words, and word group"
			                 "\n  numbers to match Said() specs\n  that look identical.\n"
			                 "\n  Note that opening braces and\n  everything behind them are\n"
			                 "\n  removed from all non-operator\n  parameter tokens.\n"
			                 "\n  simparse without parameters\n  removes the entity.\n");
#endif /* SCI_SIMPLE_SAID_CODE */
#ifdef GFXW_DEBUG_WIDGETS
			con_hook_command(c_gfx_print_widget, "gfx_print_widget", "i*", "If called with no parameters, it\n  shows which widgets are active.\n"
			                 "  With parameters, it lists the\n  widget corresponding to the\n  numerical index specified (for\n  each parameter).");
#endif
			con_hook_command(c_gfx_flush_resources, "gfx_free_widgets", "", "Frees all dynamically allocated\n  widgets (for memory profiling).\n");
			con_hook_command(c_gfx_current_port, "gfx_current_port", "", "Determines the current port number");
			con_hook_command(c_gfx_print_port, "gfx_print_port", "i*", "Displays all information about the\n  specified port,"
			                 " or the current port\n  if no port was specified.");
			con_hook_command(c_gfx_print_visual, "gfx_print_visual", "", "Displays all information about the\n  current widget state");
			con_hook_command(c_gfx_print_dynviews, "gfx_print_dynviews", "", "Shows the dynview list");
			con_hook_command(c_gfx_print_dropviews, "gfx_print_dropviews", "", "Shows the list of dropped\n  dynviews");
			con_hook_command(c_gfx_drawpic, "gfx_drawpic", "ii*", "Draws a pic resource\n\nUSAGE\n  gfx_drawpic <nr> [<pal> [<fl>]]\n"
			                 "  where <nr> is the number of the pic resource\n  to draw\n  <pal> is the optional default\n  palette for the pic (0 is"
			                 "\n  assumed if not specified)\n  <fl> are any pic draw flags (default\n  is 1)");
			con_hook_command(c_dump_words, "dumpwords", "", "Lists all parser words");
			con_hook_command(c_gfx_show_map, "gfx_show_map", "i", "Shows one of the screen maps\n  Semantics of the int parameter:\n"
			                 "    0: visual map (back buffer)\n    1: priority map (back buf.)\n    2: control map (static buf.)");
			con_hook_command(c_gfx_fill_screen, "gfx_fill_screen", "i", "Fills the screen with one\n  of the EGA colors\n");
			con_hook_command(c_gfx_draw_rect, "gfx_draw_rect", "iiiii", "Draws a rectangle to the screen\n  with one of the EGA colors\n\nUSAGE\n\n"
			                 "  gfx_draw_rect <x> <y> <xl> <yl> <color>");
			con_hook_command(c_gfx_propagate_rect,
			                 "gfx_propagate_rect",
			                 "iiiii",
			                 "Propagates a lower gfx buffer to a\n"
			                 "  higher gfx buffer.\n\nUSAGE\n\n"
			                 "  gfx_propagate_rect <x> <y> <xl> <yl> <buf>\n");
			con_hook_command(c_gfx_update_zone, "gfx_update_zone", "iiii", "Propagates a rectangular area from\n  the back buffer to the front buffer"
			                 "\n\nUSAGE\n\n"
			                 "  gfx_update_zone <x> <y> <xl> <yl>");
#if 0
			// TODO: Re-enable con:draw_viewobj
			con_hook_command(c_gfx_draw_viewobj, "draw_viewobj", "i", "Draws the nsRect and brRect of a\n  dynview object.\n\n  nsRect is green, brRect\n"
			                 "  is blue.\n");
#endif
			con_hook_command(c_gfx_draw_cel, "gfx_draw_cel", "iiii", "Draws a single view\n  cel to the center of the\n  screen\n\n"
			                 "USAGE\n  gfx_draw_cel <view> <loop> <cel> <palette>\n");
			con_hook_command(c_gfx_priority, "gfx_priority", "i*", "Prints information about priority\n  bands\nUSAGE\n\n  gfx_priority\n\n"
			                 "  will print the min and max values\n  for the priority bands\n\n  gfx_priority <val>\n\n  Print start of the priority\n"
			                 "  band for the specified\n  priority\n");
			con_hook_command(c_segtable, "segtable", "!",
			                 "Gives a short listing of all segments\n\n"
			                 "SEE ALSO\n\n"
			                 "  seginfo.1");
			con_hook_command(c_segkill, "segkill", "!i*",
			                 "Deletes the specified segment\n\n"
			                 "USAGE\n\n"
			                 "  segkill <nr>\n");
			con_hook_command(c_seginfo, "seginfo", "!i*",
			                 "Explains the specified segment\n\n"
			                 "USAGE\n\n"
			                 "  seginfo\n"
			                 "  seginfo <nr>\n"
			                 "  Either explains all active segments\n"
			                 "  (no parameter) or the specified one.\n\n"
			                 "SEE ALSO\n\n"
			                 "  segtable.1");
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
			con_hook_command(c_songlib_print, "songlib_print", "",
			                 "");
			con_hook_command(c_type, "type", "!a",
			                 "Determines the type of a value\n\n"
			                 "SEE ALSO\n\n  addresses.3, vo.1");
			con_hook_command(c_shownode, "shownode", "!a",
			                 "Prints information about a list node\n"
			                 "  or list base.\n\n");
			con_hook_command(c_is_sample, "is-sample", "i",
			                 "Tests whether a given sound resource\n"
			                 "  is a PCM sample, and displays infor-\n"
			                 "  mation on it if it is.\n\n");
			con_hook_command(c_sfx_01_header, "sfx-01-header", "i",
			                 "Dumps the header of an SCI01 song\n\n"
			                 "SEE ALSO\n\n"
			                 "  sfx-01-track.1\n\n");
			con_hook_command(c_sfx_01_track, "sfx-01-track", "ii",
			                 "Dumps a track from an SCI01 song\n\n"
			                 "USAGE\n\n"
			                 "  sfx-01-track <song> <offset>\n\n"
			                 "SEE ALSO\n\n"
			                 "  sfx-01-header.1\n\n");
//			con_hook_command(c_sleep, "sleep", "i", "Suspends everything for the\n"
//			                 " specified number of seconds");
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
			con_hook_command(c_gc, "gc", "",
			                 "Performs garbage collection.\n\n"
			                 "SEE ALSO\n\n"
			                 "  gc-list-freeable.1, gc-list-reachable.1,\n"
			                 "  gc-all-reachable.1, gc-normalise.1");
			con_hook_command(c_gc_list_reachable, "gc-all-reachable", "",
			                 "Lists all reachable objects, normalised.\n\n"
			                 "SEE ALSO\n\n"
			                 "  gc-list-freeable.1, gc-list-reachable.1,\n"
			                 "  gc.1, gc-normalise.1");


			con_hook_int(&script_debug_flag, "script_debug_flag", "Set != 0 to enable debugger\n");
			con_hook_int(&script_checkloads_flag, "script_checkloads_flag", "Set != 0 to display information\n"
			             "  when scripts are loaded or unloaded");
			con_hook_int(&script_abort_flag, "script_abort_flag", "Set != 0 to abort execution\n");
			con_hook_int(&script_step_counter, "script_step_counter", "# of executed SCI operations\n");
			con_hook_int(&sci_debug_flags, "debug_flags", "Debug flags:\n  0x0001: Log each command executed\n"
			             "  0x0002: Break on warnings\n  \0x0004: Print VM warnings\n");
			con_hook_int(&_weak_validations, "weak_validations", "Set != 0 to turn some validation errors\n"
			             "  into warnings\n");

			con_hook_int(&script_gc_interval, "gc-interval", "Number of kernel calls in between gcs");
			con_hook_int(&debug_sleeptime_factor, "sleep-factor", "Factor to multiply with wait times\n  Set to 0 to speed up games");

			con_hook_page("codebugging",
			              "Co-debugging allows to run two (sufficiently\n"
			              "  recent) versions of FreeSCI concurrently,\n"
			              "  with one acting as a client of the other.\n"
			              "    Co-debugging can be started by calling\n"
			              "  'codebug' (see codebug.1); note that the\n"
			              "  argument passed to it must be a version of\n"
			              "  FreeSCI that performs fflush(NULL) before\n"
			              "  each read; only late 0.3.3-devel and later\n"
			              "  have this property.\n\n"
			              "  In co-debug mode, all commands are sent to\n"
			              "  both programs, UNLESS one of the following\n"
			              "  prefixes is used:\n\n"
			              "    '.' : Only sends to the foreground version\n"
			              "    ':' : Only sends to tbe background version\n\n"
			              "  For example, when running 0.3.3 from within\n"
			              "  0.6.0, \".version\" would determine the version\n"
			              "  as 0.6.0, and \"0.3.3\" would be returned for\n"
			              "  \":version\". Both versions would be print\n"
			              "  if only \"version\" was invoked, each result\n"
			              "  coming from a different process.\n\n"
			              "COLORS\n\n"
			              "  Whenever possible, the background process will\n"
			              "  have its output marked by a non-default color\n"
			              "  (usually red).\n\n"
			              "TROUBLESHOOTING\n\n"
			              "  If the background version appears to be silent,\n"
			              "  make sure it is calling fflush(NULL) before\n"
			              "  reading input.\n\n"
			              "SEE ALSO\n\n"
			              "  codebug.1");

		} // If commands were not hooked up
	}

	if (_debug_step_running)
		_debug_step_running--;

	while (_debugstate_valid) {
		int skipfirst = 0;
		const char *commandstring;
#ifdef WANT_CONSOLE
		char *input;
#endif

		// Suspend music playing
		sfx_suspend(&s->sound, 1);

#ifdef WANT_CONSOLE
		if (!have_windowed) {
			con_gfx_show(s->gfx_state);
			input = con_gfx_read(s->gfx_state);
			con_gfx_hide(s->gfx_state);
			commandstring = input;
			sciprintf("> %s\n", commandstring);
		} else
#endif
			commandstring = _debug_get_input();

		// Check if a specific destination has been given
		if (commandstring && (commandstring[0] == '.' || commandstring[0] == ':'))
			skipfirst = 1;

		if (commandstring && commandstring[0] != ':')
			con_parse(s, commandstring + skipfirst);
		sciprintf("\n");

		// Resume music playing
		sfx_suspend(&s->sound, 0);
	}
}

} // End of namespace Sci
