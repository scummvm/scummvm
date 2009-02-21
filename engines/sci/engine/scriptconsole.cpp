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

/* Second half of the console implementation: VM dependent stuff */
/* Remember, it doesn't have to be fast. */

#include "sci/include/sci_memory.h"
#include "sci/include/engine.h"
#ifdef SCI_CONSOLE

state_t *con_gamestate = NULL;

// console commands

static int c_version(struct _state *s); // displays the package and version number
static int c_list(struct _state *s); // lists various types of things
static int c_man(struct _state *s); // 'manual page'
static int c_set(struct _state *s); // sets an int variable
static int c_print(struct _state *s); // prints a variable
static int c_size(struct _state *s); // displays the size of a resource
static int c_dump(struct _state *s); // gives a hex dump of a resource
//static int c_objinfo(struct _state *s); // shows some info about one class
//static int c_objmethods(struct _state *s); // Disassembles all methods of a class
static int c_hexgrep(struct _state *s); // Searches a string in one resource or resource class
static int c_selectornames(struct _state *s); // Displays all selector names
static int c_kernelnames(struct _state *s); // Displays all kernel function names
static int c_dissectscript(struct _state *s); // Splits a script into objects and explains them

typedef struct {
	const char *name;
	const char *description;
} cmd_mm_entry_t; // All later structures must "extend" this

typedef cmd_mm_entry_t cmd_page_t; // Simple info page

typedef struct {
	const char *name;
	const char *description;
	int (*command)(state_t *);
	const char *param;
} cmd_command_t;

typedef struct {
	const char *name;
	const char *description;
	union {
		int *intp;
		char **charpp;
		reg_t *reg;
	} var;
} cmd_var_t;


typedef void printfunc_t(cmd_mm_entry_t *data, int full);

typedef struct {
	const char *name;
	void *data; // cmd_mm_entry_t
	size_t size_per_entry;
	printfunc_t *print;
	int entries; // Number of used entries
	int allocated;  // Number of allocated entries
} cmd_mm_struct_t;

#define CMD_MM_ENTRIES 3 // command console memory and manual page manager
#define CMD_MM_DEFAULT_ALLOC 4 // Number of table entries to allocate per default

#define CMD_MM_CMD 0 // Commands
#define CMD_MM_VAR 1 // Variables
#define CMD_MM_DOC 2 // Misc. documentation

static const char *cmd_mm_names[CMD_MM_ENTRIES] = {
	"Commands",
	"Variables",
	"Documentation"
};
static size_t cmd_mm_sizes_per_entry[CMD_MM_ENTRIES] = {
	sizeof(cmd_command_t),
	sizeof(cmd_var_t),
	sizeof(cmd_page_t)
};

static void _cmd_print_command(cmd_mm_entry_t *data, int full);
static void _cmd_print_var(cmd_mm_entry_t *data, int full);
static void _cmd_print_page(cmd_mm_entry_t *data, int full);

static printfunc_t *cmd_mm_printers[CMD_MM_ENTRIES] = {
	_cmd_print_command,
	_cmd_print_var,
	_cmd_print_page
};

static cmd_mm_struct_t cmd_mm[CMD_MM_ENTRIES];

static int _cmd_initialized = 0;
static int _lists_need_sorting = 0;

unsigned int cmd_paramlength;
cmd_param_t *cmd_params;

void _cmd_exit() {
	int t;

	for (t = 0; t < CMD_MM_ENTRIES; t++)
		free(cmd_mm[t].data);
}

static cmd_mm_entry_t *cmd_mm_find(char *name, int type) {
	int i;

	for (i = 0; i < cmd_mm[type].entries; i++)
		if (!strcmp(((cmd_mm_entry_t *)((byte *)cmd_mm[type].data + i * cmd_mm[type].size_per_entry))->name, name))
			return ((cmd_mm_entry_t *)((byte *)cmd_mm[type].data + i * cmd_mm[type].size_per_entry));

	return NULL;
}

static int _cmd_mm_comp(const void *a, const void *b) {
	return strcmp(((cmd_mm_entry_t *) a)->name, ((cmd_mm_entry_t *) b)->name);
}

