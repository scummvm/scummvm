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

#include "glk/archetype/semantic.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/error.h"
#include "glk/archetype/id_table.h"

namespace Glk {
namespace Archetype {

typedef int *IntegerPtr;

int classify_as(progfile &f, int id_number, ClassifyType interpretation, void *ptr_to_data) {
	IdRecPtr the_id_ptr;
    String error_string;
	int result = 0;

	if (!index_ident(id_number, the_id_ptr)) {
		error_message(f, "Attempt to classify unencountered identifier");
	} else {
		//with the_id_ptr^ do begin
		if (the_id_ptr->id_kind == interpretation)
			result = the_id_ptr->id_integer;

		// If the existing id_kind is the DefaultClassification, we're allowed to
		// change it; otherwise there's a conflict
		else if (the_id_ptr->id_kind == DefaultClassification) {
			the_id_ptr->id_kind    = interpretation;
			the_id_ptr->id_integer = the_id_ptr->id_index;

			switch (the_id_ptr->id_kind) {
			case TYPE_ID:
				append_to_xarray(g_vm->Type_List, ptr_to_data);
				append_to_xarray(g_vm->Type_ID_List, (void *)the_id_ptr->id_name);
				the_id_ptr->id_integer = g_vm->Type_List.size();
				break;

			case OBJECT_ID:
				if (ptr_to_data == nullptr) {
					the_id_ptr->id_integer = 0;
				} else {
					// Object_List may have grown by unnamed objects between calls to classify_as.
					// Fill in the intervening spaces with "null"
					while (g_vm->Object_ID_List.size() < g_vm->Object_List.size())
						append_to_xarray(g_vm->Object_ID_List, (void *)g_vm->NullStr);

					append_to_xarray(g_vm->Object_List, ptr_to_data);
					append_to_xarray(g_vm->Object_ID_List, (void *)the_id_ptr->id_name);
					the_id_ptr->id_integer = g_vm->Object_List.size();
				}
				break;

			case ATTRIBUTE_ID:
				append_to_xarray(g_vm->Attribute_ID_List, (void *)the_id_ptr->id_name);
				the_id_ptr->id_integer = g_vm->Attribute_ID_List.size();
				break;

			default:
				break;
			}
		} else {
			error_string = String::format("Identifier type conflict: \"%s\" already declared as ",
				the_id_ptr->id_name->c_str());

			switch (the_id_ptr->id_kind) {
			case TYPE_ID:
				error_string = error_string + "a type";
				break;
			case OBJECT_ID:
				error_string = error_string + "an object";
				break;
			case ATTRIBUTE_ID:
				error_string = error_string + "an attribute";
				break;
			case ENUMERATE_ID:
				error_string = error_string + "a keyword";
				break;
			default:
				break;
			}

			error_message(f, error_string);
			the_id_ptr->id_integer = 0;
		}

		result = the_id_ptr->id_integer;
	}

	return result;
}

void get_meaning(int id_number, ClassifyType &meaning, int &number) {
	IdRecPtr the_id_ptr;

	if (!index_ident(id_number, the_id_ptr)) {
		error("Internal error:  attempt to find meaning of unencountered identifier");
	} else {
		meaning = the_id_ptr->id_kind;
		number = the_id_ptr->id_integer;
	}
}

void add_undefined(int the_ID) {
	NodePtr np;
	IntegerPtr ip;

	np = find_item(g_vm->Overlooked, the_ID);
	if (np != nullptr) {
		++*((IntegerPtr)np->data);
	} else {
		np = new NodeType();
		np->key = the_ID;
		ip = new int();
		*ip = 1;			// TODO: Should this be 0-based?
		np->data = ip;
		insert_item(g_vm->Overlooked, np);
	}
}

bool display_undefined() {
	NodePtr np = nullptr;
	IntegerPtr ip;
	IdRecPtr id_rec;
	bool exists = false;

	while (iterate_list(g_vm->Overlooked, np)) {
		if (!exists) {
			debugN("The following identifiers were not explicitly defined.");
			exists = true;
		}

		ip = (IntegerPtr)np->data;
		debugN("Used %d", *ip);
		if (*ip == 1)
			debugN(" time:   ");
		else
			debugN(" times:  ");

		if (index_ident(np->key, id_rec))
			debug("%s", id_rec->id_name->c_str());
		else
			debug("<unknown identifier>");

		delete ip;
	}

	dispose_list(g_vm->Overlooked);

	return exists;
}

bool verify_expr(progfile &f, ExprTree the_expr) {
	bool success = true;

	switch (the_expr->_kind) {
	case OPER:
		switch (the_expr->_data._oper.op_name) {
		case OP_DOT:
			if (the_expr->_data._oper.right->_kind != IDENT) {
				error_message(f, "Right side of dot must be an identifier");
				success = false;
			}
			else if (the_expr->_data._oper.right->_data._ident.ident_kind != ATTRIBUTE_ID) {
				the_expr->_data._oper.right->_data._ident.ident_int = classify_as(f,
					the_expr->_data._oper.right->_data._ident.ident_int, ATTRIBUTE_ID, nullptr);
			}

			the_expr->_data._oper.right->_data._ident.ident_kind = ATTRIBUTE_ID;
			if (the_expr->_data._oper.right->_data._ident.ident_int == 0)
				success = false;
			// fallthrough
			// FIXME: is this fallthrough intentional?

		case OP_ASSIGN:
		case OP_C_CONCAT:
		case OP_C_MULTIPLY:
		case OP_C_DIVIDE:
		case OP_C_PLUS:
		case OP_C_MINUS:
			if (the_expr->_data._oper.left->_kind == IDENT) {
				get_meaning(the_expr->_data._oper.left->_data._ident.ident_int,
					the_expr->_data._oper.left->_data._ident.ident_kind, the_expr->_data._oper.left->_data._ident.ident_int);

				if (the_expr->_data._oper.left->_data._ident.ident_kind != ATTRIBUTE_ID) {
					error_message(f, "Left side of assignment is not an attribute");
					success = false;
				}
			}
			else if (!(the_expr->_data._oper.left->_kind == OPER &&
				the_expr->_data._oper.left->_data._oper.op_name == OP_DOT)) {
				error_message(f, "Left side of assignment must reference an attribute");
				success = false;
			}
			break;

		default:
			break;
		}

		if (success) {
			if (Binary[the_expr->_data._oper.op_name])
				success = verify_expr(f, the_expr->_data._oper.left);
		}
		if (success)
			success = verify_expr(f, the_expr->_data._oper.right);
		break;

	default:
		break;
	}

	return success;
}

} // End of namespace Archetype
} // End of namespace Glk
