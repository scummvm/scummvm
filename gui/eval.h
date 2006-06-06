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
#include "common/hashmap.h"

namespace GUI {

using Common::String;
using Common::HashMap;

enum {
	EVAL_UNDEF_VAR  = -13375,
	EVAL_STRING_VAR = -13376
};

class Eval {
public:
	Eval();
	~Eval();

	int eval(const String &input, const String &section, const String &name, int startpos);
	void setVar(const String &section, const String &name, const String &value);

	void setParent(const String &name);

	void setVar(const String &name, int val) { _vars[name.c_str()] = val; }
	void setStringVar(const String &name, const String &val) { _strings[name.c_str()] = val; }
	void setAlias(const char *name, const String &val) { _aliases[name] = val; }

	int getVar(const char *s) { return getVar_(s); }
	int getVar(const char *s, int def) {
		int val = getVar_(s);
		return (val == EVAL_UNDEF_VAR) ? def : val;
	}

	int getVar(const String &s) { return getVar(s.c_str()); }
	int getVar(const String &s, int def) { return getVar(s.c_str(), def); }

	const String &getStringVar(const char *name) { return _strings[name]; }

	uint getNumVars() { return _vars.size(); }

	void reset();

	char *lastToken() { return _token; }

	struct CharStar_EqualTo {
		bool operator()(const char *x, const char *y) const { return strcmp(x, y) == 0; }
	};

	//typedef HashMap<String, int> VariablesMap;
	typedef HashMap<const char *, int, Common::Hash<const char *>, CharStar_EqualTo> VariablesMap;
	typedef HashMap<const char *, String, Common::Hash<const char *>, CharStar_EqualTo> AliasesMap;
	typedef HashMap<const char *, String, Common::Hash<const char *>, CharStar_EqualTo> StringsMap;

private:
	enum TokenTypes {
		tNone,
		tDelimiter,
		tVariable,
		tNumber,
		tString
	};

	enum EvalErrors {
		eSyntaxError,
		eExtraBracket,
		eUnclosedBracket,
		eBadExpr,
		eUndefVar,
		eMissingQuote
	};


	void getToken();
	void level2(int *);
	void level3(int *);
	void level4(int *);
	void level5(int *);
	void primitive(int *);
	void arith(char op, int *r, int *h);
	void unary(char op, int *r);
	void exprError(EvalErrors error);
	int getVar_(const char *s, bool includeAliases = true);
	int getBuiltinVar(const char *s);
	void loadConstants();

	char _input[256];
	String _section;
	String _name;

	int _startpos;

	TokenTypes _tokenType;
	int _pos;

	char _token[256];

	AliasesMap _aliases;
	VariablesMap _vars;
	StringsMap _strings;
};

} // end of namespace GUI

#endif
