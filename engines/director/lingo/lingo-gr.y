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
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"

extern int yylex();
extern int yyparse();

using namespace Director;

static void yyerror(const char *s) {
	LingoCompiler *compiler = g_lingo->_compiler;
	compiler->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d in %s id: %d",
		s, compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
		compiler->_assemblyContext->_id);
	if (compiler->_lines[2] != compiler->_lines[1])
		warning("# %3d: %s", compiler->_linenumber - 2, Common::String(compiler->_lines[2], compiler->_lines[1] - 1).c_str());

	if (compiler->_lines[1] != compiler->_lines[0])
		warning("# %3d: %s", compiler->_linenumber - 1, Common::String(compiler->_lines[1], compiler->_lines[0] - 1).c_str());

	const char *ptr = compiler->_lines[0];

	while (*ptr && *ptr != '\n')
		ptr++;

	warning("# %3d: %s", compiler->_linenumber, Common::String(compiler->_lines[0], ptr).c_str());

	Common::String arrow;
	for (uint i = 0; i < compiler->_colnumber; i++)
		arrow += ' ';

	warning("#      %s^ about here", arrow.c_str());
}

static void checkEnd(Common::String *token, Common::String *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(*expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect->c_str(), token->c_str());
			yyerror(err.c_str());
		}
	}
}

%}

%union {
	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field

	struct {
		Common::String *obj;
		Common::String *prop;
	} objectprop;

	Director::IDList *idlist;
	Director::Node *node;
	Director::NodeList *nodelist;
}

%token tUNARY

%token tLEXERROR
%token<i> tINT
%token<e> tTHEENTITY tTHEENTITYWITHID tTHEMENUITEMENTITY tTHEMENUITEMSENTITY
%token<f> tFLOAT
%token<s> tTHEFUNC tTHEFUNCINOF
%token<s> tVARID tSTRING tSYMBOL
%token<s> tENDCLAUSE tPLAYACCEL
%token<objectprop> tTHEOBJECTPROP
%token tCAST tFIELD tSCRIPT tWINDOW
%token tDOWN tELSE tELSIF tEXIT tGLOBAL tGO tGOLOOP tIF tIN tINTO tMACRO
%token tMOVIE tNEXT tOF tPREVIOUS tPUT tREPEAT tSET tTHEN tTO tWHEN
%token tWITH tWHILE tFACTORY tOPEN tPLAY tINSTANCE
%token tGE tLE tEQ tNEQ tAND tOR tNOT tMOD
%token tAFTER tBEFORE tCONCAT tCONTAINS tSTARTS tCHAR tITEM tLINE tWORD
%token tSPRITE tINTERSECTS tWITHIN tTELL tPROPERTY
%token tON tMETHOD tENDIF tENDREPEAT tENDTELL
%token tASSERTERROR

// TOP-LEVEL STUFF
%type<node> script scriptpart
%type<nodelist> scriptpartlist

// MACRO
%type<node> macro

// FACTORY
%type<node> factory method
%type<nodelist> methodlist nonemptymethodlist
%type<node> methodlistline

// HANDLER
%type<node> handler

// GENERIC VAR STUFF
%type<s> ID
%type<idlist> idlist nonemptyidlist

// STATEMENT
%type<node> stmt stmtoneliner proc definevars ifstmt ifelsestmt
%type<nodelist> stmtlist nonemptystmtlist
%type<node> stmtlistline

// EXPRESSION
%type<node> simpleexprnoparens simpleexpr expr
%type<nodelist> exprlist nonemptyexprlist

%left tAND tOR
%left '<' tLE '>' tGE tEQ tNEQ tCONTAINS tSTARTS
%left '&' tCONCAT
%left '+' '-'
%left '*' '/' '%' tMOD
%right tUNARY
// %right tCAST tFIELD tSCRIPT tWINDOW
// %nonassoc tVARID

%destructor { delete $$; } <s>

