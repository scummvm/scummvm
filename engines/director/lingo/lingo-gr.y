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

// Heavily inspired by hoc
// Copyright (C) AT&T 1995
// All Rights Reserved
//
// Permission to use, copy, modify, and distribute this software and
// its documentation for any purpose and without fee is hereby
// granted, provided that the above copyright notice appear in all
// copies and that both that the copyright notice and this
// permission notice and warranty disclaimer appear in supporting
// documentation, and that the name of AT&T or any of its entities
// not be used in advertising or publicity pertaining to
// distribution of the software without specific, written prior
// permission.
//
// AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL AT&T OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
// SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
// IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
// THIS SOFTWARE.

%require "3.6"
%defines "engines/director/lingo/lingo-gr.h"
%output "engines/director/lingo/lingo-gr.cpp"
%define parse.error custom
%define parse.trace

// %glr-parser

%{
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"

extern int yylex();
extern int yyparse();

using namespace Director;

static void yyerror(const char *s) {
	g_lingo->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d in %s id: %d",
		s, g_lingo->_linenumber, g_lingo->_colnumber, scriptType2str(g_lingo->_assemblyContext->_scriptType),
		g_lingo->_assemblyContext->_id);
	if (g_lingo->_lines[2] != g_lingo->_lines[1])
		warning("# %3d: %s", g_lingo->_linenumber - 2, Common::String(g_lingo->_lines[2], g_lingo->_lines[1] - 1).c_str());

	if (g_lingo->_lines[1] != g_lingo->_lines[0])
		warning("# %3d: %s", g_lingo->_linenumber - 1, Common::String(g_lingo->_lines[1], g_lingo->_lines[0] - 1).c_str());

	const char *ptr = g_lingo->_lines[0];

	while (*ptr && *ptr != '\n')
		ptr++;

	warning("# %3d: %s", g_lingo->_linenumber, Common::String(g_lingo->_lines[0], ptr).c_str());

	Common::String arrow;
	for (uint i = 0; i < g_lingo->_colnumber; i++)
		arrow += ' ';

	warning("#      %s^ about here", arrow.c_str());
}

static void checkEnd(Common::String *token, const char *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect, token->c_str());
			yyerror(err.c_str());
		}
	}
}

static void inArgs() { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateInArgs; }
static void inDef()  { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateInDef; }
static void inNone() { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateNone; }
static void inLast() { g_lingo->_indef = g_lingo->_indefStore; }

static void startDef() {
	inArgs();
	g_lingo->_methodVarsStash = g_lingo->_methodVars;
	g_lingo->_methodVars = new VarTypeHash;

	for (VarTypeHash::iterator i = g_lingo->_methodVarsStash->begin(); i != g_lingo->_methodVarsStash->end(); ++i) {
		if (i->_value == kVarGlobal || i->_value == kVarProperty)
			(*g_lingo->_methodVars)[i->_key] = i->_value;
	}
	if (g_lingo->_inFactory) {
		for (DatumHash::iterator i = g_lingo->_assemblyContext->_properties.begin(); i != g_lingo->_assemblyContext->_properties.end(); ++i) {
			(*g_lingo->_methodVars)[i->_key] = kVarInstance;
		}
	}
}

static void endDef() {
	g_lingo->clearArgStack();
	inNone();

	delete g_lingo->_methodVars;
	g_lingo->_methodVars = g_lingo->_methodVarsStash;
	g_lingo->_methodVarsStash = nullptr;
}

static void startRepeat() {
	g_lingo->_repeatStack.push_back(new RepeatBlock);
}

static void endRepeat(uint exitPos, uint nextPos) {
	RepeatBlock *block = g_lingo->_repeatStack.back();
	g_lingo->_repeatStack.pop_back();
	for (uint i = 0; i < block->exits.size(); i++) {
		uint32 pos = block->exits[i];
		inst exit = 0;
		WRITE_UINT32(&exit, exitPos - (pos - 1));
		(*g_lingo->_currentAssembly)[pos] = exit;
	}
	for (uint i = 0; i < block->nexts.size(); i++) {
		uint32 pos = block->nexts[i];
		inst next = 0;
		WRITE_UINT32(&next, nextPos - (pos - 1));
		(*g_lingo->_currentAssembly)[pos] = next;
	}
	delete block;
}