void con_sort_all() {
	int i;

	for (i = 0; i < CMD_MM_ENTRIES; i++)
		if (cmd_mm[i].entries && _lists_need_sorting & (1 << i))
			qsort(cmd_mm[i].data, cmd_mm[i].entries, cmd_mm[i].size_per_entry, _cmd_mm_comp);

	_lists_need_sorting = 0;
}

void con_init() {
	if (!_cmd_initialized) {
		int i;

		_cmd_initialized = 1;
		for (i = 0; i < CMD_MM_ENTRIES; i++) {
			cmd_mm[i].name = cmd_mm_names[i];
			cmd_mm[i].size_per_entry = cmd_mm_sizes_per_entry[i];
			cmd_mm[i].entries = 0;
			cmd_mm[i].allocated = CMD_MM_DEFAULT_ALLOC;
			cmd_mm[i].data = sci_calloc(cmd_mm[i].allocated, cmd_mm[i].size_per_entry);
			cmd_mm[i].print = cmd_mm_printers[i];
		}

		atexit(_cmd_exit);

		// Hook up some commands
		con_hook_command(&c_version, "version", "", "Displays the version number");
		con_hook_command(&c_list, "list", "s*", "Lists various things (try 'list')");
		con_hook_command(&c_man, "man", "s", "Gives a short description of something");
		con_hook_command(&c_print, "print", "s", "Prints an int variable");
		con_hook_command(&c_set, "set", "si", "Sets an int variable");
		con_hook_command(&c_size, "size", "si", "Displays the size of a resource");
		con_hook_command(&c_dump, "dump", "si", "HexDumps a resource");
		con_hook_command(&c_hexgrep, "hexgrep", "shh*", "Searches some resources for a\n"
		                 "  particular sequence of bytes, re-\n  presented as hexadecimal numbers.\n\n"
		                 "EXAMPLES:\n  hexgrep script e8 03 c8 00\n  hexgrep pic.042 fe");
		con_hook_command(&c_dissectscript, "dissectscript", "i", "Examines a script.");

		con_hook_page("addresses", "Passing address parameters\n\n"
		              "  Address parameters may be passed in one of\n"
		              "  three forms:\n"
		              "  - ssss:oooo -- where 'ssss' denotes a\n"
		              "    segment and 'oooo' an offset. Example:\n"
		              "    \"a:c5\" would address something in seg-\n"
		              "    ment 0xa at offset 0xc5.\n"
		              "  - &scr:oooo -- where 'scr' is a script number\n"
		              "    and oooo an offset within that script; will\n"
		              "    fail if the script is not currently loaded\n"
		              "  - $REG -- where 'REG' is one of 'PC', 'ACC',\n"
		              "    'PREV' or 'OBJ': References the address\n"
		              "    indicated by the register of this name.\n"
		              "  - $REG+n (or -n) -- Like $REG, but modifies\n"
		              "    the offset part by a specific amount (which\n"
		              "    is specified in hexadecimal).\n"
		              "  - ?obj -- Looks up an object with the specified\n"
		              "    name, uses its address. This will abort if\n"
		              "    the object name is ambiguous; in that case,\n"
		              "    a list of addresses and indices is provided.\n"
		              "    ?obj.idx may be used to disambiguate 'obj'\n"
		              "    by the index 'idx'.\n");

		con_hook_int(&con_passthrough, "con_passthrough", "scicon->stdout passthrough");
	}
}

static inline int clone_is_used(clone_table_t *t, int idx) {
	return ENTRY_IS_VALID(t, idx);
}

