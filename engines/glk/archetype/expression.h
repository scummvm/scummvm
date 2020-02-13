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

#ifndef ARCHETYPE_EXPRESSION
#define ARCHETYPE_EXPRESSION

#include "glk/archetype/keywords.h"
#include "glk/archetype/linked_list.h"
#include "glk/archetype/misc.h"

namespace Glk {
namespace Archetype {

const int OP_LPAREN = NUM_OPERS + 1;		// book-keeping operator
const int OP_SEND_TO_TYPE = NUM_OPERS + 2;	// for use with interpreter

struct ExprNode;

struct OperNode {
	int8 op_name;
	ExprNode *left, *right;
};

struct MessageTextQuoteNode {
	int index;
};

struct NumericNode {
	int acl_int;
};

struct StrNode {
	StringPtr acl_str;
};

struct AttrNode {
	NodePtr acl_attr;
};

struct ReservedNode {
	int8 keyword;
};

struct IdentNode {
	ClassifyType ident_kind;
	int ident_int;
};

union ExprNodeData {
	OperNode _oper;
	NumericNode _numeric;
	MessageTextQuoteNode _msgTextQuote;
	StrNode _str;
	AttrNode _attr;
	ReservedNode _reserved;
	IdentNode _ident;
};

struct ExprNode {
	AclType _kind;
	ExprNodeData _data;

	ExprNode() : _kind(RESERVED) {
		_data._reserved.keyword = RW_UNDEFINED;
	}
};

typedef ExprNode *ExprPtr;
typedef ExprPtr ExprTree;

// Global variables
extern bool Right_Assoc[NUM_OPERS + 2];
extern bool Binary[NUM_OPERS + 2];
extern int8 Precedence[NUM_OPERS + 2];

extern void expression_init();

} // End of namespace Archetype
} // End of namespace Glk

#endif
