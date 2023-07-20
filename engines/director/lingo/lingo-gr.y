/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "director/lingo/lingo-the.h"

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
	Director::ChunkType chunktype;
	struct {
		Common::String *eventName;
		Common::String *stmt;
	} w;

	Director::IDList *idlist;
	Director::Node *node;
	Director::NodeList *nodelist;
}

%token tUNARY

%token<i> tINT
%token<f> tFLOAT
%token<s> tVARID tSTRING tSYMBOL
%token<s> tENDCLAUSE
%token tCAST tFIELD tSCRIPT tWINDOW
%token tDELETE tDOWN tELSE tEXIT tFRAME tGLOBAL tGO tHILITE tIF tIN tINTO tMACRO
%token tMOVIE tNEXT tOF tPREVIOUS tPUT tREPEAT tSET tTHEN tTO tWHEN
%token tWITH tWHILE tFACTORY tOPEN tPLAY tINSTANCE
%token tGE tLE tEQ tNEQ tAND tOR tNOT tMOD
%token tAFTER tBEFORE tCONCAT tCONTAINS tSTARTS
%token tCHAR tCHARS tITEM tITEMS tLINE tLINES tWORD tWORDS
%token tABBREVIATED tABBREV tABBR tLONG tSHORT
%token tDATE tLAST tMENU tMENUS tMENUITEM tMENUITEMS tNUMBER tTHE tTIME tXTRAS tCASTLIBS
%token tSOUND tSPRITE tINTERSECTS tWITHIN tTELL tPROPERTY
%token tON tMETHOD tENDIF tENDREPEAT tENDTELL
%token tASSERTERROR

%type<w> tWHEN

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
%type<s> CMDID ID
%type<idlist> idlist nonemptyidlist

// STATEMENT
%type<node> stmt stmt_insideif stmtoneliner
%type<node> proc asgn definevars
%type<node> ifstmt ifelsestmt loop tell when
%type<nodelist> cmdargs frameargs stmtlist nonemptystmtlist stmtlist_insideif nonemptystmtlist_insideif
%type<node> stmtlistline stmtlistline_insideif

// EXPRESSION
%type<node> simpleexpr_nounarymath simpleexpr
%type<node> unarymath
%type<node> expr expr_nounarymath expr_noeq sprite
%type<node> var varorchunk varorthe
%type<chunktype> chunktype
%type<node> the theobj menu thedatetime thenumberof
%type<node> writablethe writabletheobj
%type<node> list proppair
%type<node> chunk object
%type<nodelist> refargs proplist exprlist nonemptyexprlist

%left tAND tOR
%left '<' tLE '>' tGE tEQ tNEQ tCONTAINS tSTARTS
%left '&' tCONCAT
%left '+' '-'
%left '*' '/' tMOD
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
	| tENDCLAUSE endargdef '\n'			{ $$ = nullptr; delete $tENDCLAUSE; } // stray `end`s are allowed for some reason
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
	| nonemptymethodlist[prev] methodlistline[item]	{
		if ($item) {
			$prev->push_back($item);
		}
		$$ = $prev; }
	;

methodlistline: '\n'				{ $$ = nullptr; }
	| method
	| tENDCLAUSE endargdef '\n'			{ $$ = nullptr; delete $tENDCLAUSE; } // stray `end`s are allowed for some reason
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