static VarType globalCheck() {
	// If in a definition, assume variables are local unless
	// they were declared global with `global varname`
	if (g_lingo->_indef == kStateInDef) {
		return kVarLocal;
	}
	return kVarGlobal;
}

static void mVar(Common::String *s, VarType type) {
	if (!g_lingo->_methodVars->contains(*s)) {
		(*g_lingo->_methodVars)[*s] = type;
		if (type == kVarProperty || type == kVarInstance) {
			g_lingo->_assemblyContext->_properties[*s] = Datum();
		} else if (type == kVarGlobal) {
			g_lingo->varCreate(*s, true);
		}
	}
}

%}

%union {
	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

	struct {
		Common::String *obj;
		Common::String *prop;
	} objectprop;
}

%token UNARY

// Datum types
%token VOID VAR POINT RECT ARRAY OBJECT LEXERROR PARRAY
%token CASTREF FIELDREF CHUNKREF
%token<i> INT ARGC ARGCNORET

%token<e> THEENTITY THEENTITYWITHID THEMENUITEMENTITY THEMENUITEMSENTITY
%token<f> FLOAT
%token<s> THEFUNC THEFUNCINOF
%token<s> VARID STRING SYMBOL
%token<s> ENDCLAUSE tPLAYACCEL tMETHOD
%token<objectprop> THEOBJECTPROP
%token tCAST tFIELD tSCRIPT tWINDOW
%token tDOWN tELSE tELSIF tEXIT tGLOBAL tGO tGOLOOP tIF tIN tINTO tMACRO
%token tMOVIE tNEXT tOF tPREVIOUS tPUT tREPEAT tSET tTHEN tTO tWHEN
%token tWITH tWHILE tFACTORY tOPEN tPLAY tINSTANCE
%token tGE tLE tEQ tNEQ tAND tOR tNOT tMOD
%token tAFTER tBEFORE tCONCAT tCONTAINS tSTARTS tCHAR tITEM tLINE tWORD
%token tSPRITE tINTERSECTS tWITHIN tTELL tPROPERTY
%token tON tENDIF tENDREPEAT tENDTELL
%token tASSERTERROR

%type<code> asgn lbl expr if chunkexpr simpleexprnoparens
%type<code> tellstart reference simpleexpr list valuelist
%type<code> jump jumpifz varassign
%type<narg> argdef arglist nonemptyarglist linearlist proplist
%type<s> ID on

%left tAND tOR
%left '<' tLE '>' tGE tEQ tNEQ tCONTAINS tSTARTS
%left '&' tCONCAT
%left '+' '-'
%left '*' '/' '%' tMOD
%right UNARY tCAST tFIELD tSCRIPT tWINDOW
%nonassoc VARID

%destructor { delete $$; } <s>

%%

program: programline
	| program '\n' programline

programline: /* empty */
	| defn
	| stmt

ID: VARID
	| tCAST			{ $$ = new Common::String("cast"); }
	| tFIELD		{ $$ = new Common::String("field"); }
	| tSCRIPT		{ $$ = new Common::String("script"); }
	| tWINDOW		{ $$ = new Common::String("window"); }

