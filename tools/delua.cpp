/* Residual - A 3D game interpreter
*
* Residual is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the AUTHORS
* file distributed with this source distribution.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.

* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*
* $URL$
* $Id$
*
*/

#include <common/sys.h>
#include <common/file.h>
#include <common/str.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <map>
#include <queue>
#include <stack>
#include <list>
#include <set>

#include <engines/grim/lua/lua.h>
#include <engines/grim/lua/lundump.h>
#include <engines/grim/lua/lopcodes.h>
#include <engines/grim/lua/lzio.h>
#include <engines/grim/lua/lobject.h>
#include <engines/grim/localize.h>
#include <engines/grim/resource.h>


namespace Grim {

class Actor;
class Color;
class ResourceLoader;
class GrimEngine;

// hacks below for shutup linker
int g_flags = 0;
GrimEngine *g_grim = NULL;
ResourceLoader *g_resourceloader = NULL;
LuaFile *ResourceLoader::openNewStreamLuaFile(const char *filename) const { return NULL; }
Actor *check_actor(int num) { return NULL; }
Color *check_color(int num) { return NULL; }

static bool translateStrings = false;

class Expression;

void decompile(std::ostream &os, TProtoFunc *tf, std::string indent_str, Expression **upvals, int num_upvals);

std::string localname(TProtoFunc *tf, int n) {
	LocVar *l = tf->locvars;
	if (l != NULL) {
		for (int i = 0; i < n; i++, l++) {
			if (l->varname == NULL) {
				l = NULL;
				break;
			}
			if (l != NULL)
				return l->varname->str;
			else {
				std::ostringstream s;
				if (n < tf->code[1])
					s << "arg" << n + 1;
				else
					s << "local" << n - tf->code[1] + 1;
				return s.str();
			}
		}
	}
	return "";
}

class Expression {
public:
	Expression(byte *p) : pos(p) { }
	byte *pos;			// Position just after the expression
	// is pushed onto the stack
	virtual void print(std::ostream &os) const = 0;
	virtual int precedence() const { return 100; }
	virtual ~Expression() { }
};

inline std::ostream& operator <<(std::ostream &os, const Expression &e) {
	e.print(os);
	return os;
}

class NumberExpr : public Expression {
public:
	NumberExpr(byte *p, float val) : Expression(p), value(val) { }
	float value;
	void print(std::ostream &os) const { os << value; }
};

class VarExpr : public Expression {
public:
	VarExpr(byte *p, std::string varname) : Expression(p), name(varname) { }
	std::string name;
	void print(std::ostream &os) const { os << name; }
};

class StringExpr : public Expression {
public:
	StringExpr(byte *p, TaggedString *txt) : Expression(p), text(txt) { }
	TaggedString *text;
	bool validIdentifier() const {
		if (strlen(text->str) == 0)
			return false;
		if (isdigit(text->str[0]))
			return false;
		if (text->str[0] >= '0' && text->str[0] <= '9')
			return false;
		for (unsigned int i = 0; i < strlen(text->str); i++) {
			char c = text->str[i];
			if ((! isalnum(text->str[0])) && c != '_')
				return false;
		}
		return true;
	}
	void print(std::ostream &os) const {
		static const char *specials = "\a\b\f\n\r\t\v\\\"";
		static const char *special_text[] = {
			"\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v", "\\\\", "\\\""
		};

		os << "\"";
		std::string str(text->str);
		if (translateStrings)
			str = g_localizer->localize(str.c_str()).c_str();
		for (std::string::iterator i = str.begin(); i != str.end(); i++) {
			unsigned char c = *i;
			if (strchr(specials, c)) {
				int i = strchr(specials, c) - specials;
				os << special_text[i];
			} else if (! isprint(c))
				os << "\\" << int(c >> 6) << int((c >> 3) & 7) << int(c & 7);
			else
				os << c;
		}
		os << "\"";
	}
};

class FuncExpr : public Expression {
public:
	FuncExpr(byte *p, TProtoFunc *tf0, std::string is) :
	Expression(p), indent_str(is), tf(tf0), upvals(NULL), num_upvals(0) { }
	std::string indent_str;
	TProtoFunc *tf;
	Expression **upvals;
	int num_upvals;