// This is only the identifiers that can appaear at the start of a line
// and will not conflict with other statement types.
CMDID: tVARID
	| tABBREVIATED	{ $$ = new Common::String("abbreviated"); }
	| tABBREV		{ $$ = new Common::String("abbrev"); }
	| tABBR			{ $$ = new Common::String("abbr"); }
	| tAFTER		{ $$ = new Common::String("after"); }
	| tBEFORE		{ $$ = new Common::String("before"); }
	| tCAST			{ $$ = new Common::String("cast"); }
	| tCHAR			{ $$ = new Common::String("char"); }
	| tCHARS		{ $$ = new Common::String("chars"); }
	| tDATE			{ $$ = new Common::String("date"); }
	| tDELETE		{ $$ = new Common::String("delete"); }
	| tDOWN			{ $$ = new Common::String("down"); }
	| tFIELD		{ $$ = new Common::String("field"); }
	| tFRAME		{ $$ = new Common::String("frame"); }
	| tHILITE		{ $$ = new Common::String("hilite"); }
	| tIN			{ $$ = new Common::String("in"); }
	| tINTERSECTS	{ $$ = new Common::String("intersects"); }
	| tINTO			{ $$ = new Common::String("into"); }
	| tITEM			{ $$ = new Common::String("item"); }
	| tITEMS		{ $$ = new Common::String("items"); }
	| tLAST			{ $$ = new Common::String("last"); }
	| tLINE			{ $$ = new Common::String("line"); }
	| tLINES		{ $$ = new Common::String("lines"); }
	| tLONG			{ $$ = new Common::String("long"); }
	| tMENU			{ $$ = new Common::String("menu"); }
	| tMENUITEM		{ $$ = new Common::String("menuItem"); }
	| tMENUITEMS	{ $$ = new Common::String("menuItems"); }
	| tMOVIE		{ $$ = new Common::String("movie"); }
	| tNEXT			{ $$ = new Common::String("next"); }
	| tNUMBER		{ $$ = new Common::String("number"); }
	| tOF			{ $$ = new Common::String("of"); }
	| tPREVIOUS		{ $$ = new Common::String("previous"); }
	| tREPEAT		{ $$ = new Common::String("repeat"); }
	| tSCRIPT		{ $$ = new Common::String("script"); }
	| tASSERTERROR	{ $$ = new Common::String("scummvmAssertError"); }
	| tSHORT		{ $$ = new Common::String("short"); }
	| tSOUND		{ $$ = new Common::String("sound"); }
	| tSPRITE		{ $$ = new Common::String("sprite"); }
	| tTHE			{ $$ = new Common::String("the"); }
	| tTIME			{ $$ = new Common::String("time"); }
	| tTO			{ $$ = new Common::String("to"); }
	| tWHILE		{ $$ = new Common::String("while"); }
	| tWINDOW		{ $$ = new Common::String("window"); }
	| tWITH			{ $$ = new Common::String("with"); }
	| tWITHIN		{ $$ = new Common::String("within"); }
	| tWORD			{ $$ = new Common::String("word"); }
	| tWORDS		{ $$ = new Common::String("words"); }
	;

ID: CMDID
	| tELSE			{ $$ = new Common::String("else"); }
	| tENDCLAUSE	{ $$ = new Common::String("end"); delete $tENDCLAUSE; }
	| tEXIT			{ $$ = new Common::String("exit"); }
	| tFACTORY		{ $$ = new Common::String("factory"); }
	| tGLOBAL		{ $$ = new Common::String("global"); }
	| tGO			{ $$ = new Common::String("go"); }
	| tIF			{ $$ = new Common::String("if"); }
	| tINSTANCE		{ $$ = new Common::String("instance"); }
	| tMACRO		{ $$ = new Common::String("macro"); }
	| tMETHOD		{ $$ = new Common::String("method"); }
	| tON			{ $$ = new Common::String("on"); }
	| tOPEN			{ $$ = new Common::String("open"); }
	| tPLAY			{ $$ = new Common::String("play"); }
	| tPROPERTY		{ $$ = new Common::String("property"); }
	| tPUT			{ $$ = new Common::String("put"); }
	| tSET			{ $$ = new Common::String("set"); }
	| tTELL			{ $$ = new Common::String("tell"); }
	| tTHEN			{ $$ = new Common::String("then"); }
	;

idlist: /* empty */					{ $$ = new IDList; }
	| nonemptyidlist
	| nonemptyidlist ',' // allow trailing comma
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

stmt: stmt_insideif
	| tENDIF '\n'						{ $$ = nullptr; } // stray `end if`s are allowed for some reason
	;

stmt_insideif: stmtoneliner
	| ifstmt
	| ifelsestmt
	| loop
	| tell
	| when
	;

stmtoneliner: proc
	| asgn
	| definevars
	;