asgn: tPUT expr tINTO ID 			{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		mVar($ID, globalCheck());
		g_lingo->code1(LC::c_assign);
		$$ = $expr;
		delete $ID; }
	| tPUT expr tINTO chunkexpr 	{
		g_lingo->code1(LC::c_assign);
		$$ = $expr; }
	// {put the number of menuItems of} menu into <expr>
	| tPUT THEMENUITEMSENTITY ID simpleexpr tINTO expr	{
		if (!$ID->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		g_lingo->code1(LC::c_themenuentitypush);
		g_lingo->codeInt($THEMENUITEMSENTITY[0]);
		g_lingo->codeInt($THEMENUITEMSENTITY[1]);
		g_lingo->code1(LC::c_assign);
		$$ = $expr; }
	| tPUT expr tAFTER ID 			{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		mVar($ID, globalCheck());
		g_lingo->code1(LC::c_putafter);
		$$ = $expr;
		delete $ID; }		// D3
	| tPUT expr tAFTER chunkexpr	{
		g_lingo->code1(LC::c_putafter);
		$$ = $expr; }
	| tPUT expr tBEFORE ID 			{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		mVar($ID, globalCheck());
		g_lingo->code1(LC::c_putbefore);
		$$ = $expr;
		delete $ID; }		// D3
	| tPUT expr tBEFORE chunkexpr 	{
		g_lingo->code1(LC::c_putbefore);
		$$ = $expr; }
	| tSET ID tEQ expr				{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		mVar($ID, globalCheck());
		g_lingo->code1(LC::c_assign);
		$$ = $expr;
		delete $ID; }
	| tSET THEENTITY tEQ expr		{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITY[0]);
		g_lingo->codeInt($THEENTITY[1]);
		$$ = $expr; }
	| tSET ID tTO expr				{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		mVar($ID, globalCheck());
		g_lingo->code1(LC::c_assign);
		$$ = $expr;
		delete $ID; }
	| tSET THEENTITY tTO expr		{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITY[0]);
		g_lingo->codeInt($THEENTITY[1]);
		$$ = $expr; }
	| tSET THEENTITYWITHID expr[entityId] tTO expr[value]	{
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITYWITHID[0]);
		g_lingo->codeInt($THEENTITYWITHID[1]);
		$$ = $value; }
	| tSET THEENTITYWITHID expr[entityId] tEQ expr[value]	{
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITYWITHID[0]);
		g_lingo->codeInt($THEENTITYWITHID[1]);
		$$ = $value; }
	// the <field> of menuItem <expr> of menu <expr>
	| tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tTO expr	{
		if (!$ID->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt($THEMENUITEMENTITY[0]);
		g_lingo->codeInt($THEMENUITEMENTITY[1]);
		$$ = $expr; }
	| tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tEQ expr	{
		if (!$ID->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt($THEMENUITEMENTITY[0]);
		g_lingo->codeInt($THEMENUITEMENTITY[1]);
		$$ = $expr; }
	| tSET THEOBJECTPROP tTO expr		{
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString($THEOBJECTPROP.obj->c_str());
		g_lingo->codeString($THEOBJECTPROP.prop->c_str());
		delete $THEOBJECTPROP.obj;
		delete $THEOBJECTPROP.prop;
		$$ = $expr; }
	| tSET THEOBJECTPROP tEQ expr		{
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString($THEOBJECTPROP.obj->c_str());
		g_lingo->codeString($THEOBJECTPROP.prop->c_str());
		delete $THEOBJECTPROP.obj;
		delete $THEOBJECTPROP.prop;
		$$ = $expr; }

stmtoneliner: asgn
	| proc

stmt: stmtoneliner
	| ifstmt
	// repeat while (expression = TRUE)
	//   statements
	// end repeat
	//
	| tREPEAT tWHILE lbl expr jumpifz[body] startrepeat stmtlist jump[end2] tENDREPEAT	{
		inst start = 0, end = 0;
		WRITE_UINT32(&start, $lbl - $end2 + 1);
		WRITE_UINT32(&end, $end2 - $body + 2);
		(*g_lingo->_currentAssembly)[$body] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[$end2] = start;	/* looping back */
		endRepeat($end2 + 1, $lbl);	}	/* code any exit/next repeats */

	// repeat with index = start to end
	//   statements
	// end repeat
	//
	| tREPEAT tWITH ID tEQ expr[init]
				{ g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString($ID->c_str());
				  mVar($ID, globalCheck()); }
			varassign
				{ g_lingo->code1(LC::c_eval);
				  g_lingo->codeString($ID->c_str()); }
			tTO expr[finish]
				{ g_lingo->code1(LC::c_le); } jumpifz startrepeat stmtlist tENDREPEAT {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentAssembly->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, $varassign - pos + 2);
		WRITE_UINT32(&end, pos - $jumpifz + 2);
		(*g_lingo->_currentAssembly)[pos] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[$jumpifz] = end;	/* end, if cond fails */
		endRepeat(pos + 1, nextPos); }	/* code any exit/next repeats */


	// repeat with index = high down to low
	//   statements
	// end repeat
	//
	| tREPEAT tWITH ID tEQ expr[init]
				{ g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString($ID->c_str());
				  mVar($ID, globalCheck()); }
			varassign
				{ g_lingo->code1(LC::c_eval);
				  g_lingo->codeString($ID->c_str()); }
			tDOWN tTO expr[finish]
				{ g_lingo->code1(LC::c_ge); }
			jumpifz startrepeat stmtlist tENDREPEAT {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_sub);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentAssembly->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, $varassign - pos + 2);
		WRITE_UINT32(&end, pos - $jumpifz + 2);
		(*g_lingo->_currentAssembly)[pos] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[$jumpifz] = end;	/* end, if cond fails */
		endRepeat(pos + 1, nextPos); }	/* code any exit/next repeats */

	// repeat with index in list
	//   statements
	// end repeat
	//
	| tREPEAT tWITH ID tIN expr
				{ g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
			lbl
				{ g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
			jumpifz
				{ g_lingo->code1(LC::c_stackpeek);	// get list
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(1);
				  Common::String getAt("getAt");
				  g_lingo->codeFunc(&getAt, 2);
				  g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString($ID->c_str());
				  mVar($ID, globalCheck());
				  g_lingo->code1(LC::c_assign); }
			startrepeat stmtlist tENDREPEAT {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);			// Increment counter

		int jump = g_lingo->code2(LC::c_jump, 0);

		int end2 = g_lingo->code1(LC::c_stackdrop);	// remove list, size, counter
		g_lingo->codeInt(3);

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, $lbl - jump);
		WRITE_UINT32(&end, end2 - $jumpifz + 1);

		(*g_lingo->_currentAssembly)[jump + 1] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[$jumpifz] = end;		/* end, if cond fails */
		endRepeat(end2, nextPos); }	/* code any exit/next repeats */

	| tNEXT tREPEAT 		{
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
	| tWHEN ID tTHEN expr	{
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString($ID->c_str()); }
	| tTELL expr '\n' tellstart stmtlist lbl tENDTELL { g_lingo->code1(LC::c_telldone); }
	| tTELL expr tTO tellstart stmtoneliner lbl { g_lingo->code1(LC::c_telldone); }
	| tASSERTERROR asserterrorstart stmtoneliner { g_lingo->code1(LC::c_asserterrordone); }
	| error	'\n'			{ yyerrok; }

startrepeat:	/* nothing */	{ startRepeat(); }

tellstart:	  /* empty */	{ g_lingo->code1(LC::c_tell); }

asserterrorstart:	/* empty */	{ g_lingo->code1(LC::c_asserterror); }

ifstmt: if expr jumpifz[then] tTHEN stmtlist jump[else1] elseifstmtlist lbl[end3] tENDIF {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, $else1 + 1 - $then + 1);
		WRITE_UINT32(&end3, $end3 - $else1 + 1);
		(*g_lingo->_currentAssembly)[$then] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[$else1] = end3;		/* end, if cond fails */
		g_lingo->processIf($else1, $end3); }
	| if expr jumpifz[then] tTHEN stmtlist jump[else1] elseifstmtlist tELSE stmtlist lbl[end3] tENDIF {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, $else1 + 1 - $then + 1);
		WRITE_UINT32(&end, $end3 - $else1 + 1);
		(*g_lingo->_currentAssembly)[$then] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[$else1] = end;		/* end, if cond fails */
		g_lingo->processIf($else1, $end3); }

