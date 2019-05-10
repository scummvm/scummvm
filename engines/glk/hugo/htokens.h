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

#ifndef GLK_HUGO_HTOKENS
#define GLK_HUGO_HTOKENS

namespace Glk {
namespace Hugo {

/*
 * This file contains token definitions for the Hugo Compiler and Engine
 * The enum constants of type TOKEN_T reflect the token names given in the token array.
 * Token names followed by a # are for system use.
 */

/* i.e., highest numbered token */
#define TOKENS 0x7B

/* arbitrary */
#define HASH_KEY 1023

enum TOKEN_T {
	/* 0x00 - 0x0f */
	NULL_T, OPEN_BRACKET_T, CLOSE_BRACKET_T, DECIMAL_T,
	COLON_T, EQUALS_T, MINUS_T, PLUS_T,
	ASTERISK_T, FORWARD_SLASH_T, PIPE_T, SEMICOLON_T,
	OPEN_BRACE_T, CLOSE_BRACE_T, OPEN_SQUARE_T, CLOSE_SQUARE_T,

	/* 0x10 - 0x1f */
	POUND_T, TILDE_T, GREATER_EQUAL_T, LESS_EQUAL_T,
	NOT_EQUAL_T, AMPERSAND_T, GREATER_T, LESS_T,
	IF_T, COMMA_T, ELSE_T, ELSEIF_T,
	WHILE_T, DO_T, SELECT_T, CASE_T,
	
	/* 0x20 - 0x2f */
	FOR_T, RETURN_T, BREAK_T, AND_T,
	OR_T, JUMP_T, RUN_T, IS_T,
	NOT_T, TRUE_T, FALSE_T, LOCAL_T,
	VERB_T, XVERB_T, HELD_T, MULTI_T,
	
	/* 0x30 - 0x3f */
	MULTIHELD_T, NEWLINE_T, ANYTHING_T, PRINT_T,
	NUMBER_T, CAPITAL_T, TEXT_T, GRAPHICS_T, 
	COLOR_T, REMOVE_T, MOVE_T, TO_T,
	PARENT_T, SIBLING_T, CHILD_T, YOUNGEST_T,

	/* 0x40 - 0x4f */
	ELDEST_T, YOUNGER_T, ELDER_T, PROP_T,
	ATTR_T, VAR_T, DICTENTRY_T, TEXTDATA_T,
	ROUTINE_T, DEBUGDATA_T, OBJECTNUM_T, VALUE_T,
	EOL_T, SYSTEM_T, NOTHELD_T, MULTINOTHELD_T,

	/* 0x50 - 0x5f */
	WINDOW_T, RANDOM_T, WORD_T, LOCATE_T,
	PARSE_T, CHILDREN_T, IN_T, PAUSE_T,
	RUNEVENTS_T, ARRAYDATA_T, CALL_T, STRINGDATA_T,
	SAVE_T, RESTORE_T, QUIT_T, INPUT_T,

	/* 0x60 - 0x6f */
	SERIAL_T, CLS_T, SCRIPTON_T, SCRIPTOFF_T,
	RESTART_T, HEX_T, OBJECT_T, XOBJECT_T,
	STRING_T, ARRAY_T, PRINTCHAR_T, UNDO_T,
	DICT_T, RECORDON_T, RECORDOFF_T, WRITEFILE_T,
	
	/* 0x70 - */
	READFILE_T, WRITEVAL_T, READVAL_T, PLAYBACK_T,
	COLOUR_T, PICTURE_T, LABEL_T, SOUND_T,
	MUSIC_T, REPEAT_T, ADDCONTEXT_T, VIDEO_T
};

struct HTokens {
	static const char *const token[];
	static int token_hash[];

	HTokens();
};

} // End of namespace Hugo
} // End of namespace Glk

#endif