proc: CMDID cmdargs '\n'				{ $$ = new CmdNode($CMDID, $cmdargs, g_lingo->_compiler->_linenumber - 1); }
	| tPUT cmdargs '\n'					{ $$ = new CmdNode(new Common::String("put"), $cmdargs, g_lingo->_compiler->_linenumber - 1); }
	| tGO cmdargs '\n'					{ $$ = new CmdNode(new Common::String("go"), $cmdargs, g_lingo->_compiler->_linenumber - 1); }
	| tGO frameargs '\n'				{ $$ = new CmdNode(new Common::String("go"), $frameargs, g_lingo->_compiler->_linenumber - 1); }
	| tPLAY cmdargs '\n'				{ $$ = new CmdNode(new Common::String("play"), $cmdargs, g_lingo->_compiler->_linenumber - 1); }
	| tPLAY frameargs '\n'				{ $$ = new CmdNode(new Common::String("play"), $frameargs, g_lingo->_compiler->_linenumber - 1); }
	| tOPEN cmdargs '\n'				{ $$ = new CmdNode(new Common::String("open"), $cmdargs, g_lingo->_compiler->_linenumber - 1); }
	| tOPEN expr[arg1] tWITH expr[arg2] '\n' {
		NodeList *args = new NodeList;
		args->push_back($arg1);
		args->push_back($arg2);
		$$ = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
	| tNEXT tREPEAT '\n'				{ $$ = new NextRepeatNode(); }
	| tEXIT tREPEAT '\n'				{ $$ = new ExitRepeatNode(); }
	| tEXIT '\n'						{ $$ = new ExitNode(); }
	| tDELETE chunk '\n'				{ $$ = new DeleteNode($chunk); }
	| tHILITE chunk '\n'				{ $$ = new HiliteNode($chunk); }
	| tASSERTERROR stmtoneliner			{ $$ = new AssertErrorNode($stmtoneliner); }
	;

cmdargs: /* empty */									{
		// This matches `cmd`
		$$ = new NodeList; }
	| expr trailingcomma								{
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back($expr);
		$$ = args; }
	| expr ',' nonemptyexprlist[args] trailingcomma		{
		// This matches `cmd args, ...)
		$args->insert_at(0, $expr);
		$$ = $args; }
	| expr expr_nounarymath trailingcomma				{
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back($expr);
		args->push_back($expr_nounarymath);
		$$ = args; }
	| expr expr_nounarymath ',' nonemptyexprlist[args] trailingcomma	{
		// This matches `cmd arg arg, ...`
		$args->insert_at(0, $expr_nounarymath);
		$args->insert_at(0, $expr);
		$$ = $args; }
	| '(' ')'							{
		// This matches `cmd()`
		$$ = new NodeList; }
	| '(' expr ',' ')' {
		// This matches `cmd(args,)`
		NodeList *args = new NodeList;
		args->push_back($expr);
		$$ = args; }
	| '(' expr ',' nonemptyexprlist[args] trailingcomma ')' {
		// This matches `cmd(args, ...)`
		$args->insert_at(0, $expr);
		$$ = $args; }
	;

trailingcomma: /* empty */ | ',' ;

frameargs:
	// On the off chance that we encounter something like `play frame done`
	// we will wrap the frame arg in a FrameNode. This has no purpose other than
	// to avoid detecting this case as `play done`.
	tFRAME expr[frame]						{
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode($frame));
		$$ = args; }
	| tMOVIE expr[movie]							{
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode($movie));
		$$ = args; }
	| tFRAME expr[frame] tOF tMOVIE expr[movie]		{
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode($frame));
		args->push_back(new MovieNode($movie));
		$$ = args; }
	| expr[frame] tOF tMOVIE expr[movie]			{
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back($frame);
		args->push_back(new MovieNode($movie));
		$$ = args; }
	| tFRAME expr[frame] expr_nounarymath[movie]	{
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode($frame));
		args->push_back($movie);
		$$ = args; }
	;

asgn: tPUT expr tINTO varorchunk '\n'	{ $$ = new PutIntoNode($expr, $varorchunk); }
	| tPUT expr tAFTER varorchunk '\n'	{ $$ = new PutAfterNode($expr, $varorchunk); }
	| tPUT expr tBEFORE varorchunk '\n'	{ $$ = new PutBeforeNode($expr, $varorchunk); }
	| tSET varorthe to expr '\n'		{ $$ = new SetNode($varorthe, $expr); }
	;

to: tTO | tEQ ;

definevars: tGLOBAL idlist '\n'			{ $$ = new GlobalNode($idlist); }
	| tPROPERTY idlist '\n'				{ $$ = new PropertyNode($idlist); }
	| tINSTANCE idlist '\n'				{ $$ = new InstanceNode($idlist); }
	;