elseifstmtlist:	/* nothing */
	| elseifstmtlist elseifstmt

elseifstmt: tELSIF expr jumpifz[then] tTHEN stmtlist jump[end3] {
		inst else1 = 0;
		WRITE_UINT32(&else1, $end3 + 1 - $then + 1);
		(*g_lingo->_currentAssembly)[$then] = else1;	/* end, if cond fails */
		g_lingo->codeLabel($end3); }

jumpifz:	/* nothing */	{
		g_lingo->code2(LC::c_jumpifz, 0);
		$$ = g_lingo->_currentAssembly->size() - 1; }

jump:		/* nothing */	{
		g_lingo->code2(LC::c_jump, 0);
		$$ = g_lingo->_currentAssembly->size() - 1; }

varassign:		/* nothing */	{
		g_lingo->code1(LC::c_assign);
		$$ = g_lingo->_currentAssembly->size() - 1; }

if:	  tIF					{
		g_lingo->codeLabel(0); } // Mark beginning of the if() statement

lbl:	  /* nothing */		{ $$ = g_lingo->_currentAssembly->size(); }

stmtlist: stmtlistline
	| stmtlistline '\n' stmtlist

stmtlistline: /* empty */
	| stmt

simpleexprnoparens: INT		{
		$$ = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt($INT); }
	| FLOAT		{
		$$ = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat($FLOAT); }
	| SYMBOL	{											// D3
		$$ = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString($SYMBOL->c_str());
		delete $SYMBOL; }
	| STRING		{
		$$ = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString($STRING->c_str());
		delete $STRING; }
	| '+' simpleexpr[arg]  %prec UNARY	{ $$ = $arg; }
	| '-' simpleexpr[arg]  %prec UNARY	{ $$ = $arg; g_lingo->code1(LC::c_negate); }
	| tNOT simpleexpr  %prec UNARY		{ g_lingo->code1(LC::c_not); }
	| reference
	| THEENTITY					{
		$$ = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, $THEENTITY[0]);
		WRITE_UINT32(&f, $THEENTITY[1]);
		g_lingo->code2(e, f); }
	| THEENTITYWITHID simpleexpr {
		$$ = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, $THEENTITYWITHID[0]);
		WRITE_UINT32(&f, $THEENTITYWITHID[1]);
		g_lingo->code2(e, f); }
	| THEFUNCINOF simpleexpr	{
		$$ = g_lingo->codeFunc($THEFUNCINOF, 1);
		delete $THEFUNCINOF; }
	| THEFUNC tOF simpleexpr	{
		$$ = g_lingo->codeFunc($THEFUNC, 1);
		delete $THEFUNC; }
	| THEOBJECTPROP				{
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString($THEOBJECTPROP.obj->c_str());
		g_lingo->codeString($THEOBJECTPROP.prop->c_str());
		delete $THEOBJECTPROP.obj;
		delete $THEOBJECTPROP.prop; }
    | tSPRITE expr tINTERSECTS simpleexpr	{ g_lingo->code1(LC::c_intersects); }
    | tSPRITE expr tWITHIN simpleexpr		{ g_lingo->code1(LC::c_within); }
	| list
	| ID[func] '(' ID[method] ')' {
			g_lingo->code1(LC::c_varpush);
			g_lingo->codeString($method->c_str());
			g_lingo->codeFunc($func, 1);
			delete $func;
			delete $method; }
	| ID[func] '(' ID[method] ',' { g_lingo->code1(LC::c_varpush); g_lingo->codeString($method->c_str()); }
				nonemptyarglist ')' {
			g_lingo->codeFunc($func, $nonemptyarglist + 1);
			delete $func;
			delete $method; }
	| ID '(' arglist ')'		{
		$$ = g_lingo->codeFunc($ID, $arglist);
		delete $ID; }
	| ID						{
		if (g_lingo->_builtinConsts.contains(*$ID)) {
			$$ = g_lingo->code1(LC::c_constpush);
		} else {
			$$ = g_lingo->code1(LC::c_eval);
		}
		g_lingo->codeString($ID->c_str());
		delete $ID; }

