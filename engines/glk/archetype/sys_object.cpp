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

#include "glk/archetype/sys_object.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/game_stat.h"
#include "glk/archetype/heap_sort.h"
#include "glk/archetype/parser.h"
#include "common/algorithm.h"
#include "common/debug-channels.h"
#include "common/savefile.h"

namespace Glk {
namespace Archetype {

enum SysStateType {
	IDLING, INIT_SORTER, OPEN_SORTER, CLOSE_SORTER, NEXT_SORTED, PLAYER_CMD,
	NORMALIZE, ABBR, OPEN_PARSER, VERB_LIST, NOUN_LIST, CLOSE_PARSER, INIT_PARSER,
	WHICH_OBJECT, ROLL_CALL, PRESENT, PARSE, NEXT_OBJECT, DEBUG_MESSAGES,
	DEBUG_EXPRESSIONS, DEBUG_STATEMENTS, DEBUG_MEMORY, FREE_MEMORY, SAVE_STATE, LOAD_STATE
};

const char *const StateLookup[LOAD_STATE + 1] = {
	"IDLING", "INIT SORTER", "OPEN SORTER", "CLOSE SORTER", "NEXT SORTED", "PLAYER CMD",
	"NORMALIZE", "ABBR", "OPEN PARSER", "VERB LIST", "NOUN LIST", "CLOSE PARSER", "INIT PARSER",
	"WHICH OBJECT", "ROLL CALL", "PRESENT", "PARSE", "NEXT OBJECT", "DEBUG MESSAGES",
	"DEBUG EXPRESSIONS", "DEBUG STATEMENTS", "DEBUG MEMORY", "FREE MEMORY", "SAVE STATE", "LOAD STATE"
};

// Global variables which retain the state of the system object between calls
SysStateType sys_state;
TargetListType target_list;

void sys_object_init() {
	sys_state = IDLING;
	target_list = PARSER_VERBLIST;
}

static bool figure_state(const String &s) {
	for (int st = IDLING; st <= LOAD_STATE; ++st) {
		if (StateLookup[st] == s) {
			sys_state = (SysStateType)st;
			return true;
		}
	}

	return false;
}

void send_to_system(int transport, String &strmsg, ResultType &result, ContextType &context) {
	int the_caller;
	int obj_index;
	String nomatch;
    NodePtr np;
	void *p;

	if (g_vm->shouldQuit())
		return;

	if (transport == OP_SEND)
		the_caller = context.self;
	else
		the_caller = context.sender;

	do {
		cleanup(result);

		switch (sys_state) {
		case IDLING:
			if (figure_state(strmsg)) {
				switch (sys_state) {
				case PLAYER_CMD:
				case ABBR:
				case SAVE_STATE:
				case LOAD_STATE:
				case OPEN_PARSER:
				case OPEN_SORTER:
				case WHICH_OBJECT:
					return;				// come back again!

				case INIT_SORTER:
					reinit_heap();
					sys_state = OPEN_SORTER;
					return;

				case INIT_PARSER:
					new_parse_list();
					sys_state = OPEN_PARSER;
					return;

				default:
					break;
				}
			}
			break;

		case PLAYER_CMD:
			normalize_string(strmsg, g_vm->Command);
			sys_state = IDLING;
			break;

		case NORMALIZE:
			// last normalized command
			result._kind = STR_PTR;
			result._data._str.acl_str = NewDynStr(g_vm->Command);
			sys_state = IDLING;
			break;

		case ABBR:
			result._kind = STR_PTR;
			result._data._str.acl_str = NewDynStr(strmsg);

			if (convert_to(NUMERIC, result)) {
				g_vm->Abbreviate = result._data._numeric.acl_int;
			} else {
				error("Warning: non-numeric abbreviation message sent to system");
				cleanup(result);
			}
			sys_state = IDLING;
			break;

		case OPEN_PARSER:
			if (figure_state(strmsg)) {
				switch (sys_state) {
				case CLOSE_PARSER:
					sys_state = IDLING;
					break;

				case VERB_LIST:
					target_list = PARSER_VERBLIST;
					sys_state = OPEN_PARSER;
					break;

				case NOUN_LIST:
					target_list = PARSER_NOUNLIST;
					sys_state = OPEN_PARSER;
					break;

				default:
					break;
				}
			} else {
				add_parse_word(target_list, strmsg, the_caller);
			}

			return;

		case OPEN_SORTER:
			if (figure_state(strmsg)) {
				switch (sys_state) {
				case CLOSE_SORTER:
					sys_state = IDLING;
					break;
				default:
					break;
				}
			} else {
				drop_str_on_heap(strmsg);
			}
			return;

		case NEXT_SORTED:
			if (!pop_heap(p)) {
				cleanup(result);
			} else {
				result._kind = STR_PTR;
				result._data._str.acl_str = (StringPtr)p;
				sys_state = IDLING;
			}
			break;

		case WHICH_OBJECT:
			obj_index = find_object(strmsg);
			if (obj_index != 0) {
				result._kind = IDENT;
				result._data._ident.ident_kind = OBJECT_ID;
				result._data._ident.ident_int = obj_index;
			}
			sys_state = IDLING;
			break;

		case ROLL_CALL:
			dispose_list(g_vm->Proximate);
			new_list(g_vm->Proximate);
			sys_state = IDLING;
			break;

		case PRESENT:
			np = new NodeType();
			np->data = nullptr;
			np->key = the_caller;

			insert_item(g_vm->Proximate, np);
			sys_state = IDLING;
			break;

		case PARSE:
			parse_sentence();
			sys_state = IDLING;
			break;

		case NEXT_OBJECT:
			if (!pop_object(obj_index, nomatch)) {
				cleanup(result);
			} else if (obj_index < 0) {
				result._kind = STR_PTR;
				result._data._str.acl_str = NewDynStr(nomatch);
			} else {
				result._kind = IDENT;
				result._data._ident.ident_kind = OBJECT_ID;
				result._data._ident.ident_int = obj_index;
			}

			sys_state = IDLING;
			break;

		case DEBUG_MESSAGES:
			if (DebugMan.isDebugChannelEnabled(DEBUG_MSGS))
				DebugMan.disableDebugChannel(DEBUG_MSGS);
			else
				DebugMan.enableDebugChannel(DEBUG_MSGS);
			sys_state = IDLING;
			break;

		case DEBUG_EXPRESSIONS:
			if (DebugMan.isDebugChannelEnabled(DEBUG_EXPRESSIONS))
				DebugMan.disableDebugChannel(DEBUG_EXPRESSIONS);
			else
				DebugMan.enableDebugChannel(DEBUG_EXPRESSIONS);
			sys_state = IDLING;
			break;

		case DEBUG_STATEMENTS:
			if (DebugMan.isDebugChannelEnabled(DEBUG_STATEMENTS))
				DebugMan.disableDebugChannel(DEBUG_STATEMENTS);
			else
				DebugMan.enableDebugChannel(DEBUG_STATEMENTS);
			sys_state = IDLING;
			break;

		case DEBUG_MEMORY:
			//debug("Maximum memory request: %d bytes", MaxAvail);
			//debug("Actual free memory:     %d bytes", MemAvail);
			sys_state = IDLING;
			break;

		case FREE_MEMORY:
			result._kind = NUMERIC;
			result._data._numeric.acl_int = 0xffff;		// MemAvail;
			sys_state = IDLING;
			break;

		case SAVE_STATE:
			if (g_vm->saveGame().getCode() != Common::kNoError) {
				g_vm->writeln("Error saving savegame");
				cleanup(result);
			} else {
				result._kind = RESERVED;
				result._data._reserved.keyword = RW_TRUE;
			}

			sys_state = IDLING;
			break;

		case LOAD_STATE: {
			Common::ErrorCode errCode;
			if (g_vm->loadingSavegame())
				errCode = g_vm->loadLauncherSavegame().getCode();
			else
				errCode = g_vm->loadGame().getCode();

			if (errCode == Common::kNoError) {
				result._kind = RESERVED;
				result._data._reserved.keyword = RW_TRUE;

			} else if (errCode == Common::kNoGameDataFoundError) {
				result._kind = RESERVED;
				result._data._reserved.keyword = RW_FALSE;

			} else {
				g_vm->writeln("Error restoring savegame");
				cleanup(result);
			}

			sys_state = IDLING;
			break;
		}

		default:
			break;
		}

		if (g_vm->shouldQuit())
			sys_state = IDLING;
	} while (sys_state != IDLING);
}

} // End of namespace Archetype
} // End of namespace Glk