ifstmt: tIF expr tTHEN stmt {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back($stmt);
		$$ = new IfStmtNode($expr, stmtlist); }
	| tIF expr tTHEN '\n' stmtlist_insideif endif {
		$$ = new IfStmtNode($expr, $stmtlist_insideif); }
	;

ifelsestmt: tIF expr tTHEN stmt[stmt1] tELSE stmt[stmt2] {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back($stmt1);
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back($stmt2);
		$$ = new IfElseStmtNode($expr, stmtlist1, stmtlist2); }
	| tIF expr tTHEN stmt[stmt1] tELSE '\n' stmtlist_insideif[stmtlist2] endif {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back($stmt1);
		$$ = new IfElseStmtNode($expr, stmtlist1, $stmtlist2); }
	| tIF expr tTHEN '\n' stmtlist_insideif[stmtlist1] tELSE stmt[stmt2] {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back($stmt2);
		$$ = new IfElseStmtNode($expr, $stmtlist1, stmtlist2); }
	| tIF expr tTHEN '\n' stmtlist_insideif[stmtlist1] tELSE '\n' stmtlist_insideif[stmtlist2] endif {
		$$ = new IfElseStmtNode($expr, $stmtlist1, $stmtlist2); }
	;

endif: /* empty */	{ warning("LingoCompiler::parse: no end if"); }
	| tENDIF '\n' ;

loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n' {
		$$ = new RepeatWhileNode($expr, $stmtlist); }
	| tREPEAT tWITH ID tEQ expr[start] tTO expr[end] '\n' stmtlist tENDREPEAT '\n' {
		$$ = new RepeatWithToNode($ID, $start, false, $end, $stmtlist); }
	| tREPEAT tWITH ID tEQ expr[start] tDOWN tTO expr[end] '\n' stmtlist tENDREPEAT '\n' {
		$$ = new RepeatWithToNode($ID, $start, true, $end, $stmtlist); }
	| tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n' {
		$$ = new RepeatWithInNode($ID, $expr, $stmtlist); }
	;

tell: tTELL expr tTO stmtoneliner				{
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back($stmtoneliner);
		$$ = new TellNode($expr, stmtlist); }
	| tTELL expr '\n' stmtlist tENDTELL '\n'	{
		$$ = new TellNode($expr, $stmtlist); }
	;

when: tWHEN '\n'					{ $$ = new WhenNode($tWHEN.eventName, $tWHEN.stmt); } ;

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
	| nonemptystmtlist[prev] stmtlistline[item]	{
		if ($item) {
			$prev->push_back($item);
		}
		$$ = $prev; }
	;

stmtlistline: '\n'					{ $$ = nullptr; }
	| stmt
	;

stmtlist_insideif: /* empty */		{ $$ = new NodeList; }
	| nonemptystmtlist_insideif
	;

nonemptystmtlist_insideif:
	stmtlistline_insideif[item]		{
		NodeList *list = new NodeList;
		if ($item) {
			list->push_back($item);
		}
		$$ = list; }
	| nonemptystmtlist_insideif[prev] stmtlistline_insideif[item]	{
		if ($item) {
			$prev->push_back($item);
		}
		$$ = $prev; }
	;

stmtlistline_insideif: '\n'			{ $$ = nullptr; }
	| stmt_insideif
	;

// EXPRESSION

simpleexpr_nounarymath:
	  tINT							{ $$ = new IntNode($tINT); }
	| tFLOAT						{ $$ = new FloatNode($tFLOAT); }
	| tSYMBOL						{ $$ = new SymbolNode($tSYMBOL); }	// D3
	| tSTRING						{ $$ = new StringNode($tSTRING); }
	| tNOT simpleexpr[arg]  %prec tUNARY	{ $$ = new UnaryOpNode(LC::c_not, $arg); }
	| ID '(' ')'					{ $$ = new FuncNode($ID, new NodeList); }
	| ID '(' nonemptyexprlist[args] trailingcomma ')'	{ $$ = new FuncNode($ID, $args); }
	| '(' expr ')'					{ $$ = $expr; } ;
	| var
	| chunk
	| object
	| the
	| list
	;