simpleexpr: simpleexprnoparens
	| '(' expr[arg] ')'			{ $$ = $arg; }

expr: simpleexpr { $$ = $simpleexpr; }
	| expr '+' expr				{ g_lingo->code1(LC::c_add); }
	| expr '-' expr				{ g_lingo->code1(LC::c_sub); }
	| expr '*' expr				{ g_lingo->code1(LC::c_mul); }
	| expr '/' expr				{ g_lingo->code1(LC::c_div); }
	| expr tMOD expr			{ g_lingo->code1(LC::c_mod); }
	| expr '>' expr				{ g_lingo->code1(LC::c_gt); }
	| expr '<' expr				{ g_lingo->code1(LC::c_lt); }
	| expr tEQ expr				{ g_lingo->code1(LC::c_eq); }
	| expr tNEQ expr			{ g_lingo->code1(LC::c_neq); }
	| expr tGE expr				{ g_lingo->code1(LC::c_ge); }
	| expr tLE expr				{ g_lingo->code1(LC::c_le); }
	| expr tAND expr			{ g_lingo->code1(LC::c_and); }
	| expr tOR expr				{ g_lingo->code1(LC::c_or); }
	| expr '&' expr				{ g_lingo->code1(LC::c_ampersand); }
	| expr tCONCAT expr			{ g_lingo->code1(LC::c_concat); }
	| expr tCONTAINS expr		{ g_lingo->code1(LC::c_contains); }
	| expr tSTARTS expr			{ g_lingo->code1(LC::c_starts); }