int parse_reg_t(state_t *s, const char *str, reg_t *dest) { // Returns 0 on success
	int rel_offsetting = 0;
	const char *offsetting = NULL;
	// Non-NULL: Parse end of string for relative offsets
	char *endptr;

	if (!s) {
		sciprintf("Addresses can only be parsed if a global state is present");
		return 1; // Requires a valid state
	}

	if (*str == '$') { // Register
		rel_offsetting = 1;

		if (!scumm_strnicmp(str + 1, "PC", 2)) {
			*dest = s->execution_stack[s->execution_stack_pos].addr.pc;
			offsetting = str + 3;
		} else if (!scumm_strnicmp(str + 1, "P", 1)) {
			*dest = s->execution_stack[s->execution_stack_pos].addr.pc;
			offsetting = str + 2;
		} else if (!scumm_strnicmp(str + 1, "PREV", 4)) {
			*dest = s->r_prev;
			offsetting = str + 5;
		} else if (!scumm_strnicmp(str + 1, "ACC", 3)) {
			*dest = s->r_acc;
			offsetting = str + 4;
		} else if (!scumm_strnicmp(str + 1, "A", 1)) {
			*dest = s->r_acc;
			offsetting = str + 2;
		} else if (!scumm_strnicmp(str + 1, "OBJ", 3)) {
			*dest = s->execution_stack[s->execution_stack_pos].objp;
			offsetting = str + 4;
		} else if (!scumm_strnicmp(str + 1, "O", 1)) {
			*dest = s->execution_stack[s->execution_stack_pos].objp;
			offsetting = str + 2;
		} else
			return 1; // No matching register

		if (!*offsetting)
			offsetting = NULL;
		else if (*offsetting != '+' && *offsetting != '-')
			return 1;
	} else if (*str == '&') {
		int script_nr;
		// Look up by script ID
		char *colon = (char *)strchr(str, ':');

		if (!colon)
			return 1;
		*colon = 0;
		offsetting = colon + 1;

		script_nr = strtol(str + 1, &endptr, 10);

		if (*endptr)
			return 1;

		dest->segment = sm_seg_get(&s->seg_manager, script_nr);

		if (!dest->segment) {
			return 1;
		}
	} else if (*str == '?') {
		int index = -1;
		int times_found = 0;
		char *tmp;
		const char *str_objname;
		char *str_suffix;
		char suffchar = 0;
		int i;
		// Parse obj by name

		tmp = (char *)strchr(str, '+');
		str_suffix = (char *)strchr(str, '-');
		if (tmp < str_suffix)
			str_suffix = tmp;
		if (str_suffix) {
			suffchar = (*str_suffix);
			*str_suffix = 0;
		}

		tmp = (char *)strchr(str, '.');

		if (tmp) {
			*tmp = 0;
			index = strtol(tmp + 1, &endptr, 16);
			if (*endptr)
				return -1;
		}

		str_objname = str + 1;

		// Now all values are available; iterate over all objects.
		for (i = 0; i < s->seg_manager.heap_size; i++) {
			mem_obj_t *mobj = s->seg_manager.heap[i];
			int idx = 0;
			int max_index = 0;

			if (mobj) {
				if (mobj->type == MEM_OBJ_SCRIPT)
					max_index = mobj->data.script.objects_nr;
				else if (mobj->type == MEM_OBJ_CLONES)
					max_index = mobj->data.clones.max_entry;
			}

			while (idx < max_index) {
				int valid = 1;
				object_t *obj = NULL;
				reg_t objpos;
				objpos.offset = 0;
				objpos.segment = i;

				if (mobj->type == MEM_OBJ_SCRIPT) {
					obj = mobj->data.script.objects + idx;
					objpos.offset = obj->pos.offset;
				} else if (mobj->type == MEM_OBJ_CLONES) {
					obj = &(mobj->data.clones.table[idx].entry);
					objpos.offset = idx;
					valid = clone_is_used(&mobj->data.clones, idx);
				}

				if (valid) {
					char *objname = (char *) obj->base
					                + obj->variables[SCRIPT_NAME_SELECTOR].offset;
					if (!strcmp(objname, str_objname)) {
						// Found a match!
						if (index < 0 ||
						        times_found == index)
							*dest = objpos;
						else if (times_found < 0 && index) {
							if (index == 1) {
								// First time we realized the ambiguity
								sciprintf("Ambiguous:\n");
								sciprintf("  %3x: ["PREG"] %s\n", 0, PRINT_REG(*dest), str_objname);
							}
							sciprintf("  %3x: ["PREG"] %s\n", index, PRINT_REG(objpos), str_objname);
						}
						++times_found;
					}
				}
				++idx;
			}

		}

		if (!times_found)
			return 1;

		if (times_found > 1 && index < 0) {
			sciprintf("Ambiguous: Aborting.\n");
			return 1; // Ambiguous
		}

		if (times_found <= index)
			return 1; // Not found

		offsetting = str_suffix;
		if (offsetting)
			*str_suffix = suffchar;
		rel_offsetting = 1;
	} else {
		char *colon = (char *)strchr(str, ':');

		if (!colon) {
			offsetting = str;
			dest->segment = 0;
		} else {
			*colon = 0;
			offsetting = colon + 1;

			dest->segment = strtol(str, &endptr, 16);
			if (*endptr)
				return 1;
		}
	}
	if (offsetting) {
		int val = strtol(offsetting, &endptr, 16);

		if (rel_offsetting)
			dest->offset += val;
		else
			dest->offset = val;

		if (*endptr)
			return 1;
	}

	return 0;
}

