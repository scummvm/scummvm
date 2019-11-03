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

#ifndef ARCHETYPE_STATEMENT
#define ARCHETYPE_STATEMENT

#include "glk/archetype/expression.h"
#include "glk/archetype/linked_list.h"

namespace Glk {
namespace Archetype {

enum StatementKind {
	COMPOUND, ST_EXPR, ST_IF, ST_CASE, ST_FOR, ST_WHILE, ST_BREAK, ST_CREATE,
	ST_DESTROY, ST_WRITE, ST_WRITES, ST_STOP, CONT_SEQ, END_SEQ
};

struct StatementType;
typedef StatementType *StatementPtr;

struct StmtCompound {
	ListType statements;
};

struct StmtExpr {
	ExprTree expression;
};

struct StmtIf {
	ExprTree condition;
	StatementPtr then_branch;
	StatementPtr else_branch;
};

struct StmtCase {
	ExprTree test_expr;
	ListType cases;
};

struct StmtCreate {
	int archetype;
	ExprTree new_name;
};

struct StmtDestroy {
	ExprTree victim;
};

struct StmtLoop {
	ExprTree selection;
	StatementPtr action;
};

struct StmtWrite {
	ListType print_list;
};

union StatementTypeData {
	StmtCompound _compound;
	StmtExpr _expr;
	StmtIf _if;
	StmtCase _case;
	StmtCreate _create;
	StmtDestroy _destroy;
	StmtLoop _loop;
	StmtWrite _write;
};

struct StatementType {
	StatementKind _kind;
	StatementTypeData _data;

	StatementType() : _kind(COMPOUND) {
		_data._compound.statements = nullptr;
	}
};

struct CasePairType {
	ExprTree value;
	StatementPtr action;
};
typedef CasePairType *CasePairPtr;

} // End of namespace Archetype
} // End of namespace Glk

#endif
