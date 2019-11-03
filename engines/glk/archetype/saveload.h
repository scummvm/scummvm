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

#ifndef ARCHETYPE_ERROR
#define ARCHETYPE_ERROR

/* Contains routines for both saving and loading binary ACX files.  Also
 * contains routines for disposing of the major ACL structures, in order to
 * be able to throw away the old before loading in the new
 */

#include "glk/archetype/linked_list.h"
#include "glk/archetype/statement.h"
#include "common/stream.h"

namespace Glk {
namespace Archetype {

enum ContentType { STMT_LIST, EXPR_LIST, CASE_LIST };
enum MissionType { LOAD, DUMP, FREE, DISPLAY };

struct ObjectType {
	int inherited_from;		// index to Type_List
	ListType attributes;
	ListType methods;
	StatementPtr other;

	ObjectType() : inherited_from(0), attributes(nullptr), methods(nullptr),
		other(nullptr) {}
};
typedef ObjectType *ObjectPtr;

// Global variables
extern StatementKind vEndSeq, vContSeq;		// to make BlockWrite happy
extern int Dynamic;
extern bool Translating;

extern void saveload_init();

extern void load_item_list(Common::ReadStream *f_in, ListType &elements, ContentType content);
extern void dump_item_list(Common::WriteStream *f_out, ListType &elements, ContentType content);
extern void dispose_item_list(ListType &elements, ContentType content);

extern void load_expr(Common::ReadStream *f_in, ExprTree &the_expr);
extern void dump_expr(Common::WriteStream *f_out, ExprTree &the_expr);
extern void dispose_expr(ExprTree &the_expr);

extern void load_stmt(Common::ReadStream *f_in, StatementPtr &the_stmt);
extern void dump_stmt(Common::WriteStream *f_out, StatementPtr &the_stmt);
extern void dispose_stmt(StatementPtr &the_stmt);

extern void load_object(Common::ReadStream *f_in, ObjectPtr &the_object);
extern void dump_object(Common::WriteStream *f_out, const ObjectPtr the_object);
extern void dispose_object(ObjectPtr &the_object);

extern void load_obj_list(Common::ReadStream *f_in, XArrayType &obj_list);
extern void dump_obj_list(Common::WriteStream *f_out, XArrayType &obj_list);
extern void dispose_obj_list(XArrayType &obj_list);

} // End of namespace Archetype
} // End of namespace Glk

#endif