%%

// TOP-LEVEL STUFF

script: scriptpartlist					{ g_lingo->_compiler->_assemblyAST = new ScriptNode($scriptpartlist); } ;

scriptpartlist: scriptpart[item]				{
		NodeList *list = new NodeList;
		if ($item) {
			list->push_back($item);
		}
		$$ = list; }
	| scriptpartlist[prev] scriptpart[item]		{
		if ($item) {
			$prev->push_back($item);
		}
		$$ = $prev; }
	;

scriptpart:	'\n'						{ $$ = nullptr; }
	| macro
	| factory
	| handler
	| stmt
	;

// MACRO

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

macro: tMACRO ID idlist '\n' stmtlist	{ $$ = new HandlerNode($ID, $idlist, $stmtlist); } ;

// FACTORY

factory: tFACTORY ID '\n' methodlist	{ $$ = new FactoryNode($ID, $methodlist); } ;

method: tMETHOD ID idlist '\n' stmtlist	{ $$ = new HandlerNode($ID, $idlist, $stmtlist); } ;

methodlist: /* empty */				{ $$ = new NodeList; }
	| nonemptymethodlist
	;

nonemptymethodlist: methodlistline[item]			{
		NodeList *list = new NodeList;
		if ($item) {
			list->push_back($item);
		}
		$$ = list; }
	| methodlist[prev] '\n' methodlistline[item]	{
		if ($item) {
			$prev->push_back($item);
		}
		$$ = $prev; }
	;

methodlistline: '\n'				{ $$ = nullptr; }
	| method
	;

// HANDLER

handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n' {	// D3
		$$ = new HandlerNode($ID, $idlist, $stmtlist);
		checkEnd($tENDCLAUSE, $ID, false);
		delete $tENDCLAUSE; }
	| tON ID idlist '\n' stmtlist {	// D4. No 'end' clause
		$$ = new HandlerNode($ID, $idlist, $stmtlist); }
	;

endargdef:	/* nothing */
	| ID						{ delete $ID; }
	| endargdef ',' ID			{ delete $ID; }
	;

// GENERIC VAR STUFF

ID: tVARID
	| tAFTER		{ $$ = new Common::String("after"); }
	| tAND			{ $$ = new Common::String("and"); }
	| tBEFORE		{ $$ = new Common::String("before"); }
	| tCAST			{ $$ = new Common::String("cast"); }
	| tCHAR			{ $$ = new Common::String("char"); }
	| tDOWN			{ $$ = new Common::String("down"); }
	// tENDCLAUSE
	// tELSE
	| tEXIT			{ $$ = new Common::String("exit"); }
	// tFACTORY
	| tFIELD		{ $$ = new Common::String("field"); }
	// | tGLOBAL		{ $$ = new Common::String("global"); }
	// tIF
	| tIN			{ $$ = new Common::String("in"); }
	// | tINSTANCE		{ $$ = new Common::String("instance"); }
	| tINTERSECTS	{ $$ = new Common::String("intersects"); }
	| tINTO			{ $$ = new Common::String("into"); }
	| tITEM			{ $$ = new Common::String("item"); }
	| tLINE			{ $$ = new Common::String("line"); }
	// | tMACRO		{ $$ = new Common::String("macro"); }
	// | tMETHOD		{ $$ = new Common::String("method"); }
	| tMOD			{ $$ = new Common::String("mod"); }
	| tMOVIE		{ $$ = new Common::String("movie"); }
	| tNEXT			{ $$ = new Common::String("next"); }
	| tNOT			{ $$ = new Common::String("not"); }
	| tOF			{ $$ = new Common::String("of"); }
	// | tON			{ $$ = new Common::String("on"); }
	| tOPEN			{ $$ = new Common::String("open"); }
	| tOR			{ $$ = new Common::String("or"); }
	| tPLAY			{ $$ = new Common::String("play"); }		// FIXME: lexer includes "play frame"
	| tPLAYACCEL	{ $$ = new Common::String("playAccel"); }
	| tPREVIOUS		{ $$ = new Common::String("previous"); }
	// | tPROPERTY		{ $$ = new Common::String("property"); }
	| tPUT			{ $$ = new Common::String("put"); }
	| tREPEAT		{ $$ = new Common::String("repeat"); }
	| tSCRIPT		{ $$ = new Common::String("script"); }
	| tSET			{ $$ = new Common::String("set"); }
	| tSTARTS		{ $$ = new Common::String("starts"); }
	| tTELL			{ $$ = new Common::String("tell"); }
	// tTHEN
	| tTO			{ $$ = new Common::String("to"); }
	| tASSERTERROR	{ $$ = new Common::String("scummvmAssertError"); }
	| tSPRITE		{ $$ = new Common::String("sprite"); }
	| tWHEN			{ $$ = new Common::String("when"); }
	| tWHILE		{ $$ = new Common::String("while"); }
	| tWINDOW		{ $$ = new Common::String("window"); }
	| tWITH			{ $$ = new Common::String("with"); }
	| tWITHIN		{ $$ = new Common::String("within"); }
	| tWORD			{ $$ = new Common::String("word"); }
	;

