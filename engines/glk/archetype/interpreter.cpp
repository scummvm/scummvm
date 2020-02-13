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

#include "glk/archetype/interpreter.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/misc.h"
#include "glk/archetype/saveload.h"
#include "glk/archetype/timestamp.h"

namespace Glk {
namespace Archetype {

int MainObject;

void interpreter_init() {
	MainObject = 0;
}

StringPtr MakeNewDynStr(const String &s) {
	return NewDynStr(s);
}

int find_message(const String &message) {
	void *p;

	for (uint i = 1; i <= g_vm->Vocabulary.size(); ++i) {
		if (!index_xarray(g_vm->Vocabulary, i, p))
			g_vm->writeln("Internal error - cannot index element %d of Vocabulary", i);
		else if (message == *((StringPtr)p))
			return i;
	}

	return 0;
}

bool convert_to(AclType target_type, ResultType &the_scalar) {
	int code;
	char dir_from = '\0';
	int the_number = -1;
	String s1;
	void *p;
    bool boolval = false;

	if (target_type == the_scalar._kind)
		return true;

	switch (target_type) {
	case QUOTE_LIT:
	case TEXT_LIT:
	case IDENT:
	case RESERVED:
		return false;
	default:
		break;
	}

	switch (the_scalar._kind) {
    case NUMERIC:
        dir_from = 'N';
		the_number = the_scalar._data._numeric.acl_int;
		break;

	case MESSAGE:
		dir_from = 'S';
		if (index_xarray(g_vm->Vocabulary, the_scalar._data._msgTextQuote.index, p))
			s1 = *(StringPtr)p;
		break;

	case TEXT_LIT:
	case QUOTE_LIT:
        dir_from = 'S';
		if (index_xarray(g_vm->Literals, the_scalar._data._msgTextQuote.index, p))
			s1 = *(StringPtr)p;
		break;

	case STR_PTR:
		// string memory will be disposed ONLY if successful convert
        dir_from = 'S';
		s1 = *the_scalar._data._str.acl_str;
		break;

	case IDENT:
		//with the_scalar do begin
        dir_from = 'S';
        
		switch (the_scalar._data._ident.ident_kind) {
		case ENUMERATE_ID:
			dir_from = 'N';
			the_number = the_scalar._data._ident.ident_int;
			break;

		case OBJECT_ID:
			if (the_scalar._data._ident.ident_int == 0)
				s1 = "system";
			else if (index_xarray(g_vm->Object_ID_List, the_scalar._data._ident.ident_int, p)) {
				if (p == nullptr)
					s1 = "null";
				else
					s1 = *(StringPtr)p;
			} else {
				return false;
			}
			break;

		case TYPE_ID:
			if (the_scalar._data._ident.ident_int == 0)
				s1 = "null";
			else if (index_xarray(g_vm->Type_ID_List, the_scalar._data._ident.ident_int, p))
				s1 = *(StringPtr)p;
			else
				return false;
			break;

		case ATTRIBUTE_ID:
			if (index_xarray(g_vm->Attribute_ID_List, the_scalar._data._ident.ident_int, p))
				s1 = *((StringPtr)p);
			else
				return false;
			break;

		default:
			break;
		}
		break;

	case RESERVED:
		if (the_scalar._data._reserved.keyword == RW_TRUE || the_scalar._data._reserved.keyword == RW_FALSE) {
			dir_from = 'B';
			boolval = (the_scalar._data._reserved.keyword == RW_TRUE);
		} else {
			return false;
		}
		break;

	default:
		break;
	}

	if (target_type == STR_PTR || target_type == MESSAGE) {
		if (the_scalar._kind == STR_PTR)
			FreeDynStr(the_scalar._data._str.acl_str);	// we know this will succeed

		the_scalar._kind = target_type;

		switch (dir_from) {
		case 'N':
			s1 = String::format("%d", the_number);
			break;
		case 'B':
			s1 = boolval ? "TRUE" : "false";
			break;
		default:
			break;
		}

		if (target_type == MESSAGE)
			the_scalar._data._msgTextQuote.index = find_message(s1);
		else
			the_scalar._data._str.acl_str = NewDynStr(s1);

		return true;
	} else {
		// numeric conversions
		switch (dir_from) {
		case 'N':
			the_scalar._kind = NUMERIC;
			the_scalar._data._numeric.acl_int = the_number;
			return true;

		case 'B':
			the_scalar._kind = NUMERIC;
			the_scalar._data._numeric.acl_int = boolval ? 1 : 0;
			break;

		case  'S':
			s1.trim();
			the_number = s1.val(&code);

			if (code != 0) {
				return false;
			} else {
				// successful
				if (the_scalar._kind == STR_PTR)
					FreeDynStr(the_scalar._data._str.acl_str);		// memory no longer needed

				the_scalar._kind = NUMERIC;
				the_scalar._data._numeric.acl_int = the_number;
			}

			return true;
			
		default:
			break;
		}
	}

	return false;
}

void undefine(ResultType &result) {
	result._kind = RESERVED;
	result._data._reserved.keyword = RW_UNDEFINED;
}

void cleanup(ResultType &result) {
	if (result._kind == STR_PTR)
		FreeDynStr(result._data._str.acl_str);
	result._kind = RESERVED;
	result._data._reserved.keyword = RW_UNDEFINED;
}

void copy_result(ResultType &r1, const ResultType &r2) {
	cleanup(r1);
	r1 = r2;
	if (r1._kind == STR_PTR)
		r1._data._str.acl_str = NewDynStr(*r2._data._str.acl_str);
}

bool result_compare(short comparison, ResultType &r1, ResultType &r2) {
	bool verdict = false;

	// Try numeric reckoning first, then string reckoning
	if (convert_to(NUMERIC, r1) && convert_to(NUMERIC, r2)) {
		switch (comparison) {
		case OP_EQ:
		case OP_NE:
			verdict = r1._data._numeric.acl_int == r2._data._numeric.acl_int;
			break;
		case OP_LT:
			verdict = r1._data._numeric.acl_int <  r2._data._numeric.acl_int;
			break;
		case OP_LE:
			verdict = r1._data._numeric.acl_int <= r2._data._numeric.acl_int;
			break;
		case OP_GT:
			verdict = r1._data._numeric.acl_int >  r2._data._numeric.acl_int;
			break;
		case OP_GE:
			verdict = r1._data._numeric.acl_int >= r2._data._numeric.acl_int;
			break;
		default:
			break;
		}

	} else if (convert_to(STR_PTR, r1) && convert_to(STR_PTR, r2)) {
		switch (comparison) {
		case OP_EQ:
		case OP_NE:
			verdict = *r1._data._str.acl_str == *r2._data._str.acl_str;
			break;
		case OP_LT:
			verdict = *r1._data._str.acl_str < *r2._data._str.acl_str;
			break;
		case OP_LE:
			verdict = *r1._data._str.acl_str <= *r2._data._str.acl_str;
			break;
		case OP_GT:
			verdict = *r1._data._str.acl_str > *r2._data._str.acl_str;
			break;
		case OP_GE:
			verdict = *r1._data._str.acl_str >= *r2._data._str.acl_str;
			break;
		default:
			break;
		}

	} else if (r1._kind == r2._kind) {
		switch (r1._kind) {
		case RESERVED:
			switch (comparison) {
			case OP_EQ:
			case OP_NE:
				verdict = r1._data._reserved.keyword ==  r2._data._reserved.keyword;
				break;
			default:
				break;
			}
			break;

		case IDENT:
			if (r1._data._ident.ident_kind == r2._data._ident.ident_kind) {
				switch (comparison) {
				case OP_EQ:
				case OP_NE:
					verdict = r1._data._ident.ident_int ==  r2._data._ident.ident_int;
					break;
				default:
					break;
				}
			}
			break;

		default:
			break;
		}
	}

	if (comparison == OP_NE)
		return !verdict;
	else
		return verdict;
}

bool assignment(ResultType &target, ResultType &value) {
	ExprPtr e;

	if (target._kind != ATTR_PTR) {
		error("Warning: attempted assignment to a non-attribute");
		return false;
	} else {
		e = (ExprPtr)target._data._attr.acl_attr->data;

		// If the current expression starts with an operator, we know it isn't a flat result
		// and must therefore be disposed of before proceeding.  Otherwise simply clean up
		// the previous expression node
		if (e->_kind != OPER) {
			cleanup(*e);
		} else {
			dispose_expr(e);
			e = new ExprNode();
			undefine(*e);
		}
	}

	copy_result(*e, value);
	target._data._attr.acl_attr->data = e;

	return true;
}

String get_result_string(ResultType &result) {
	ResultType r1;
	String str;

	undefine(r1);
	if (result._kind == STR_PTR)
		str = result._data._str.acl_str->c_str();
	else if (result._kind == RESERVED)
		str = Reserved_Wds[result._data._reserved.keyword];
	else {
		if (result._kind == ATTR_PTR)
			copy_result(r1, *(ResultType *)result._data._attr.acl_attr->data);
		else
			copy_result(r1, result);
		if (convert_to(STR_PTR, r1))
			str = r1._data._str.acl_str->c_str();
		cleanup(r1);
	}

	return str;
}

void write_result(ResultType &result) {
	debugN("%s", get_result_string(result).c_str());
}

void display_result(ResultType &result) {
	String enclose;

	switch (result._kind) {
	case STR_PTR:
	case TEXT_LIT:
		enclose = "\"";
		break;
	case QUOTE_LIT:
		enclose = " ";
		debugN(">>");
		break;
	case MESSAGE:
		enclose = "\'";
		break;
	default:
		enclose = ' ';
		break;
	}

	if (enclose != " ")
		debugN("%s", enclose.c_str());
	write_result(result);
	if (enclose != " ")
		debugN("%s", enclose.c_str());
}

void display_expr(ExprTree the_tree) {
	if (the_tree->_kind != OPER) {
		display_result(*the_tree);
	} else {
		if (Binary[the_tree->_data._oper.op_name]) {
			debugN(" (");
			display_expr(the_tree->_data._oper.left);
			debugN(") ");
		}

		debugN("%s", Operators[the_tree->_data._oper.op_name]);
		debugN(" (");
		display_expr(the_tree->_data._oper.right);
		debugN(") ");
	}
}

bool load_game(Common::ReadStream *f_in) {
	int length;
	char ch = '\0';
	double fileVersion;

	// First, check the initial version string against that in the misc unit
	length = strlen(VERSION_STUB);
	for (int i = 0; i < length; ++i) {
		ch = (char)f_in->readByte();
		if (ch != VERSION_STUB[i]) {
			g_vm->writeln("This file is not an Archetype file.");
			return false;
		}
	}

	// Get the textual version of the game's version. This is easier than dealing with
	// the Pascal Real type the game files use for their version number
	String versionStr;
	while (!f_in->eos() && (ch = (char)f_in->readByte()) != '\n')
		versionStr += ch;
	fileVersion = atof(versionStr.c_str());

	// Bleed off string version information
	while (!f_in->eos() && ch != 26)
		ch = f_in->readByte();

	// Skip over 6 byte Real encoded version number
	(void)f_in->readUint32LE();
	(void)f_in->readUint16LE();

	if (fileVersion > VERSION_NUM) {
		g_vm->writeln("This version of PERFORM is %.1f; file version is %.1f",
			VERSION_NUM, fileVersion);
		g_vm->writeln("Cannot PERFORM this file.");
		return false;
	}

	// Get encryption information
	Encryption = (EncryptionType)f_in->readByte();

	// Read the timestamp.  It is used to verify saved game states, and also to prime the encryption
	GTimeStamp = f_in->readUint32LE();

	// Initialize the encrypter.  This is done by using the global time stamp as a starting point
	// and using the Encryption variable to decide the method.  Be careful here; the PURPLE
	// or Dynamic encryption works differently in that we have to set Encryption to UNPURPLE
	// (since we're decoding) and then back to PURPLE again in case they save any game states.
	// See load_game_state in the GAMESTAT unit for similar machinations
	if (Encryption == PURPLE)
		Encryption = UNPURPLE;
	cryptinit(Encryption, GTimeStamp);

	// Where's the main object?
	MainObject = f_in->readUint16LE();

	load_obj_list(f_in, g_vm->Object_List);
	load_obj_list(f_in, g_vm->Type_List);

	load_text_list(f_in, g_vm->Literals);
	load_text_list(f_in, g_vm->Vocabulary);

	if (Encryption == DEBUGGING_ON) {
		g_vm->writeln("Loading debugging information");
		load_id_info(f_in);
	}

	if (Encryption == UNPURPLE)
		Encryption = PURPLE;

	return true;
}

} // End of namespace Archetype
} // End of namespace Glk