void con_parse(state_t *s, const char *command) {
	int quote = 0;		// quoting?
	int done = 0;		// are we done yet?
	int cdone = 0;		// Done with the current command?
	const char *paramt;	// parameter types
	char *cmd = (command && command[0]) ? (char *)sci_strdup(command) : (char *)sci_strdup(" ");
	char *_cmd = cmd;
	int pos = 0;

	if (!_cmd_initialized)
		con_init();

	while (!done) {
		cmd_command_t *command_todo;
		int onvar = 1;		// currently working on a variable?
		unsigned int parammem = 0;
		unsigned int i;
		cdone = 0;
		pos = 0;

		//cmd_params = sci_realloc(cmd_params, parammem);
		cmd_paramlength = 0;

		while (*cmd == ' ')
			cmd++;

		while (!cdone) {
			switch (cmd[pos]) {
			case 0:
				done = 1;
			case ';':
				if (!quote)
					cdone = 1;
			case ' ':
				if (!quote)
					cmd[pos] = onvar = 0;
				break;
			case '\\':		// don't check next char for special meaning
				memmove(cmd + pos, cmd + pos + 1, strlen(cmd + pos) - 1);
				break;
			case '"':
				quote ^= 1;
				memmove(cmd + pos, cmd + pos + 1, strlen(cmd + pos));
				pos--;
				break;
			default:
				if (!onvar) {
					onvar = 1;
					if (cmd_paramlength == parammem)
						cmd_params = (cmd_param_t*)sci_realloc(cmd_params, sizeof(cmd_param_t)* (parammem += 8));
					cmd_params[cmd_paramlength].str = cmd + pos;
					cmd_paramlength++;
				}
				break;
			}
			pos++;
		}

		if (quote)
			sciprintf("unbalanced quotes\n");
		else if (strcmp(cmd, "") != 0) {
			command_todo = (cmd_command_t *) cmd_mm_find(cmd, CMD_MM_CMD);
			if (!command_todo)
				sciprintf("%s: not found\n", cmd);
			else {
				unsigned int minparams;
				int need_state = 0;

				paramt = command_todo->param;
				if (command_todo->param[0] == '!') {
					need_state = 1;
					paramt++;
				}

				minparams = strlen(paramt);

				if ((paramt[0] != 0) && (paramt[strlen(paramt) - 1] == '*'))
					minparams -= 2;

				if (cmd_paramlength < minparams)
					sciprintf("%s: needs more than %d parameters\n", cmd, cmd_paramlength);

				else if ((cmd_paramlength > strlen(paramt)) && ((strlen(paramt) == 0) || paramt[strlen(paramt) - 1] != '*'))
					sciprintf("%s: too many parameters", cmd);
				else {
					int do_execute = !need_state || s; // /me wants an implication arrow
					char paramtype;
					int paramtypepos = 0;
					char *endptr;

					for (i = 0; i < cmd_paramlength; i++) {
						paramtype = paramt[paramtypepos];

						if ((paramt[paramtypepos + 1]) && (paramt[paramtypepos + 1] != '*'))
							paramtypepos++;
						// seek next param type unless end of string or '*						   '

						switch (paramtype) {
							// Now turn the parameters into variables of the appropriate types,
							// unless they're strings, and store them in the global cmd_params[]
							// structure

						case 'a': {
							char *oldname = cmd_params[i].str;
							if (parse_reg_t(s, oldname, &(cmd_params[i].reg))) {
								sciprintf("%s: '%s' is not an address or object\n", cmd, oldname);
								do_execute = 0;
							}
							break;
						}

						case 'i': {
							char *orgstr = cmd_params[i].str;

							cmd_params[i].val = strtol(orgstr, &endptr, 0);
							if (*endptr != '\0') {
								do_execute = 0;
								sciprintf("%s: '%s' is not an int\n", cmd, orgstr);
							}
						}
						break;

						case 'h': {
							char *orgstr = cmd_params[i].str;

							cmd_params[i].val = strtol(orgstr, &endptr, 16);

							if (*endptr != '\0') {
								do_execute = 0;
								sciprintf("%s: '%s' is not a hex number\n", cmd, orgstr);
							}

							cmd_params[i].val &= 0xff;	// Clip hex numbers to 0x00 ... 0xff
						}
						break;

						case 's':
							break;

						default:
							error("Internal error: Heap corruption or prior assertion failed:\n"
							        "Unknown parameter type '%c' for funtion\n", paramtype);

						}
					}

					if (do_execute) {
						command_todo->command(s);
					} else
						error("Skipping command...\n");
				}
			}
		}
		cmd += pos;
	}

	free(_cmd);
	if (cmd_params)
		free(cmd_params);
	cmd_params = NULL;
}