chunkexpr:  tFIELD simpleexprnoparens	{
		Common::String field("field");
		g_lingo->codeFunc(&field, 1); }
	| tCAST simpleexprnoparens		{
		Common::String cast("cast");
		g_lingo->codeFunc(&cast, 1); }
	| tCHAR expr tOF simpleexpr				{ g_lingo->code1(LC::c_charOf); }
	| tCHAR expr tTO expr tOF simpleexpr	{ g_lingo->code1(LC::c_charToOf); }
	| tITEM expr tOF simpleexpr				{ g_lingo->code1(LC::c_itemOf); }
	| tITEM expr tTO expr tOF simpleexpr	{ g_lingo->code1(LC::c_itemToOf); }
	| tLINE expr tOF simpleexpr				{ g_lingo->code1(LC::c_lineOf); }
	| tLINE expr tTO expr tOF simpleexpr	{ g_lingo->code1(LC::c_lineToOf); }
	| tWORD expr tOF simpleexpr				{ g_lingo->code1(LC::c_wordOf); }
	| tWORD expr tTO expr tOF simpleexpr	{ g_lingo->code1(LC::c_wordToOf); }

reference: 	chunkexpr
	| tSCRIPT simpleexprnoparens	{
		Common::String script("script");
		g_lingo->codeFunc(&script, 1); }
	| tWINDOW simpleexprnoparens	{
		Common::String window("window");
		g_lingo->codeFunc(&window, 1); }