idlist: /* empty */					{ $$ = new IDList; }
	| nonemptyidlist
	;

nonemptyidlist: ID[item]					{
		Common::Array<Common::String *> *list = new IDList;
		list->push_back($item);
		$$ = list; }
	| nonemptyidlist[prev] ',' ID[item]		{
		$prev->push_back($item);
		$$ = $prev; }
	;

// STATEMENT
// N.B. A statement must always be terminated by a '\n' symbol.
// Sometimes this '\n' is in a nested statement (e.g. tIF expr tTHEN stmt).
// It may not look like there's a '\n', but it's there.

stmt: stmtoneliner
	| ifstmt
	| ifelsestmt
	;

stmtoneliner: proc
	| definevars
	;

proc: ID '(' exprlist[args] ')' '\n'	{ $$ = new CmdNode($ID, $args); }
	| ID exprlist[args] '\n'			{ $$ = new CmdNode($ID, $args); }
	;

definevars: tGLOBAL idlist '\n'			{ $$ = new GlobalNode($idlist); }
	| tPROPERTY idlist '\n'				{ $$ = new PropertyNode($idlist); }
	| tINSTANCE idlist '\n'				{ $$ = new InstanceNode($idlist); }
	;

ifstmt: tIF expr tTHEN stmt {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back($stmt);
		$$ = new IfStmtNode($expr, stmtlist); }
	| tIF expr tTHEN '\n' stmtlist tENDIF '\n' {
		$$ = new IfStmtNode($expr, $stmtlist); }
	;

ifelsestmt: tIF expr tTHEN stmt[stmt1] tELSE stmt[stmt2] {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back($stmt1);
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back($stmt2);
		$$ = new IfElseStmtNode($expr, stmtlist1, stmtlist2); }
	| tIF expr tTHEN stmt[stmt1] tELSE '\n' stmtlist[stmtlist2] tENDIF '\n' {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back($stmt1);
		$$ = new IfElseStmtNode($expr, stmtlist1, $stmtlist2); }
	| tIF expr tTHEN '\n' stmtlist[stmtlist1] tELSE stmt[stmt2] {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back($stmt2);
		$$ = new IfElseStmtNode($expr, $stmtlist1, stmtlist2); }
	| tIF expr tTHEN '\n' stmtlist[stmtlist1] tELSE '\n' stmtlist[stmtlist2] tENDIF '\n' {
		$$ = new IfElseStmtNode($expr, $stmtlist1, $stmtlist2); }
	;

stmtlist: /* empty */				{ $$ = new NodeList; }
	| nonemptystmtlist
	;