/* (unused)
static cmd_mm_entry_t *con_iterate_entry(int ID, int *counter) {
	byte *retval;
	con_init();

	if (*counter >= cmd_mm[ID].entries)
		return 0;
	retval = cmd_mm[ID].data;
	retval += (*counter) * cmd_mm[ID].size_per_entry;

	(*counter)++;

	return (cmd_mm_entry_t *)retval;
}*/

static cmd_mm_entry_t *con_alloc_page_entry(int ID) {
	int entry;

	con_init();

	if (cmd_mm[ID].entries >= cmd_mm[ID].allocated) {
		int nextsize = cmd_mm[ID].allocated;
		if (nextsize >= 64)
			nextsize += 16;
		else
			nextsize <<= 1;

		cmd_mm[ID].data = sci_realloc(cmd_mm[ID].data, nextsize * cmd_mm[ID].size_per_entry);
		cmd_mm[ID].allocated = nextsize;
	}

	_lists_need_sorting |= (1 << ID);

	entry = cmd_mm[ID].entries++;
	return (cmd_mm_entry_t *)(((byte *)cmd_mm[ID].data) + entry * cmd_mm[ID].size_per_entry);
}

int con_hook_page(const char *name, const char *body) {
	cmd_page_t *page = (cmd_page_t *)con_alloc_page_entry(CMD_MM_DOC);

	page->name = name;
	page->description = body;

	return 0;
}

int con_hook_command(int command(state_t *), const char *name, const char *param, const char *description) {
	cmd_command_t *cmd = NULL;
	unsigned int i;

	if (NULL == name) {
		sciprintf("console.c: con_hook_command(): NULL passed for name\n");
		return -1;
	}

	if (command == NULL)
		return 1;

	if (param == NULL)
		param = "";

	if (description == NULL)
		description = "";

	i = 0;
	while (param[i] != 0) {
		switch (param[i]) {
		case '*':
			if (param[i + 1] != 0)
				return 1;
			if (i == 0)
				return 1;
		case 'h':
		case '!':
		case 'i':
		case 'a':
		case 's':
		case 'r':
			break;
		default:
			return 1;
		}
		i++;
	}
	cmd = (cmd_command_t *)con_alloc_page_entry(CMD_MM_CMD);

	cmd->command = command;
	cmd->name = name;
	cmd->param = param;
	cmd->description = description;

	return 0;
}

int con_hook_int(int *pointer, const char *name, const char *description) {
	cmd_var_t *var;

	if (pointer == NULL)
		return 1;

	if (description == NULL)
		description = "";

	var = (cmd_var_t *) con_alloc_page_entry(CMD_MM_VAR);

	var->var.intp = pointer;
	var->name = name;
	var->description = description;

	return 0;
}

// Console commands and support functions