proc: tPUT expr					{
		Common::String put("put");
		g_lingo->codeCmd(&put, 1); }
	| gotofunc
	| playfunc
	| tEXIT tREPEAT				{
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
	| tEXIT						{ g_lingo->code1(LC::c_procret); }
	| tGLOBAL					{ inArgs(); } globallist { inLast(); }
	| tPROPERTY					{ inArgs(); } propertylist { inLast(); }
	| tINSTANCE					{ inArgs(); } instancelist { inLast(); }
	| tOPEN expr tWITH expr		{
		Common::String open("open");
		g_lingo->codeCmd(&open, 2); }
	| tOPEN expr 				{
		Common::String open("open");
		g_lingo->codeCmd(&open, 1); }
	| ID[func] '(' ID[method] ')' {
			g_lingo->code1(LC::c_varpush);
			g_lingo->codeString($method->c_str());
			g_lingo->codeCmd($func, 1);
			delete $func;
			delete $method; }
	| ID[func] '(' ID[method] ',' { g_lingo->code1(LC::c_varpush); g_lingo->codeString($method->c_str()); }
				nonemptyarglist ')' {
			g_lingo->codeCmd($func, $nonemptyarglist + 1);
			delete $func;
			delete $method; }
	| ID '(' arglist ')'		{
		g_lingo->codeCmd($ID, $arglist);
		delete $ID; }
	| ID arglist				{
		g_lingo->codeCmd($ID, $arglist);
		delete $ID; }

globallist:		/* nothing */
	| ID						{
		mVar($ID, kVarGlobal);
		delete $ID; }
	| ID ',' globallist			{
		mVar($ID, kVarGlobal);
		delete $ID; }

propertylist:	/* nothing */
	| ID						{
		mVar($ID, kVarProperty);
		delete $ID; }
	| ID ',' propertylist		{
		mVar($ID, kVarProperty);
		delete $ID; }

instancelist:	/* nothing */
	| ID						{
		mVar($ID, kVarInstance);
		delete $ID; }
	| ID ',' instancelist		{
		mVar($ID, kVarInstance);
		delete $ID; }

// go {to} {frame} whichFrame {of movie whichMovie}
// go {to} {frame "Open23" of} movie whichMovie
// go loop
// go next
// go previous
// go to {frame} whichFrame {of movie whichMovie}
// go to {frame whichFrame of} movie whichMovie
gotofunc: tGOLOOP				{ g_lingo->code1(LC::c_gotoloop); }
	| tGO tNEXT					{ g_lingo->code1(LC::c_gotonext); }
	| tGO tPREVIOUS				{ g_lingo->code1(LC::c_gotoprevious); }
	| tGO expr 					{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
	| tGO expr gotomovie		{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
	| tGO gotomovie				{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }

gotomovie: tOF tMOVIE expr
	| tMOVIE expr

playfunc: tPLAY expr 			{ // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
	| tPLAY expr gotomovie		{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
	| tPLAY gotomovie			{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
	| tPLAYACCEL { g_lingo->codeSetImmediate(true); } arglist {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeCmd($tPLAYACCEL, $arglist);
		delete $tPLAYACCEL; }

// macro
//
// Special Note  The macro keyword is retained in Director 3.0 to maintain compatibility
// with scripts developed under Version 2.0. When writing new scripts, or editing old
// scripts, you should use handlers instead of macros. (Handlers are defined with the on keyword.)
//
// Syntax:
//
// --  [comment]
// macro macroName [argument1] [, argument2]
// [, argument3]
// [statement1]
// [statement2]
//
// Keyword.  Defines a macro. A macro is a multiple-line script defined
// in the Text window. Macros can accept arguments (inputs) and
// optionally return a result. Macros can call other macros and can be
// called from any other script or factory.
//
// The first line of a castmember in the Text window that contains a macro must be
// a comment (--). You can define more than one macro in a given text castmember.
// The macro definition ends where the next macro (or factory) begins.
//
// See also:
//   on keyword
defn: tMACRO { startDef(); } ID
			lbl argdef '\n' argstore stmtlist 		{
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*$ID, $lbl, $argdef);
		endDef();
		delete $ID; }
	| tFACTORY ID	{ g_lingo->codeFactory(*$ID); delete $ID; }
	| tMETHOD {
			startDef();
			Common::String me("me");
			g_lingo->codeArg(&me);
			mVar(&me, kVarArgument);
		} lbl argdef '\n' argstore stmtlist {
			g_lingo->code1(LC::c_procret);
			g_lingo->codeDefine(*$tMETHOD, $lbl, $argdef + 1);
			endDef();
			delete $tMETHOD; }
	| on lbl argdef '\n' argstore stmtlist ENDCLAUSE endargdef {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*$on, $lbl, $argdef);
		endDef();

		checkEnd($ENDCLAUSE, $on->c_str(), false);
		delete $on;
		delete $ENDCLAUSE; }
	| on lbl argdef '\n' argstore stmtlist {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*$on, $lbl, $argdef);
		endDef();
		delete $on; }

on:  tON { startDef(); } ID 	{ $$ = $ID; }

argname: ID						{ g_lingo->codeArg($ID); mVar($ID, kVarArgument); delete $ID; }

argdef:  /* nothing */ 			{ $$ = 0; }
	| argname					{ $$ = 1; }
	| argname ',' argdef		{ $$ = $3 + 1; }

endargdef:	/* nothing */
	| ID						{ delete $ID; }
	| endargdef ',' ID			{ delete $ID; }

argstore:	  /* nothing */		{ inDef(); }

arglist:  /* nothing */ 		{ $$ = 0; }
	| expr						{ $$ = 1; }
	| expr ',' arglist			{ $$ = $3 + 1; }

nonemptyarglist:  expr			{ $$ = 1; }
	| expr ','					{ $$ = 1; }
	| expr ',' nonemptyarglist	{ $$ = $3 + 1; }

list: '[' valuelist ']'			{ $$ = $valuelist; }

valuelist:	/* nothing */		{ $$ = g_lingo->code2(LC::c_arraypush, 0); }
	| ':'						{ $$ = g_lingo->code2(LC::c_proparraypush, 0); }
	| proplist	 { $$ = g_lingo->code1(LC::c_proparraypush); $$ = g_lingo->codeInt($proplist); }
	| linearlist { $$ = g_lingo->code1(LC::c_arraypush); $$ = g_lingo->codeInt($linearlist); }

linearlist: expr				{ $$ = 1; }
	| linearlist ',' expr		{ $$ = $1 + 1; }

proplist:  proppair				{ $$ = 1; }
	| proplist ',' proppair		{ $$ = $1 + 1; }

proppair: SYMBOL ':' expr {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString($SYMBOL->c_str());
		delete $SYMBOL; }
	| STRING ':' expr 	{
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString($STRING->c_str());
		delete $STRING; }
	| ID ':' expr 	{
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString($ID->c_str());
		delete $ID; }


%%

int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

	Common::String msg = "syntax error, ";

	// Report the unexpected token.
	yysymbol_kind_t lookahead = yypcontext_token(ctx);
	if (lookahead != YYSYMBOL_YYEMPTY)
		msg += Common::String::format("unexpected %s", yysymbol_name(lookahead));

	// Report the tokens expected at this point.
	enum { TOKENMAX = 10 };
	yysymbol_kind_t expected[TOKENMAX];

	int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
	if (n < 0)
		// Forward errors to yyparse.
		res = n;
	else
		for (int i = 0; i < n; ++i)
			msg += Common::String::format("%s %s", i == 0 ? ": expected" : " or", yysymbol_name(expected[i]));

	yyerror(msg.c_str());

	return res;
}