var: ID							{ $$ = new VarNode($ID); } ;

varorchunk: var
	| chunk
	;

varorthe: var
	| writablethe
	;

chunk: tFIELD refargs		{ $$ = new FuncNode(new Common::String("field"), $refargs); }
	| tCAST refargs			{ $$ = new FuncNode(new Common::String("cast"), $refargs); }
	| tCHAR expr[idx] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkChar, $idx, nullptr, $src); }
	| tCHAR expr[start] tTO expr[end] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkChar, $start, $end, $src); }
	| tWORD expr[idx] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkWord, $idx, nullptr, $src); }
	| tWORD expr[start] tTO expr[end] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkWord, $start, $end, $src); }
	| tITEM expr[idx] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkItem, $idx, nullptr, $src); }
	| tITEM expr[start] tTO expr[end] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkItem, $start, $end, $src); }
	| tLINE expr[idx] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkLine, $idx, nullptr, $src); }
	| tLINE expr[start] tTO expr[end] tOF simpleexpr[src]	{
		$$ = new ChunkExprNode(kChunkLine, $start, $end, $src); }
	| tTHE tLAST chunktype inof simpleexpr	{ $$ = new TheLastNode($chunktype, $simpleexpr); }
	;

chunktype: tCHAR				{ $$ = kChunkChar; }
	| tWORD						{ $$ = kChunkWord; }
	| tITEM						{ $$ = kChunkItem; }
	| tLINE						{ $$ = kChunkLine; }
	;

object: tSCRIPT refargs		{ $$ = new FuncNode(new Common::String("script"), $refargs); }
	| tWINDOW refargs		{ $$ = new FuncNode(new Common::String("window"), $refargs); }
	;

refargs: simpleexpr								{
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back($simpleexpr);
		$$ = args; }
	| '(' ')'									{
		// This matches `ref()`
		$$ = new NodeList; }
	| '(' expr ',' ')' {
		// This matches `ref(args,)`
		NodeList *args = new NodeList;
		args->push_back($expr);
		$$ = args; }
	| '(' expr ',' nonemptyexprlist[args] trailingcomma ')'	{
		// This matches `ref(args, ...)`
		$args->insert_at(0, $expr);
		$$ = $args; }
	;

the: tTHE ID							{ $$ = new TheNode($ID); }
	| tTHE ID tOF theobj				{ $$ = new TheOfNode($ID, $theobj); }
	| tTHE tNUMBER tOF theobj			{ $$ = new TheOfNode(new Common::String("number"), $theobj); }
	| thedatetime
	| thenumberof
	;

theobj: simpleexpr
	| menu
	| tMENUITEM simpleexpr[item] tOF tMENU simpleexpr[menu]	{ $$ = new MenuItemNode($item, $menu); }
	| tSOUND simpleexpr[arg]			{ $$ = new SoundNode($arg); }
	| tSPRITE simpleexpr[arg]			{ $$ = new SpriteNode($arg); }
	;

menu: tMENU	simpleexpr[arg]				{ $$ = new MenuNode($arg); } ;

thedatetime: tTHE tABBREVIATED tDATE	{ $$ = new TheDateTimeNode(kTheAbbr, kTheDate); }
	| tTHE tABBREVIATED tTIME			{ $$ = new TheDateTimeNode(kTheAbbr, kTheTime); }
	| tTHE tABBREV tDATE				{ $$ = new TheDateTimeNode(kTheAbbr, kTheDate); }
	| tTHE tABBREV tTIME				{ $$ = new TheDateTimeNode(kTheAbbr, kTheTime); }
	| tTHE tABBR tDATE					{ $$ = new TheDateTimeNode(kTheAbbr, kTheDate); }
	| tTHE tABBR tTIME					{ $$ = new TheDateTimeNode(kTheAbbr, kTheTime); }
	| tTHE tLONG tDATE					{ $$ = new TheDateTimeNode(kTheLong, kTheDate); }
	| tTHE tLONG tTIME					{ $$ = new TheDateTimeNode(kTheLong, kTheTime); }
	| tTHE tSHORT tDATE					{ $$ = new TheDateTimeNode(kTheShort, kTheDate); }
	| tTHE tSHORT tTIME					{ $$ = new TheDateTimeNode(kTheShort, kTheTime); }
	;