static int get_resource_number(char *resid) {
	// Gets the resource number of a resource string, or returns -1
	int i, res = -1;

	for (i = 0; i < sci_invalid_resource; i++)
		if (strcmp(sci_resource_types[i], resid) == 0)
			res = i;

	return res;
}

static int c_version(state_t * s) {
	if (NULL == s) {
		sciprintf("console.c: c_version: NULL passed for parameter s\n");
		return -1;
	}

	sciprintf("FreeSCI, version " VERSION "\n");
	sciprintf("Resource file version:        %s\n", sci_version_types[s->resmgr->sci_version]);
	sciprintf("Emulated interpreter version: %d.%03d.%03d\n", SCI_VERSION_MAJOR(s->version),
	          SCI_VERSION_MINOR(s->version), SCI_VERSION_PATCHLEVEL(s->version));

	return 0;
}

static int c_list_words(state_t *s) {
	word_t **words;
	int words_nr;
	int i;

	words = vocab_get_words(s->resmgr, &words_nr);

	if (!words) {
		sciprintf("No vocabulary.\n");
		return 1;
	}

	for (i = 0; i < words_nr; i++)
		sciprintf("%4d: %03x [%03x] %s\n", i, words[i]->w_class, words[i]->group, words[i]->word);

	vocab_free_words(words, words_nr);
	return 0;
}

int c_list_suffices(state_t *s) {
	suffix_t **suffices;
	int suffices_nr;
	int i;
	char word_buf[256], alt_buf[256];

	suffices = vocab_get_suffices(s->resmgr, &suffices_nr);

	if (!suffices) {
		sciprintf("No suffix vocabulary.\n");
		return 1;
	}

	for (i = 0; i < suffices_nr; i++) {
		suffix_t *suf = suffices[i];

		strncpy(word_buf, suf->word_suffix, suf->word_suffix_length);
		word_buf[suf->word_suffix_length] = 0;
		strncpy(alt_buf, suf->alt_suffix, suf->alt_suffix_length);
		alt_buf[suf->alt_suffix_length] = 0;

		sciprintf("%4d: (%03x) -%12s  =>  -%12s (%03x)\n", i, suf->class_mask, word_buf, alt_buf, suf->result_class);
	}

	vocab_free_suffices(s->resmgr, suffices, suffices_nr);

	return 0;
}

static void _cmd_print_command(cmd_mm_entry_t *data, int full) {
	const char *paramseeker = ((cmd_command_t *)data)->param;

	if (full) {
		sciprintf("SYNOPSIS\n\n  %s (%s) ", data->name, paramseeker);

		while (*paramseeker) {
			switch (*paramseeker) {
			case '!':
				break;
			case 'i':
				sciprintf(" (int)");
				break;
			case 'a':
				sciprintf(" (addr)");
				break;
			case 's':
				sciprintf(" (string)");
				break;
			case 'h':
				sciprintf(" (hexbyte)");
				break;
			case '*':
				sciprintf("*");
				break;
			default:
				sciprintf(" (Unknown(%c))", *paramseeker);
			}
			paramseeker++;
		}

		sciprintf("\n\nDESCRIPTION\n\n  %s", data->description);
	} else
		sciprintf(" %s", data->name);
}

static void _cmd_print_var(cmd_mm_entry_t *data, int full) {
	cmd_var_t *var = (cmd_var_t *) data;
	if (full)
		sciprintf("VALUE\n\n");
	sciprintf("  %s = %d\n", var->name, *(var->var.intp));

	if (full)
		sciprintf("\n\nDESCRIPTION\n\n  %s", data->description);
}

static void _cmd_print_page(cmd_mm_entry_t *data, int full) {
	if (full)
		sciprintf("\n\nDESCRIPTION\n\n  %s\n", data->description);
	else
		sciprintf("%s\n", data->name);
}

