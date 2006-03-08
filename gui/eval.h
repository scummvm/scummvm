/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#ifndef GUI_EVAL_H
#define GUI_EVAL_H

#include "common/stdafx.h"
#include "common/str.h"
#include "common/map.h"

namespace GUI {

using Common::String;
using Common::Map;

#define EVAL_UNDEF_VAR -13375

enum tokenTypes {
	tDelimiter,
	tVariable,
	tNumber,
	tString
};

enum evalErrors {
	eSyntaxError,
	eExtraBracket,
	eUnclosedBracket,
	eBadExpr,
	eUndefVar
};

struct IgnoreCaseComparator {
  int operator()(const String& x, const String& y) const { return scumm_stricmp(x.c_str(), y.c_str()); }
};

class Eval {
public:
	Eval();
	~Eval();

	int eval(const String &input, const String &section, const String &name, int startpos);
	void setAlias(const String &section, const String name, const String value);
	void setVariable(const String &section, const String name, const String value);

	void setParent(const String name);

	void setVariable(const String name, int val) { _vars[name] = val; }
	void setAlias(const String name, const String val) { _aliases[name] = val; }

	int getVar(String s) { return getVar_(s.c_str()); };

	void reset();

	typedef Map<String, int, IgnoreCaseComparator> VariablesMap;
	typedef Map<String, String, IgnoreCaseComparator> AliasesMap;

private:
	void getToken();
	void level2(int *);
	void level3(int *);
	void level4(int *);
	void level5(int *);
	void primitive(int *);
	void arith(char op, int *r, int *h);
	void unary(char op, int *r);
	void exprError(int error);
	int getVar_(const char *s, bool includeAliases = true);
	int getBuiltinVar(const char *s);

	char _input[256];
	String _section;
	String _name;

	int _startpos;

	int _tokenType;
	int _pos;

	char _token[256];

	AliasesMap _aliases;
	VariablesMap _vars;
};

} // end of namespace GUI

#endif