	void print(std::ostream &os) const {
		os << "function(";
		for (int i = 0; i < tf->code[1]; i++) {
			os << localname(tf, i);
			if (i + 1 < tf->code[1])
				os << ", ";
		}
		os << ") -- line " << tf->lineDefined << std::endl;
		decompile(os, tf, indent_str + std::string(4, ' '), upvals, num_upvals);
		os << indent_str << "end";
	}
	~FuncExpr() {
		for (int i = 0; i < num_upvals; i++)
			delete upvals[i];
		delete[] upvals;
	}
};

class IndexExpr : public Expression {
public:
	IndexExpr(byte *p, Expression *tbl, Expression *i)
		: Expression(p), table(tbl), index(i) { }
	Expression *table, *index;
	void print(std::ostream &os) const {
		table->print(os);
		StringExpr *field = dynamic_cast<StringExpr *>(index);
		if (field != NULL && field->validIdentifier())
			os << "." << field->text->str;
		else
			os << "[" << *index << "]";
	}
	~IndexExpr() {
		delete table;
		delete index;
	}
};

class SelfExpr : public IndexExpr {
public:
	SelfExpr(byte *p, Expression *tbl, StringExpr *i) : IndexExpr(p, tbl, i) { }
	void print(std::ostream &os) const {
		StringExpr *field = static_cast<StringExpr *>(index);
		os << *table << ":" << field->text->str;
	}
};

class FuncCallExpr : public Expression {
public:
	FuncCallExpr(byte *p) : Expression(p) { }
	int num_args;
	Expression **args;
	Expression *func;
	void print(std::ostream &os) const {
		os << *func << "(";
		int i = 0;

		// Skip implicit self argument in a:b(c)
		if (dynamic_cast<SelfExpr *>(func) != NULL)
			i = 1;
		for (; i < num_args; i++) {
			args[i]->print(os);
			if (i + 1 < num_args)
				os << ", ";
		}
		os << ")";
	}
	~FuncCallExpr() {
		for (int i = 0; i < num_args; i++)
			delete args[i];
		delete args;
		delete func;
	}
};

class ArrayExpr : public Expression {
public:
	ArrayExpr(byte *p) : Expression(p) { }
	typedef std::pair<Expression *, Expression *> mapping;
	typedef std::list<mapping> mapping_list;
	mapping_list mappings;
	void print(std::ostream &os) const {
		os << "{";
		mapping_list::const_iterator i = mappings.begin();
		while (i != mappings.end()) {
			if (i->first != NULL) {
				StringExpr *field = dynamic_cast<StringExpr *>(i->first);
				if (field != NULL && field->validIdentifier())
					os << " " << field->text->str;
				else
					os << " [" << *i->first << "]";
				os << " =";
			}
			os << " " << *i->second;
			i++;
			if (i != mappings.end())
				os << ",";
		}
		os << " }";
	}
	~ArrayExpr() {
		for (mapping_list::iterator i = mappings.begin(); i != mappings.end(); i++) {
			delete i->first;
			delete i->second;
		}
	}
};

class BinaryExpr : public Expression {
public:
	BinaryExpr(byte *ps, Expression *l, Expression *r, int p, bool ra,
		std::string o) :
	Expression(ps), left(l), right(r), prec(p), right_assoc(ra), op(o) { }
	Expression *left, *right;
	int prec;
	bool right_assoc;
	std::string op;
	int precedence() const { return prec; }
	void print(std::ostream &os) const {
		if (left->precedence() < prec ||
			(left->precedence() == prec && right_assoc))
			os << "(" << *left << ")";
		else
			os << *left;
		os << op;
		if (right->precedence() < prec ||
			(right->precedence() == prec && ! right_assoc))
			os << "(" << *right << ")";
		else
			os << *right;
	}
	~BinaryExpr() { delete left; delete right; }
};

class UnaryExpr : public Expression {
public:
	UnaryExpr(byte *ps, Expression *a, int p, std::string o) :
	  Expression(ps), arg(a), prec(p), op(o) { }
	  Expression *arg;
	  int prec;
	  std::string op;
	  int precedence() const { return prec; }
	  void print(std::ostream &os) const {
		  os << op;
		  if (arg->precedence() < prec)
			  os << "(" << *arg << ")";
		  else
			  os << *arg;
	  }
	  ~UnaryExpr() { delete arg; }
};

typedef std::stack<Expression *> ExprStack;

class Decompiler {
public:
	void decompileRange(byte *start, byte *end);