static int c_list(state_t *s) {
	if (_lists_need_sorting)
		con_sort_all();

	if (cmd_paramlength == 0) {
		sciprintf("usage: list [type]\nwhere type is one of the following:\n"
		          "cmds       - lists all commands\n"
		          "vars       - lists all variables\n"
		          "docs       - lists all misc. documentation\n"
		          "\n"
		          "restypes   - lists all resource types\n"
		          "selectors  - lists all selectors\n"
		          "syscalls   - lists all kernel functions\n"
		          "words      - lists all kernel words\n"
		          "suffixes   - lists all suffix replacements\n"
		          "[resource] - lists all [resource]s");
	} else if (cmd_paramlength == 1) {
		const char *mm_subsects[3] = {"cmds", "vars", "docs"};
		int mm_found = -1;
		int i;

		for (i = 0; i < 3; i++)
			if (mm_subsects[i] && !strcmp(mm_subsects[i], cmd_params[0].str))
				mm_found = i;

		if (mm_found >= 0)
			for (i = 0; i < cmd_mm[mm_found].entries; i++)
				cmd_mm[mm_found].print((cmd_mm_entry_t *)(((byte *)cmd_mm[mm_found].data) + i * cmd_mm[mm_found].size_per_entry), 0);
		else {
			if (!s) {
				sciprintf("You need a state to do that");
				return 1;
			}

			if (!strcmp("selectors", cmd_params[0].str))
				return c_selectornames(s);
			else if (!strcmp("syscalls", cmd_params[0].str))
				return c_kernelnames(s);
			else if (!strcmp("suffixes", cmd_params[0].str) || !strcmp("suffices", cmd_params[0].str) || !strcmp("sufficos", cmd_params[0].str))
				// sufficos: Accusative Plural of 'suffix'
				return c_list_suffices(s);
			else if (!strcmp("words", cmd_params[0].str))
				return c_list_words(s);
			else if (strcmp("restypes", cmd_params[0].str) == 0) {
				for (i = 0; i < sci_invalid_resource; i++)
					sciprintf("%s\n", sci_resource_types[i]);
			} else {
				int res = get_resource_number(cmd_params[0].str);
				if (res == -1)
					sciprintf("Unknown resource type: '%s'\n", cmd_params[0].str);
				else {
					for (i = 0; i < sci_max_resource_nr[s->resmgr->sci_version]; i++)
						if (scir_test_resource(s->resmgr, res, i))
							sciprintf("%s.%03d\n", sci_resource_types[res], i);
				}
			}
		}
	} else
		sciprintf("list can only be used with one argument");
	return 0;
}

static int c_man(state_t *s) {
	int section = 0;
	unsigned int i;
	char *name = cmd_params[0].str;
	char *c = strchr(name, '.');
	cmd_mm_entry_t *entry = 0;

	if (c) {
		*c = 0;
		section = atoi(c + 1);
	}

	if (section < 0 || section >= CMD_MM_ENTRIES) {
		sciprintf("Invalid section %d\n", section);
		return 1;
	}

	sciprintf("section:%d\n", section);
	if (section)
		entry = cmd_mm_find(name, section - 1);
	else
		for (i = 0; i < CMD_MM_ENTRIES && !section; i++) {
			if ((entry = cmd_mm_find(name, i)))
				section = i + 1;
		}

	if (!entry) {
		sciprintf("No manual entry\n");
		return 1;
	}

	sciprintf("-- %s: %s.%d\n", cmd_mm[section - 1].name, name, section);
	cmd_mm[section - 1].print(entry, 1);

	return 0;
}

static int c_set(state_t *s) {
	cmd_var_t *var = (cmd_var_t *) cmd_mm_find(cmd_params[0].str, CMD_MM_VAR);

	if (var)
		*(var->var.intp) = cmd_params[1].val;

	return 0;
}

static int c_print(state_t *s) {
	cmd_var_t *var = (cmd_var_t *) cmd_mm_find(cmd_params[0].str, CMD_MM_VAR);

	if (var)
		sciprintf("%d", *(var->var.intp));
	else
		sciprintf("Not defined.");

	return 0;
}

static int c_size(state_t *s) {
	int res = get_resource_number(cmd_params[0].str);
	if (res == -1)
		sciprintf("Resource type '%s' is not valid\n", cmd_params[0].str);
	else {
		resource_t *resource = scir_find_resource(s->resmgr, res, cmd_params[1].val, 0);
		if (resource) {
			sciprintf("Size: %d\n", resource->size);
		} else
			sciprintf("Resource %s.%03d not found\n", cmd_params[0].str, cmd_params[1].val);
	}

	return 0;
}

