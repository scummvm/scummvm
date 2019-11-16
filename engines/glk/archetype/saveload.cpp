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

#include "glk/archetype/saveload.h"
#include "glk/archetype/id_table.h"
#include "glk/archetype/semantic.h"

namespace Glk {
namespace Archetype {

StatementKind vEndSeq, vContSeq;		// to make BlockWrite happy
int Dynamic;
bool Translating;

void saveload_init() {
	vEndSeq = END_SEQ;
	vContSeq = CONT_SEQ;

	Dynamic = 0;
	Translating = false;
}

// ===================== Forward Declarations =======================

static void walk_item_list(MissionType mission, Common::Stream *bfile,
	ListType &elements, ContentType content);
static void walk_expr(MissionType mission, Common::Stream *bfile, ExprTree &the_expr);
static void walk_stmt(MissionType mission, Common::Stream *bfile, StatementPtr &the_stmt);

// ========================== Item Lists ============================

// Wrappers

void load_item_list(Common::ReadStream *f_in, ListType &elements, ContentType content) {
	walk_item_list(LOAD, f_in, elements, content);
}

void dump_item_list(Common::WriteStream *f_out, ListType &elements, ContentType content) {
	walk_item_list(DUMP, f_out, elements, content);
}

void dispose_item_list(ListType &elements, ContentType content) {
	walk_item_list(FREE, nullptr, elements, content);
}

/**
 * Used for operating on general linked lists which are homogenous, all containing the same type
 * of data, signified by the "content" variable.
 * @param mission			action to perform while walking through
 * @param bfile				binary file to read or write from
 * @param elements			list of items
 * @param content			contents of each of the items
 */
void walk_item_list(MissionType mission, Common::Stream *bfile, ListType &elements, ContentType content) {
	StatementKind sentinel;
	StatementPtr this_stmt;
	ExprTree this_expr;
	CasePairPtr this_case;
	NodePtr np = nullptr;
	bool yet_more = false;

	Common::ReadStream *readStream = dynamic_cast<Common::ReadStream *>(bfile);
	Common::WriteStream *writeStream = dynamic_cast<Common::WriteStream *>(bfile);

	// Prelude
	switch (mission) {
	case LOAD:
		assert(readStream);
		sentinel = (StatementKind)readStream->readByte();
		new_list(elements);
		yet_more = sentinel == CONT_SEQ;
		break;

	case DUMP:
	case FREE:
		np = nullptr;
		yet_more = iterate_list(elements, np);
		break;

	default:
		break;
	}

	while (yet_more) {
		// Main walk
		switch (mission) {
		case LOAD:
			assert(readStream);
			np = new NodeType();
			add_bytes(sizeof(NodeType));
			np->key = readStream->readSint16LE();
			break;

		case DUMP:
			assert(writeStream);
			writeStream->writeByte(vContSeq);
			writeStream->writeSint16LE(np->key);
			break;

		default:
			break;
		}

		switch (content) {
		case EXPR_LIST:
			switch (mission) {
			case LOAD:
				walk_expr(mission, bfile, this_expr);
				np->data = this_expr;
				break;
			case DUMP:
			case FREE:
				this_expr = (ExprTree)np->data;
				walk_expr(mission, bfile, this_expr);
				break;
			default:
				break;
			}
			break;

		case STMT_LIST:
			switch (mission) {
			case LOAD:
				walk_stmt(mission, bfile, this_stmt);
				np->data = this_stmt;
				break;
			case DUMP:
			case FREE:
				this_stmt = (StatementPtr)np->data;
				walk_stmt(mission, bfile, this_stmt);
				break;
			default:
				break;
			}
			break;

		case CASE_LIST:
			switch (mission) {
			case LOAD:
				this_case = new CasePairType();
				add_bytes(sizeof(CasePairType));
				walk_expr(mission, bfile, this_case->value);
				walk_stmt(mission, bfile, this_case->action);

				np->data = this_case;
				break;

			case DUMP:
			case FREE:
				this_case = (CasePairPtr)np->data;
				walk_expr(mission, bfile, this_case->value);
				walk_stmt(mission, bfile, this_case->action);

				if (mission == FREE) {
					add_bytes(sizeof(CasePairType));
					delete this_case;
				}
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}

		switch (mission) {
		case LOAD:
			assert(readStream);
			append_to_list(elements, np);
			sentinel = (StatementKind)readStream->readByte();
			yet_more = sentinel == CONT_SEQ;
			break;

		case DUMP:
		case FREE:
			yet_more = iterate_list(elements, np);
			break;

		default:
			break;
		}
	}

	// Postlude
	switch (mission) {
	case DUMP:
		writeStream->writeByte(vEndSeq);
		break;
	case FREE:
		dispose_list(elements);
		elements = nullptr;
		break;
	default:
		break;
	}
}

// ============================ Expressions ===========================

// Wrappers
void load_expr(Common::ReadStream *f_in, ExprTree &the_expr) {
	walk_expr(LOAD, f_in, the_expr);
}

void dump_expr(Common::WriteStream *f_out, ExprTree &the_expr) {
	walk_expr(DUMP, f_out, the_expr);
}

void dispose_expr(ExprTree &the_expr) {
	walk_expr(FREE, nullptr, the_expr);
}

/**
 * Separated from walk_expr so as not to consume too much stack space with its large temporary string..
 */
static StringPtr LoadDynStr(Common::ReadStream *f_in) {
	String s;
	load_string(f_in, s);
	return NewDynStr(s);
}

/**
 * Walks through an expression tree.
 * @param mission			action to take on each visited element
 * @param bfile				binary file to read or write from as necessary
 * @param the_expr			expression tree to walk
 */
static void walk_expr(MissionType mission, Common::Stream *bfile, ExprTree &the_expr) {
	int temp = 0;
	ClassifyType ID_kind = UNDEFINED_ID;

	Common::ReadStream *readStream = dynamic_cast<Common::ReadStream *>(bfile);
	Common::WriteStream *writeStream = dynamic_cast<Common::WriteStream *>(bfile);

	// Prelude
	switch (mission) {
	case LOAD:
		assert(readStream);
		the_expr = new ExprNode();
		add_bytes(sizeof(ExprNode));
		the_expr->_kind = (AclType)readStream->readByte();
		break;

	case DUMP:
		if (the_expr == nullptr)
			return;

		assert(writeStream);
		while (the_expr->_kind == OPER && the_expr->_data._oper.op_name == OP_LPAREN)
			the_expr = the_expr->_data._oper.right;

		writeStream->writeByte(the_expr->_kind);
		break;

	case FREE:
		if (the_expr == nullptr)
			return;
		break;

	default:
		break;
	}

	// Main walk
	switch (the_expr->_kind) {
	case OPER:
		switch (mission) {
		case LOAD:
			the_expr->_data._oper.op_name = readStream->readSByte();
			the_expr->_data._oper.left = nullptr;
			break;
		case DUMP:
			writeStream->writeSByte(the_expr->_data._oper.op_name);
			break;
		default:
			break;
		}

		if (Binary[the_expr->_data._oper.op_name])
			walk_expr(mission, bfile, the_expr->_data._oper.left);
		walk_expr(mission, bfile, the_expr->_data._oper.right);
		break;

	case NUMERIC:
		switch (mission) {
		case LOAD:
			the_expr->_data._numeric.acl_int = readStream->readSint32LE();
			break;
		case DUMP:
			writeStream->writeSint32LE(the_expr->_data._numeric.acl_int);
			break;
		default:
			break;
		}
		break;

	case MESSAGE:
	case TEXT_LIT:
	case QUOTE_LIT:
		switch (mission) {
		case LOAD:
			the_expr->_data._msgTextQuote.index = readStream->readSint16LE();
			break;
		case DUMP:
			writeStream->writeSint16LE(the_expr->_data._msgTextQuote.index);
			break;
		default:
			break;
		}
		break;

	case IDENT:
		switch (mission) {
		case LOAD:
			the_expr->_data._ident.ident_kind = (ClassifyType)readStream->readByte();
			the_expr->_data._ident.ident_int = readStream->readSint16LE();
			break;
		case DUMP:
			if (Translating && the_expr->_data._ident.ident_kind == DefaultClassification) {
				// may have changed meaning
				get_meaning(the_expr->_data._ident.ident_int, ID_kind, temp);
				if (ID_kind == UNDEFINED_ID) {
					add_undefined(the_expr->_data._ident.ident_int);
				} else {
					the_expr->_data._ident.ident_kind = ID_kind;
					the_expr->_data._ident.ident_int = temp;
				}
			}

			writeStream->writeByte(the_expr->_data._ident.ident_kind);
			writeStream->writeSint16LE(the_expr->_data._ident.ident_int);
			break;
		default:
			break;
		}
		break;

	case RESERVED:
		switch (mission) {
		case LOAD:
			the_expr->_data._reserved.keyword = readStream->readSByte();
			break;
		case DUMP:
			writeStream->writeSByte(the_expr->_data._reserved.keyword);
			break;
		default:
			break;
		}
		break;

	case STR_PTR:
		switch (mission) {
		case LOAD:
			the_expr->_data._str.acl_str = LoadDynStr(readStream);
			break;
		case DUMP:
			dump_string(writeStream, *the_expr->_data._str.acl_str);
			break;
		case FREE:
			FreeDynStr(the_expr->_data._str.acl_str);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	// Postlude
	switch (mission) {
	case FREE:
		delete the_expr;
		the_expr = nullptr;
		break;
	default:
		break;
	}
}


// =========================== Statements =========================

// Wrappers

void load_stmt(Common::ReadStream *f_in, StatementPtr &the_stmt) {
	walk_stmt(LOAD, f_in, the_stmt);
}

void dump_stmt(Common::WriteStream *f_out, StatementPtr &the_stmt) {
	walk_stmt(DUMP, f_out, the_stmt);
}

void dispose_stmt(StatementPtr &the_stmt) {
	walk_stmt(FREE, nullptr, the_stmt);
}

/**
 * Handles the control involved in walking through a statement.
 * @param mission		action to take for each statement
 * @param bfile			binary file to read or write as necessary
 * @param the_stmt		pointer to a statement record
 */
static void walk_stmt(MissionType mission, Common::Stream *bfile, StatementPtr &the_stmt) {
	NodePtr np;				// for appending to lists
	StatementKind sentinel;
	ExprTree this_expr;

	Common::ReadStream *readStream = dynamic_cast<Common::ReadStream *>(bfile);
	Common::WriteStream *writeStream = dynamic_cast<Common::WriteStream *>(bfile);

	// Prelude
	switch (mission) {
	case LOAD:
		the_stmt = nullptr;
		if (readStream->eos())
			return;

		sentinel = (StatementKind)readStream->readByte();
		if (sentinel == END_SEQ)
			return;

		the_stmt = new StatementType();
		add_bytes(sizeof(StatementType));
		the_stmt->_kind = sentinel;
		break;

	case DUMP:
		if (the_stmt == nullptr) {
			writeStream->writeByte(vEndSeq);
			return;
		} else {
			writeStream->writeByte(the_stmt->_kind);
		}
		break;

	case FREE:
		if (the_stmt == nullptr)
			return;
		break;

	default:
		break;
	}

	// Main walk
	switch (the_stmt->_kind) {
	case COMPOUND:
		walk_item_list(mission, bfile, the_stmt->_data._compound.statements, STMT_LIST);
		break;

	case ST_EXPR:
		walk_expr(mission, bfile, the_stmt->_data._expr.expression);
		break;

	case ST_IF:
		walk_expr(mission, bfile, the_stmt->_data._if.condition);
		walk_stmt(mission, bfile, the_stmt->_data._if.then_branch);
		walk_stmt(mission, bfile, the_stmt->_data._if.else_branch);
		break;

	case ST_CASE:
		walk_expr(mission, bfile, the_stmt->_data._case.test_expr);
		walk_item_list(mission, bfile, the_stmt->_data._case.cases, CASE_LIST);
		break;

	case ST_CREATE:
		switch (mission) {
		case LOAD:
			the_stmt->_data._create.archetype = readStream->readSint16LE();
			break;
		case DUMP:
			writeStream->writeSint16LE(the_stmt->_data._create.archetype);
			break;
		default:
			break;
		}

		walk_expr(mission, bfile, the_stmt->_data._create.new_name);
		break;

	case ST_DESTROY:
		walk_expr(mission, bfile, the_stmt->_data._destroy.victim);
		break;

	case ST_FOR:
	case ST_WHILE:
		walk_expr(mission, bfile, the_stmt->_data._loop.selection);
		walk_stmt(mission, bfile, the_stmt->_data._loop.action);
		break;

	case ST_WRITE:
	case ST_WRITES:
	case ST_STOP:
		switch (mission) {
		case LOAD:
			new_list(the_stmt->_data._write.print_list);
			sentinel = (StatementKind)readStream->readByte();

			while (sentinel != END_SEQ) {
				walk_expr(mission, bfile, this_expr);
				np = new NodeType();
				add_bytes(sizeof(NodeType));

				np->data = this_expr;
				append_to_list(the_stmt->_data._write.print_list, np);

				sentinel = (StatementKind)readStream->readByte();
			}
			break;

		case DUMP:
		case FREE:
			np = nullptr;
			while (iterate_list(the_stmt->_data._write.print_list, np)) {
				if (mission == DUMP)
					writeStream->writeByte(vContSeq);
				this_expr = (ExprTree)np->data;
				walk_expr(mission, bfile, this_expr);

				if (mission == FREE)
					np->data = nullptr;
			}

			if (mission == DUMP)
				writeStream->writeByte(vEndSeq);
			else
				dispose_list(the_stmt->_data._write.print_list);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	// Postlude
	switch (mission) {
	case FREE:
		add_bytes(sizeof(StatementType));
		delete the_stmt;
		break;
	default:
		break;
	}
}


// ============================ Objects ===========================

void load_object(Common::ReadStream *f_in, ObjectPtr &the_object) {
	StatementKind sentinel;

	the_object = new ObjectType();
	add_bytes(sizeof(ObjectType));

	the_object->inherited_from = f_in->readSint16LE();
	load_item_list(f_in, the_object->attributes, EXPR_LIST);
	load_item_list(f_in, the_object->methods, STMT_LIST);

	sentinel = (StatementKind)f_in->readByte();
	if (sentinel == CONT_SEQ)
		load_stmt(f_in, the_object->other);
	else
		the_object->other = nullptr;
}

void dump_object(Common::WriteStream *f_out, const ObjectPtr the_object) {
	f_out->writeSint16LE(the_object->inherited_from);
	dump_item_list(f_out, the_object->attributes, EXPR_LIST);
	dump_item_list(f_out, the_object->methods, STMT_LIST);

	if (the_object->other == nullptr) {
		f_out->writeByte(vEndSeq);
	} else {
		f_out->writeByte(vContSeq);
		dump_stmt(f_out, the_object->other);
	}
}

void dispose_object(ObjectPtr &the_object) {
	dispose_item_list(the_object->attributes, EXPR_LIST);
	dispose_item_list(the_object->methods, STMT_LIST);
	if (the_object->other != nullptr)
		dispose_stmt(the_object->other);

	add_bytes(sizeof(ObjectType));
	delete the_object;
	the_object = nullptr;
}


// ============================= Object Lists ========================

void load_obj_list(Common::ReadStream *f_in, XArrayType &obj_list) {
	ObjectPtr new_object;
	void *p;
	int i, list_size;

	new_xarray(obj_list);
	list_size = f_in->readUint16LE();

	for (i = 0; i < list_size; ++i) {
		load_object(f_in, new_object);
		p = new_object;
		append_to_xarray(obj_list, p);
	}

	// Objects may be dynamically allocated beneath this limit.  It is okay to set that limit
	// at this time since this routine is only invoked when initially loading a game
	Dynamic = obj_list.size() + 1;
}

void dump_obj_list(Common::WriteStream *f_out, XArrayType &obj_list) {
	uint i;
	void *p;
	ObjectPtr this_obj;

	f_out->writeUint16LE(obj_list.size());

	for (i = 1; i <= obj_list.size(); ++i) {
		if (index_xarray(obj_list, i, p)) {
			this_obj = (ObjectPtr)p;
			dump_object(f_out, this_obj);
		}
	}
}

void dispose_obj_list(XArrayType &obj_list) {
	uint i;
	void *p;
	ObjectPtr axe_obj;

	for (i = 1; i <= obj_list.size(); ++i) {
		if (index_xarray(obj_list, i, p)) {
			axe_obj = (ObjectPtr)p;
			dispose_object(axe_obj);
		}
	}

	dispose_xarray(obj_list);
}

} // End of namespace Archetype
} // End of namespace Glk