nonemptystmtlist:
	stmtlistline[item]					{
		NodeList *list = new NodeList;
		if ($item) {
			list->push_back($item);
		}
		$$ = list; }
	| stmtlist[prev] stmtlistline[item]	{
		if ($item) {
			$prev->push_back($item);
		}
		$$ = $prev; }
	;

stmtlistline: '\n'					{ $$ = nullptr; }
	| stmt
	;

// EXPRESSION

simpleexprnoparens: tINT			{ $$ = new IntNode($tINT); }
	| tFLOAT						{ $$ = new FloatNode($tFLOAT); }
	| tSYMBOL						{ $$ = new SymbolNode($tSYMBOL); }	// D3
	| tSTRING						{ $$ = new StringNode($tSTRING); }
	| '+' simpleexpr[arg]  %prec tUNARY		{ $$ = $arg; }
	| '-' simpleexpr[arg]  %prec tUNARY		{ $$ = new UnaryOpNode(LC::c_negate, $arg); }
	| tNOT simpleexpr[arg]  %prec tUNARY	{ $$ = new UnaryOpNode(LC::c_not, $arg); }
	| ID '(' exprlist[args] ')'		{ $$ = new FuncNode($ID, $args); }
	| ID							{ $$ = new VarNode($ID); }
	;

simpleexpr: simpleexprnoparens
	| '(' expr ')'				{ $$ = $expr; }
	;

expr: simpleexpr				{ $$ = $simpleexpr; }
	| expr[a] '+' expr[b]		{ $$ = new BinaryOpNode(LC::c_add, $a, $b); }
	| expr[a] '-' expr[b]		{ $$ = new BinaryOpNode(LC::c_sub, $a, $b); }
	| expr[a] '*' expr[b]		{ $$ = new BinaryOpNode(LC::c_mul, $a, $b); }
	| expr[a] '/' expr[b]		{ $$ = new BinaryOpNode(LC::c_div, $a, $b); }
	| expr[a] tMOD expr[b]		{ $$ = new BinaryOpNode(LC::c_mod, $a, $b); }
	| expr[a] '>' expr[b]		{ $$ = new BinaryOpNode(LC::c_gt, $a, $b); }
	| expr[a] '<' expr[b]		{ $$ = new BinaryOpNode(LC::c_lt, $a, $b); }
	| expr[a] tEQ expr[b]		{ $$ = new BinaryOpNode(LC::c_eq, $a, $b); }
	| expr[a] tNEQ expr[b]		{ $$ = new BinaryOpNode(LC::c_neq, $a, $b); }
	| expr[a] tGE expr[b]		{ $$ = new BinaryOpNode(LC::c_ge, $a, $b); }
	| expr[a] tLE expr[b]		{ $$ = new BinaryOpNode(LC::c_le, $a, $b); }
	| expr[a] tAND expr[b]		{ $$ = new BinaryOpNode(LC::c_and, $a, $b); }
	| expr[a] tOR expr[b]		{ $$ = new BinaryOpNode(LC::c_or, $a, $b); }
	| expr[a] '&' expr[b]		{ $$ = new BinaryOpNode(LC::c_ampersand, $a, $b); }
	| expr[a] tCONCAT expr[b]	{ $$ = new BinaryOpNode(LC::c_concat, $a, $b); }
	| expr[a] tCONTAINS expr[b]	{ $$ = new BinaryOpNode(LC::c_contains, $a, $b); }
	| expr[a] tSTARTS expr[b]	{ $$ = new BinaryOpNode(LC::c_starts, $a, $b); }
	;

exprlist: /* empty */						{ $$ = new NodeList; }
	| nonemptyexprlist
	;

nonemptyexprlist: expr[item]				{
		NodeList *list = new NodeList; 
		list->push_back($item);
		$$ = list; }
	| nonemptyexprlist[prev] ',' expr[item]	{
		$prev->push_back($item);
		$$ = $prev; }
	;

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