	std::ostream *os;
	ExprStack *stk;
	TProtoFunc *tf;
	std::string indent_str;
	byte *break_pos;
	Expression **upvals; int num_upvals;
	std::multiset<byte *> *local_var_defs;

private:
	void do_multi_assign(byte *&start);
	void do_binary_op(byte *pos, int prec, bool right_assoc, std::string op);
	void do_unary_op(byte *pos, int prec, std::string op);
	static bool is_expr_opc(byte opc);
	void get_else_part(byte *start, byte *&if_part_end,
		bool &has_else, byte *&else_part_end);
};

// Scan for a series of assignments
void Decompiler::do_multi_assign(byte *&start) {
	std::queue<Expression *> results;
	ExprStack values;

	bool done;
	int num_tables = 0;
	do {
		int aux, opc;
		done = false;

		opc = *start++;
		switch (opc) {
	case SETLOCAL:
		aux = *start++;
		goto setlocal;

	case SETLOCAL0:
	case SETLOCAL1:
	case SETLOCAL2:
	case SETLOCAL3:
	case SETLOCAL4:
	case SETLOCAL5:
	case SETLOCAL6:
	case SETLOCAL7:
		aux = opc - SETLOCAL0;
setlocal:
		results.push(new VarExpr(start, localname(tf, aux)));
		break;

	case SETGLOBAL:
		aux = *start++;
		goto setglobal;

	case SETGLOBAL0:
	case SETGLOBAL1:
	case SETGLOBAL2:
	case SETGLOBAL3:
	case SETGLOBAL4:
	case SETGLOBAL5:
	case SETGLOBAL6:
	case SETGLOBAL7:
		aux = opc - SETGLOBAL0;
		goto setglobal;

	case SETGLOBALW:
		aux = start[0] | (start[1] << 8);
		start += 2;
setglobal:
		results.push(new VarExpr(start, svalue(tf->consts + aux)));
		break;

	case SETTABLE:
		start++;			// assume offset is correct
	case SETTABLE0:
		results.push(new IndexExpr(start, NULL, NULL));
		num_tables++;
		// this needs stuff from farther up the stack, wait until
		// it's available
		break;

	default:
		start--;
		done = true;
		}

		if (! done) {
			Expression *e = stk->top();
			// Check for fake result from function calls with multiple return values
			VarExpr *v = dynamic_cast<VarExpr *>(e);
			if (v != NULL && v->name == "<extra result>")
				delete e;
			else
				values.push(e);
			stk->pop();
		}
	} while (! done);

	// Check for popping tables and indices
	if (num_tables > 0 && (*start == POP || *start == POP0 || *start == POP1)) {
		start++;
		if (start[-1] == POP)
			start++;
	}

	// Now get actual tables and indices from the stack, reversing
	// the list to the right order at the same time

	ExprStack results2;
	while (! results.empty()) {
		Expression *var = results.front(); results.pop();
		IndexExpr *tbl = dynamic_cast<IndexExpr *>(var);
		if (tbl != NULL) {
			tbl->index = stk->top(); stk->pop();
			tbl->table = stk->top(); stk->pop();
		}
		results2.push(var);
	}

	*os << indent_str;
	while (! results2.empty()) {
		Expression *var = results2.top(); results2.pop();
		*os << *var;
		delete var;
		if (! results2.empty())
			*os << ", ";
	}
	*os << " = ";
	while (! values.empty()) {
		Expression *val = values.top(); values.pop();
		*os << *val;
		delete val;
		if (! values.empty())
			*os << ", ";
	}
	*os << std::endl;
}

void Decompiler::do_binary_op(byte *pos, int prec, bool right_assoc, std::string op) {
	Expression *right = stk->top(); stk->pop();
	Expression *left = stk->top(); stk->pop();
	stk->push(new BinaryExpr(pos, left, right, prec, right_assoc, op));
}

void Decompiler::do_unary_op(byte *pos, int prec, std::string op) {
	Expression *arg = stk->top(); stk->pop();
	stk->push(new UnaryExpr(pos, arg, prec, op));
}

// Provide instruction lengths to make it easy to scan through instructions
int instr_lens[] = {
	1,				// ENDCODE
	2,				// PUSHNIL
	1,				// PUSHNIL0
	2,				// PUSHNUMBER
	1, 1, 1,			// PUSHNUMBER0..2
	3,				// PUSHNUMBERW
	2,				// PUSHCONSTANT
	1, 1, 1, 1, 1, 1, 1, 1,	// PUSHCONSTANT0..7
	3,				// PUSHCONSTANTW
	2,				// PUSHUPVALUE
	1, 1,				// PUSHUPVALUE0,1
	2,				// PUSHLOCAL
	1, 1, 1, 1, 1, 1, 1, 1,	// PUSHLOCAL0..7
	2,				// GETGLOBAL
	1, 1, 1, 1, 1, 1, 1, 1,	// GETGLOBAL0..7
	3,				// GETGLOBALW
	1,				// GETTABLE
	2,				// GETDOTTED
	1, 1, 1, 1, 1, 1, 1, 1,	// GETDOTTED0..7
	3,				// GETDOTTEDW
	2,				// PUSHSELF
	1, 1, 1, 1, 1, 1, 1, 1,	// PUSHSELF0..7
	3,				// PUSHSELFW
	2,				// CREATEARRAY
	1, 1,				// CREATEARRAY0,1
	3,				// CREATEARRAYW
	2,				// SETLOCAL
	1, 1, 1, 1, 1, 1, 1, 1,	// SETLOCAL0..7
	2,				// SETGLOBAL
	1, 1, 1, 1, 1, 1, 1, 1,	// SETGLOBAL0..7
	3,				// SETGLOBALW
	1,				// SETTABLE0
	2,				// SETTABLE
	3,				// SETLIST
	2,				// SETLIST0
	4,				// SETLISTW
	2,				// SETMAP
	1,				// SETMAP0
	1,				// EQOP
	1,				// NEQOP
	1,				// LTOP
	1,				// LEOP
	1,				// GTOP
	1,				// GEOP
	1,				// ADDOP
	1,				// SUBOP
	1,				// MULTOP
	1,				// DIVOP
	1,				// POWOP
	1,				// CONCOP
	1,				// MINUSOP
	1,				// NOTOP
	2, 3,				// ONTJMP, ONTJMPW
	2, 3,				// ONFJMP, ONFJMPW
	2, 3,				// JMP, JMPW
	2, 3,				// IFFJMP, IFFJMPW
	2, 3,				// IFTUPJMP, IFTUPJMPW
	2, 3,				// IFFUPJMP, IFFUPJMPW
	2,				// CLOSURE
	1, 1,				// CLOSURE0,1
	3,				// CALLFUNC
	2, 2,				// CALLFUNC0,1
	2,				// RETCODE
	2,				// SETLINE
	3,				// SETLINEW
	2,				// POP
	1, 1				// POP0,1
};

bool Decompiler::is_expr_opc(byte opc) {
	if (opc >= PUSHNIL && opc <= CREATEARRAYW)
		return true;
	if (opc == SETLIST0)
		return true;
	if (opc >= SETMAP && opc <= ONFJMPW)
		return true;
	if (opc >= CLOSURE && opc <= CLOSURE1)
		return true;
	if (opc == CALLFUNC1 || opc == SETLINE || opc == SETLINEW)
		return true;
	return false;
}

// Check for JMP or JMPW at end of "if" part
void Decompiler::get_else_part(byte *start, byte *&if_part_end, bool &has_else, byte *&else_part_end) {
	byte *last_instr = NULL;
	has_else = false;
	else_part_end = NULL;

	for (byte *instr_scan = start; instr_scan < if_part_end;
		instr_scan += instr_lens[*instr_scan])
		last_instr = instr_scan;
	if (last_instr != NULL &&
		(*last_instr == JMP || *last_instr == JMPW)) {
			has_else = true;
			else_part_end = if_part_end + last_instr[1];
			if (*last_instr == JMPW)
				else_part_end += (last_instr[2] << 8);
			if_part_end = last_instr;
	}
}

void Decompiler::decompileRange(byte *start, byte *end) {
	// First, scan for IFFUPJMP, which is used for repeat/until, so
	// we can recognize the start of such loops.  We only keep the
	// last value to match each address, which represents the outermost
	// repeat/until loop starting at that point.
	std::map<byte *, byte *> rev_iffupjmp_map;

	for (byte *scan = start; end == NULL || scan < end; scan += instr_lens[*scan]) {
		if (*scan == IFFUPJMP)
			rev_iffupjmp_map[scan + 2 - scan[1]] = scan;
		else if (*scan == IFFUPJMPW)
			rev_iffupjmp_map[scan + 3 - (scan[1] | (scan[2] << 8))] = scan;
		else if (*scan == ENDCODE)
			break;
	}

	while (end == NULL || start < end) {
		int locs_here = local_var_defs->count(start);
		if (locs_here > 0) {
			// There were local variable slots just pushed onto the stack
			// Print them out (in the second pass)

			// First, if there are multiple defined, it must be from
			// local x, y, z = f() or local a, b.  So just ignore the extra
			// entries.
			for (int i = 1; i < locs_here; i++) {
				delete stk->top(); stk->pop();
			}
			Expression *def = stk->top(); stk->pop();

			// Print the local variable names, and at the same time push
			// fake values onto the stack
			*os << indent_str << "local ";
			for (int i = 0; i < locs_here; i++) {
				std::string locname = localname(tf, tf->code[1] + stk->size());
				*os << locname;
				if (i + 1 < locs_here)
					*os << ", ";
				stk->push(new VarExpr(start, "<" + locname + " stack slot>"));
			}

			// Print the definition, unless it's nil
			VarExpr *v = dynamic_cast<VarExpr *>(def);
			if (v == NULL || v->name != "nil")
				*os << " = " << *def;
			*os << std::endl;
		}

		if (rev_iffupjmp_map.find(start) != rev_iffupjmp_map.end()) {
			// aha, do a repeat/until loop
			*os << indent_str << "repeat\n";
			Decompiler indented_dc = *this;
			indented_dc.indent_str += std::string(4, ' ');
			indented_dc.break_pos = rev_iffupjmp_map[start];
			indented_dc.break_pos += instr_lens[*indented_dc.break_pos];
			indented_dc.decompileRange(start, rev_iffupjmp_map[start]);

			Expression *e = stk->top(); stk->pop();
			*os << indent_str << "until " << *e << std::endl;
			delete e;

			start = indented_dc.break_pos;
			continue;
		}

		byte opc = *start++;
		int aux;

		switch (opc) {
	case ENDCODE:
		return;

	case PUSHNIL:
		aux = *start++;
		goto pushnil;

	case PUSHNIL0:
		aux = 0;
pushnil:
		for (int i = 0; i <= aux; i++)
			stk->push(new VarExpr(start, "nil")); // Cheat a little :)
		break;

	case PUSHNUMBER:
		aux = *start++;
		goto pushnumber;

	case PUSHNUMBER0:
	case PUSHNUMBER1:
	case PUSHNUMBER2:
		aux = opc - PUSHNUMBER0;
		goto pushnumber;

	case PUSHNUMBERW:
		aux = start[0] | (start[1] << 8);
		start += 2;
pushnumber:
		stk->push(new NumberExpr(start, aux));
		break;

	case PUSHCONSTANT:
		aux = *start++;
		goto pushconst;

	case PUSHCONSTANT0:
	case PUSHCONSTANT1:
	case PUSHCONSTANT2:
	case PUSHCONSTANT3:
	case PUSHCONSTANT4:
	case PUSHCONSTANT5:
	case PUSHCONSTANT6:
	case PUSHCONSTANT7:
		aux = opc - PUSHCONSTANT0;
		goto pushconst;

	case PUSHCONSTANTW:
		aux = start[0] | (start[1] << 8);
		start += 2;
pushconst:
		switch (ttype(tf->consts + aux)) {
	case LUA_T_STRING:
		stk->push(new StringExpr(start, tsvalue(tf->consts + aux)));
		break;
	case LUA_T_NUMBER:
		stk->push(new NumberExpr(start, nvalue(tf->consts + aux)));
		break;
	case LUA_T_PROTO:
		stk->push(new FuncExpr(start, tfvalue(tf->consts + aux), indent_str));
		break;
	default:
		*os << indent_str << "error: invalid constant type "
			<< int(ttype(tf->consts + aux)) << std::endl;
		}
		break;

	case PUSHUPVALUE:
		aux = *start++;
		goto pushupvalue;

	case PUSHUPVALUE0:
	case PUSHUPVALUE1:
		aux = opc - PUSHUPVALUE0;
pushupvalue:
		{
			if (aux >= num_upvals) {
				*os << indent_str << "error: invalid upvalue #"
					<< aux << std::endl;
			}

			std::ostringstream s;
			s << "%" << *upvals[aux];
			stk->push(new VarExpr(start, s.str()));
		}
		break;

	case PUSHLOCAL:
		aux = *start++;
		goto pushlocal;

	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
		aux = opc - PUSHLOCAL0;
pushlocal:
		stk->push(new VarExpr(start, localname(tf, aux)));
		break;

	case GETGLOBAL:
		aux = *start++;
		goto getglobal;

	case GETGLOBAL0:
	case GETGLOBAL1:
	case GETGLOBAL2:
	case GETGLOBAL3:
	case GETGLOBAL4:
	case GETGLOBAL5:
	case GETGLOBAL6:
	case GETGLOBAL7:
		aux = opc - GETGLOBAL0;
		goto getglobal;

	case GETGLOBALW:
		aux = start[0] | (start[1] << 8);
		start += 2;
getglobal:
		stk->push(new VarExpr(start, svalue(tf->consts + aux)));
		break;

	case GETTABLE:
		{
			Expression *index = stk->top(); stk->pop();
			Expression *table = stk->top(); stk->pop();

			stk->push(new IndexExpr(start, table, index));
		}
		break;

	case GETDOTTED:
		aux = *start++;
		goto getdotted;

	case GETDOTTED0:
	case GETDOTTED1:
	case GETDOTTED2:
	case GETDOTTED3:
	case GETDOTTED4:
	case GETDOTTED5:
	case GETDOTTED6:
	case GETDOTTED7:
		aux = opc - GETDOTTED0;
		goto getdotted;

	case GETDOTTEDW:
		aux = start[0] | (start[1] << 8);
		start += 2;
getdotted:
		{
			Expression *tbl = stk->top(); stk->pop();
			stk->push(new IndexExpr(start, tbl, new StringExpr(start, tsvalue(tf->consts + aux))));
		}
		break;

	case PUSHSELF:
		aux = *start++;
		goto pushself;

	case PUSHSELF0:
	case PUSHSELF1:
	case PUSHSELF2:
	case PUSHSELF3:
	case PUSHSELF4:
	case PUSHSELF5:
	case PUSHSELF6:
	case PUSHSELF7:
		aux = opc - PUSHSELF0;
		goto pushself;

	case PUSHSELFW:
		aux = start[0] | (start[1] << 8);
		start += 2;
pushself:
		{
			Expression *tbl = stk->top(); stk->pop();
			stk->push(new SelfExpr(start, tbl, new StringExpr(start, tsvalue(tf->consts + aux))));
			stk->push(new VarExpr(start, "<self>"));
			// Fake value, FuncCallExpr will handle it
		}
		break;

	case CREATEARRAY:
		start++;
		goto createarray;

	case CREATEARRAY0:
	case CREATEARRAY1:
		goto createarray;

	case CREATEARRAYW:
		start += 2;
createarray:
		stk->push(new ArrayExpr(start));
		break;

	case SETLOCAL:
	case SETLOCAL0:
	case SETLOCAL1:
	case SETLOCAL2:
	case SETLOCAL3:
	case SETLOCAL4:
	case SETLOCAL5:
	case SETLOCAL6:
	case SETLOCAL7:
	case SETGLOBAL:
	case SETGLOBAL0:
	case SETGLOBAL1:
	case SETGLOBAL2:
	case SETGLOBAL3:
	case SETGLOBAL4:
	case SETGLOBAL5:
	case SETGLOBAL6:
	case SETGLOBAL7:
	case SETGLOBALW:
	case SETTABLE0:
	case SETTABLE:
		start--;
		do_multi_assign(start);
		break;

	case SETLIST:
		start++;			// assume offset is correct
		goto setlist;

	case SETLISTW:
		start += 2;

	case SETLIST0:
setlist:
		aux = *start++;
		{
			ArrayExpr::mapping_list new_mappings;
			for (int i = 0; i < aux; i++) {
				Expression *val = stk->top(); stk->pop();
				new_mappings.push_front(std::make_pair((Expression *) NULL, val));
			}
			ArrayExpr *a = dynamic_cast<ArrayExpr *>(stk->top());
			if (a == NULL) {
				*os << indent_str << "error: attempt to setlist a non-array object\n";
			}
			// Append the new list
			a->mappings.splice(a->mappings.end(), new_mappings);
			a->pos = start;
		}
		break;

	case SETMAP:
		aux = *start++;
		goto setmap;

	case SETMAP0:
		aux = 0;
setmap:
		{
			ArrayExpr::mapping_list new_mappings;
			for (int i = 0; i <= aux; i++) {
				Expression *val = stk->top(); stk->pop();
				Expression *key = stk->top(); stk->pop();
				new_mappings.push_front(std::make_pair(key, val));
			}
			ArrayExpr *a = dynamic_cast<ArrayExpr *>(stk->top());
			if (a == NULL) {
				*os << indent_str << "error: attempt to setmap a non-array object\n";
			}
			// Append the new list
			a->mappings.splice(a->mappings.end(), new_mappings);
			a->pos = start;
		}
		break;

	case EQOP:
		do_binary_op(start, 1, false, " == ");
		break;

	case NEQOP:
		do_binary_op(start, 1, false, " ~= ");
		break;

	case LTOP:
		do_binary_op(start, 1, false, " < ");
		break;

	case LEOP:
		do_binary_op(start, 1, false, " <= ");
		break;

	case GTOP:
		do_binary_op(start, 1, false, " > ");
		break;

	case GEOP:
		do_binary_op(start, 1, false, " >= ");
		break;

	case ADDOP:
		do_binary_op(start, 3, false, " + ");
		break;

	case SUBOP:
		do_binary_op(start, 3, false, " - ");
		break;

	case MULTOP:
		do_binary_op(start, 4, false, " * ");
		break;

	case DIVOP:
		do_binary_op(start, 4, false, " / ");
		break;

	case POWOP:
		do_binary_op(start, 6, true, " ^ ");
		break;

	case CONCOP:
		do_binary_op(start, 2, false, "..");
		break;

	case MINUSOP:
		do_unary_op(start, 5, "-");
		break;

	case NOTOP:
		do_unary_op(start, 5, "not ");
		break;

	case ONTJMP:
		aux = *start++;
		goto ontjmp;

	case ONTJMPW:
		aux = start[0] | (start[1] << 8);
		start += 2;
ontjmp:
		// push_expr_1 ontjmp(label) push_expr_2 label:  -> expr_1 || expr_2
		decompileRange(start, start + aux);
		do_binary_op(start + aux, 0, false, " or ");
		start = start + aux;
		break;

	case ONFJMP:
		aux = *start++;
		goto onfjmp;

	case ONFJMPW:
		aux = start[0] | (start[1] << 8);
		start += 2;
onfjmp:
		// push_expr_1 onfjmp(label) push_expr_2 label:  -> expr_2 && expr_2
		decompileRange(start, start + aux);
		do_binary_op(start + aux, 0, false, " and ");
		start = start + aux;
		break;

	case JMP:
		aux = *start++;
		goto jmp;

	case JMPW:
		aux = start[0] | (start[1] << 8);
		start += 2;
jmp:
		{
			byte *dest = start + aux;
			if (dest == break_pos) {
				*os << indent_str << "break\n";
				break;
			}

			// otherwise, must be the start of a while statement
			byte *while_cond_end;
			for (while_cond_end = dest; end == NULL || while_cond_end < end;
				while_cond_end += instr_lens[*while_cond_end])
				if (*while_cond_end == IFTUPJMP || *while_cond_end == IFTUPJMPW)
					break;
			if (end != NULL && while_cond_end >= end) {
				*os << indent_str << "error: JMP not in break, while, if/else\n";
			}

			// push the while condition onto the stack
			decompileRange(dest, while_cond_end);

			*os << indent_str << "while " << *stk->top() << " do\n";
			delete stk->top();
			stk->pop();

			// decompile the while body
			Decompiler indented_dc = *this;
			indented_dc.indent_str += std::string(4, ' ');
			indented_dc.break_pos = while_cond_end + instr_lens[*while_cond_end];
			indented_dc.decompileRange(start, dest);

			*os << indent_str << "end\n";
			start = indented_dc.break_pos;
		}
		break;

	case IFFJMP:
		aux = *start++;
		goto iffjmp;

	case IFFJMPW:
		aux = start[0] | (start[1] << 8);
		start += 2;
iffjmp:
		{
			// Output an if/end, if/else/end, if/elseif/else/end, ... statement
			byte *if_part_end = start + aux;
			Decompiler indented_dc = *this;
			indented_dc.indent_str += std::string(4, ' ');

			*os << indent_str << "if " << *stk->top();
			delete stk->top();
			stk->pop();
			*os << " then\n";

			bool has_else;
			byte *else_part_end;
			get_else_part(start, if_part_end, has_else, else_part_end);

			// Output the if part
output_if:
			indented_dc.decompileRange(start, if_part_end);
			start = start + aux;

			if (has_else) {
				// Check whether the entire else part is a single
				// if or if/else statement
				byte *instr_scan = start;
				while (is_expr_opc(*instr_scan) && (end == NULL || instr_scan < else_part_end))
					instr_scan += instr_lens[*instr_scan];
				if ((end == NULL || instr_scan < else_part_end) && (*instr_scan == IFFJMP || *instr_scan == IFFJMPW)) {
					// OK, first line will be if, check if it will go all
					// the way through
					byte *new_start, *new_if_part_end, *new_else_part_end;
					bool new_has_else;
					if (*instr_scan == IFFJMP) {
						aux = instr_scan[1];
						new_start = instr_scan + 2;
					} else {
						aux = instr_scan[1] | (instr_scan[2] << 8);
						new_start = instr_scan + 3;
					}
					new_if_part_end = new_start + aux;
					get_else_part(new_start, new_if_part_end, new_has_else, new_else_part_end);
					if (new_if_part_end == else_part_end || (new_has_else && new_else_part_end == else_part_end)) {
						// Yes, output an elseif
						decompileRange(start, instr_scan); // push condition
						*os << indent_str << "elseif " << *stk->top() << " then\n";
						delete stk->top();
						stk->pop();

						start = new_start;
						if_part_end = new_if_part_end;
						has_else = new_has_else;
						else_part_end = new_else_part_end;
						goto output_if;
					}
				}
				*os << indent_str << "else\n";
				indented_dc.decompileRange(start, else_part_end);
				start = else_part_end;
			}
			*os << indent_str << "end\n";
		}
		break;

	case CLOSURE:
		aux = *start++;
		goto closure;

	case CLOSURE0:
	case CLOSURE1:
		aux = opc - CLOSURE0;
closure:
		{
			FuncExpr *f = dynamic_cast<FuncExpr *>(stk->top());
			if (f == NULL) {
				*os << indent_str << "error: closure requires a function\n";
			}
			stk->pop();
			f->num_upvals = aux;
			f->upvals = new Expression*[aux];
			for (int i = aux - 1; i >= 0; i--) {
				f->upvals[i] = stk->top(); stk->pop();
			}
			stk->push(f);
		}
		break;

	case CALLFUNC:
		aux = *start++;
		goto callfunc;

	case CALLFUNC0:
	case CALLFUNC1:
		aux = opc - CALLFUNC0;
callfunc:
		{
			int num_args = *start++;
			FuncCallExpr *e = new FuncCallExpr(start);
			e->num_args = num_args;
			e->args = new Expression*[num_args];
			for (int i = num_args - 1; i >= 0; i--) {
				e->args[i] = stk->top();
				stk->pop();
			}
			e->func = stk->top();
			stk->pop();
			if (aux == 0) {
				*os << indent_str << *e << std::endl;
				delete e;
			}
			else if (aux == 1 || aux == 255) // 255 for return f()
				stk->push(e);
			else {
				stk->push(e);
				for (int i = 1; i < aux; i++)
					stk->push(new VarExpr(start, "<extra result>"));
			}
		}
		break;

	case RETCODE:
		{
			int num_rets = stk->size() + tf->code[1] - *start++;
			ExprStack rets;

			for (int i = 0; i < num_rets; i++) {
				rets.push(stk->top());
				stk->pop();
			}
			*os << indent_str << "return";
			for (int i = 0; i < num_rets; i++) {
				*os << " " << *rets.top();
				delete rets.top();
				rets.pop();
				if (i + 1 < num_rets)
					*os << ",";
			}
			*os << std::endl;
		}
		break;

	case SETLINE:
		aux = *start++;
		goto setline;

	case SETLINEW:
		aux = start[0] | (start[1] << 8);
		start += 2;
setline:
		break;			// ignore line info

	case POP:
		aux = *start++;
		goto pop;

	case POP0:
	case POP1:
		aux = opc - POP0;
pop:
		for (int i = 0; i <= aux; i++) {
			local_var_defs->insert(stk->top()->pos);
			delete stk->top(); stk->pop();
		}
		break;

	default:
		*os << indent_str << "error: unrecognized opcode " << int(opc) << std::endl;
		return;
		}
	}
}

// Decompile the body of a function.
void decompile(std::ostream &os, TProtoFunc *tf, std::string indent_str, Expression **upvals, int num_upvals) {
	byte *instr = tf->code + 2;
	ExprStack s;
	std::ostringstream first_time;
	std::multiset<byte *> loc_vars;

	// First, do a preliminary pass to see where local variables are defined
	Decompiler dc;
	dc.os = &first_time;
	dc.stk = &s;
	dc.tf = tf;
	dc.indent_str = indent_str;
	dc.break_pos = NULL;
	dc.upvals = upvals;
	dc.num_upvals = num_upvals;
	dc.local_var_defs = &loc_vars;
	dc.decompileRange(instr, NULL);

	if (s.empty() && loc_vars.empty()) {
		// OK, it didn't actually have any local variables.  Just output
		// the results right now.
		os << first_time.str();
		return;
	}

	// See where the local variables were defined.
	while (! s.empty()) {
		loc_vars.insert(s.top()->pos);
		delete s.top(); s.pop();
	}

	// Now do the real decompilation
	dc.os = &os;
	dc.decompileRange(instr, NULL);

	while (! s.empty()) {
		delete s.top(); s.pop();
	}
}

}

int main(int argc, char *argv[]) {
	int filename_pos = 1;

	if (argc > 1 && strcmp(argv[1], "-t") == 0) {
		Grim::translateStrings = true; 
		filename_pos = 2;
	}
	if (argc != filename_pos + 1) {
		fprintf(stderr, "Usage: delua [-t] file.lua\n");
		exit(1);
	}
	char *filename = argv[filename_pos];
	FILE *f = fopen(filename, "rb");
	if (!f) {
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buff = new char[size];
	fread(buff, size, 1, f);
	fclose(f);

	Grim::lua_open();
	Grim::ZIO z;

	Grim::luaZ_mopen(&z, buff, size, "(buffer)");
	Grim::TProtoFunc *tf = Grim::luaU_undump1(&z);

	Grim::decompile(std::cout, tf, "", NULL, 0);

	delete[] buff;
	Grim::lua_close();
	return 0;
}