thenumberof:
	  tTHE tNUMBER tOF tCHARS inof simpleexpr	{ $$ = new TheNumberOfNode(kNumberOfChars, $simpleexpr); }
	| tTHE tNUMBER tOF tWORDS inof simpleexpr	{ $$ = new TheNumberOfNode(kNumberOfWords, $simpleexpr); }
	| tTHE tNUMBER tOF tITEMS inof simpleexpr	{ $$ = new TheNumberOfNode(kNumberOfItems, $simpleexpr); }
	| tTHE tNUMBER tOF tLINES inof simpleexpr	{ $$ = new TheNumberOfNode(kNumberOfLines, $simpleexpr); }
	| tTHE tNUMBER tOF tMENUITEMS inof menu		{ $$ = new TheNumberOfNode(kNumberOfMenuItems, $menu); }
	| tTHE tNUMBER tOF tMENUS					{ $$ = new TheNumberOfNode(kNumberOfMenus, nullptr); }
	| tTHE tNUMBER tOF tXTRAS					{ $$ = new TheNumberOfNode(kNumberOfXtras, nullptr); } // D5
	| tTHE tNUMBER tOF tCASTLIBS				{ $$ = new TheNumberOfNode(kNumberOfCastlibs, nullptr); } // D5
	;

inof: tIN | tOF ;

writablethe: tTHE ID					{ $$ = new TheNode($ID); }
	| tTHE ID tOF writabletheobj		{ $$ = new TheOfNode($ID, $writabletheobj); }
	;

writabletheobj: simpleexpr
	| tMENU	expr_noeq[arg]				{ $$ = new MenuNode($arg); } ;
	| tMENUITEM expr_noeq[item] tOF tMENU expr_noeq[menu]	{ $$ = new MenuItemNode($item, $menu); }
	| tSOUND expr_noeq[arg]				{ $$ = new SoundNode($arg); }
	| tSPRITE expr_noeq[arg]			{ $$ = new SpriteNode($arg); }
	;

list: '[' exprlist ']'			{ $$ = new ListNode($exprlist); }
	| '[' ':' ']'				{ $$ = new PropListNode(new NodeList); }
	| '[' proplist ']'			{ $$ = new PropListNode($proplist); }
	;

// A property list must start with a proppair, but it may be followed by
// keyless expressions, which will be compiled as equivalent to the
// proppair <index>: <expr>.
proplist: proppair[item]				{
		NodeList *list = new NodeList;
		list->push_back($item);
		$$ = list; }
	| proplist[prev] ',' proppair[item]	{
		$prev->push_back($item);
		$$ = $prev; }
	| proplist[prev] ',' expr[item]	{
		$prev->push_back($item);
		$$ = $prev; }
	;

proppair: tSYMBOL ':' expr		{ $$ = new PropPairNode(new SymbolNode($tSYMBOL), $expr); }
	| ID ':' expr				{ $$ = new PropPairNode(new SymbolNode($ID), $expr); }
	| tSTRING ':' expr 			{ $$ = new PropPairNode(new StringNode($tSTRING), $expr); }
	| tINT ':' expr             { $$ = new PropPairNode(new IntNode($tINT), $expr); }
	| tFLOAT ':' expr           { $$ = new PropPairNode(new FloatNode($tFLOAT), $expr); }
	;

unarymath: '+' simpleexpr[arg]  %prec tUNARY	{ $$ = $arg; }
	| '-' simpleexpr[arg]  %prec tUNARY			{ $$ = new UnaryOpNode(LC::c_negate, $arg); }
	;

simpleexpr: simpleexpr_nounarymath
	| unarymath
	;

// REMEMBER TO SYNC THIS WITH expr_nounarymath and expr_noeq!
expr: simpleexpr
	| sprite
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