static int c_dump(state_t *s) {
	int res = get_resource_number(cmd_params[0].str);

	if (res == -1)
		sciprintf("Resource type '%s' is not valid\n", cmd_params[0].str);
	else {
		resource_t *resource = scir_find_resource(s->resmgr, res, cmd_params[1].val, 0);
		if (resource)
			sci_hexdump(resource->data, resource->size, 0);
		else
			sciprintf("Resource %s.%03d not found\n", cmd_params[0].str, cmd_params[1].val);
	}

	return 0;
}

static int c_hexgrep(state_t *s) {
	int i, seeklen, resnr, restype, resmax;
	unsigned char *seekstr = NULL;
	resource_t *script = NULL;
	char *dot = strchr(cmd_params[0].str, '.');

	if (NULL == s) {
		error("console.c: c_hexgrep(): NULL passed for s\r\n");
		return(-1);
	}

	seekstr = (unsigned char *)sci_malloc(seeklen = (cmd_paramlength - 1));

	if (NULL == seekstr) {
		error("console.c: c_hexgrep(): malloc failed for seekstr\r\n");
		return(-1);
	}

	for (i = 0; i < seeklen; i++)
		seekstr[i] = (byte)cmd_params[i + 1].val;

	if (dot) {
		*dot = 0;
		resmax = resnr = atoi(dot + 1);
	} else {
		resnr = 0;
		resmax = 999;
	}

	if ((restype = get_resource_number(cmd_params[0].str)) == -1) {
		sciprintf("Unknown resource type \"%s\"\n", cmd_params[0].str);
		free(seekstr);
		return 1;
	}

	for (; resnr <= resmax; resnr++)
		if ((script = scir_find_resource(s->resmgr, restype, resnr, 0))) {
			unsigned int seeker = 0, seekerold = 0;
			int comppos = 0;
			int output_script_name = 0;

			while (seeker < script->size) {
				if (script->data[seeker] == seekstr[comppos]) {
					if (comppos == 0)
						seekerold = seeker;

					comppos++;

					if (comppos == seeklen) {
						comppos = 0;
						seeker = seekerold + 1;

						if (!output_script_name) {
							sciprintf("\nIn %s.%03d:\n", sci_resource_types[restype], resnr);
							output_script_name = 1;
						}
						sciprintf("   0x%04x\n", seekerold);
					}
				} else
					comppos = 0;

				seeker++;
			}
		}

	free(seekstr);

	return 0;
}

static int c_selectornames(state_t * s) {
	int namectr;
	char **snames	= NULL;
	int seeker = 0;

	if (NULL == s) {
		sciprintf("console.c: c_selectornames(): NULL passed for parameter s\n");
		return -1;
	}

	snames = vocabulary_get_snames(s->resmgr, &namectr, s ? s->version : 0);

	if (!snames) {
		sciprintf("No selector name table found");
		return 1;
	}

	sciprintf("Selector names in numeric order:\n");
	while (snames[seeker]) {
		sciprintf("%03x: %s\n", seeker, snames[seeker]);
		seeker++;
	}
	vocabulary_free_snames(snames);

	return 0;
}

static int c_kernelnames(state_t * s) {
	int knamectr;
	char **knames = vocabulary_get_knames(s->resmgr, &knamectr);
	int seeker = 0;

	if (NULL == s) {
		sciprintf("console.c: c_kernelnames NULL passed for parameter s\n");
		return -1;
	}

	if (!knames) {
		sciprintf("No kernel name table found");
		return 1;
	}

	sciprintf("Syscalls in numeric order:\n");
	for (seeker = 0; seeker < knamectr; seeker++)
		sciprintf("%03x: %s\n", seeker, knames[seeker]);

	vocabulary_free_knames(knames);

	return 0;
}

static int c_dissectscript(state_t * s) {
	if (NULL == s) {
		sciprintf("console.c: c_dissectscript(): NULL passed for parameter s\n");
		return -1;
	}

	script_dissect(s->resmgr, cmd_params[0].val, s->selector_names, s->selector_names_nr);
	return 0;
}

#endif // SCI_CONSOLE