// This is the same as expr except it can't start with a unary math operator.
// It's ugly but unfortunately necessary to allow two expressions in a row with no delimeter.
// Without this, `cmd 1 + 1` could be interpreted as either `cmd(1 + 1)` or `cmd(1, +1)`.
// We only want to allow the first interpretation, so we must exclude unary math from the second expression.
expr_nounarymath: simpleexpr_nounarymath
	| sprite
	| expr_nounarymath[a] '+' expr[b]		{ $$ = new BinaryOpNode(LC::c_add, $a, $b); }
	| expr_nounarymath[a] '-' expr[b]		{ $$ = new BinaryOpNode(LC::c_sub, $a, $b); }
	| expr_nounarymath[a] '*' expr[b]		{ $$ = new BinaryOpNode(LC::c_mul, $a, $b); }
	| expr_nounarymath[a] '/' expr[b]		{ $$ = new BinaryOpNode(LC::c_div, $a, $b); }
	| expr_nounarymath[a] tMOD expr[b]		{ $$ = new BinaryOpNode(LC::c_mod, $a, $b); }
	| expr_nounarymath[a] '>' expr[b]		{ $$ = new BinaryOpNode(LC::c_gt, $a, $b); }
	| expr_nounarymath[a] '<' expr[b]		{ $$ = new BinaryOpNode(LC::c_lt, $a, $b); }
	| expr_nounarymath[a] tEQ expr[b]		{ $$ = new BinaryOpNode(LC::c_eq, $a, $b); }
	| expr_nounarymath[a] tNEQ expr[b]		{ $$ = new BinaryOpNode(LC::c_neq, $a, $b); }
	| expr_nounarymath[a] tGE expr[b]		{ $$ = new BinaryOpNode(LC::c_ge, $a, $b); }
	| expr_nounarymath[a] tLE expr[b]		{ $$ = new BinaryOpNode(LC::c_le, $a, $b); }
	| expr_nounarymath[a] tAND expr[b]		{ $$ = new BinaryOpNode(LC::c_and, $a, $b); }
	| expr_nounarymath[a] tOR expr[b]		{ $$ = new BinaryOpNode(LC::c_or, $a, $b); }
	| expr_nounarymath[a] '&' expr[b]		{ $$ = new BinaryOpNode(LC::c_ampersand, $a, $b); }
	| expr_nounarymath[a] tCONCAT expr[b]	{ $$ = new BinaryOpNode(LC::c_concat, $a, $b); }
	| expr_nounarymath[a] tCONTAINS expr[b]	{ $$ = new BinaryOpNode(LC::c_contains, $a, $b); }
	| expr_nounarymath[a] tSTARTS expr[b]	{ $$ = new BinaryOpNode(LC::c_starts, $a, $b); }
	;

expr_noeq: simpleexpr
	| sprite
	| expr_noeq[a] '+' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_add, $a, $b); }
	| expr_noeq[a] '-' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_sub, $a, $b); }
	| expr_noeq[a] '*' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_mul, $a, $b); }
	| expr_noeq[a] '/' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_div, $a, $b); }
	| expr_noeq[a] tMOD expr_noeq[b]		{ $$ = new BinaryOpNode(LC::c_mod, $a, $b); }
	| expr_noeq[a] '>' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_gt, $a, $b); }
	| expr_noeq[a] '<' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_lt, $a, $b); }
	| expr_noeq[a] tNEQ expr_noeq[b]		{ $$ = new BinaryOpNode(LC::c_neq, $a, $b); }
	| expr_noeq[a] tGE expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_ge, $a, $b); }
	| expr_noeq[a] tLE expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_le, $a, $b); }
	| expr_noeq[a] tAND expr_noeq[b]		{ $$ = new BinaryOpNode(LC::c_and, $a, $b); }
	| expr_noeq[a] tOR expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_or, $a, $b); }
	| expr_noeq[a] '&' expr_noeq[b]			{ $$ = new BinaryOpNode(LC::c_ampersand, $a, $b); }
	| expr_noeq[a] tCONCAT expr_noeq[b]		{ $$ = new BinaryOpNode(LC::c_concat, $a, $b); }
	| expr_noeq[a] tCONTAINS expr_noeq[b]	{ $$ = new BinaryOpNode(LC::c_contains, $a, $b); }
	| expr_noeq[a] tSTARTS expr_noeq[b]		{ $$ = new BinaryOpNode(LC::c_starts, $a, $b); }
	;

sprite: tSPRITE expr tINTERSECTS simpleexpr	{ $$ = new IntersectsNode($expr, $simpleexpr); }
	| tSPRITE expr tWITHIN simpleexpr		{ $$ = new WithinNode($expr, $simpleexpr); }
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
