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


%debug

%require "3.5"
%defines "engines/director/lingo/lingo-gr.h"
%output "engines/director/lingo/lingo-gr.cpp"

// %glr-parser

%{
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-gr.h"

extern int yylex();
extern int yyparse();

using namespace Director;
void yyerror(const char *s) {
	g_lingo->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
}

void checkEnd(Common::String *token, const char *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect, token->c_str());
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
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

	struct {
		Common::String *os;
		int oe;
	} objectfield;

	struct {
		Common::String *obj;
		Common::String *field;
	} objectref;
}

%token UNARY
%token CASTREF VOID VAR POINT RECT ARRAY OBJECT REFERENCE LEXERROR
%token<i> INT ARGC ARGCNORET
%token<e> THEENTITY THEENTITYWITHID THEMENUITEMENTITY THEMENUITEMSENTITY
%token<f> FLOAT
%token<s> BLTIN FBLTIN RBLTIN
%token<s> ID STRING HANDLER SYMBOL
%token<s> ENDCLAUSE tPLAYACCEL tMETHOD
%token<objectfield> THEOBJECTFIELD
%token<objectref> THEOBJECTREF
%token tDOWN tELSE tELSIF tEXIT tGLOBAL tGO tIF tIN tINTO tLOOP tMACRO
%token tMOVIE tNEXT tOF tPREVIOUS tPUT tREPEAT tSET tTHEN tTO tWHEN
%token tWITH tWHILE tNLELSE tFACTORY tOPEN tPLAY tDONE tINSTANCE
%token tGE tLE tEQ tNEQ tAND tOR tNOT tMOD
%token tAFTER tBEFORE tCONCAT tCONTAINS tSTARTS tCHAR tITEM tLINE tWORD
%token tSPRITE tINTERSECTS tWITHIN tTELL tPROPERTY
%token tON tENDIF tENDREPEAT tENDTELL

%type<code> asgn begin elseif end expr if when repeatwhile chunkexpr
%type<code> repeatwith stmtlist tellstart reference simpleexpr list valuelist
%type<narg> argdef arglist nonemptyarglist linearlist proplist
%type<s> on

%left tAND tOR
%left '<' tLE '>' tGE tEQ tNEQ tCONTAINS tSTARTS
%left '&' tCONCAT
%left '+' '-'
%left '*' '/' '%' tMOD
%right UNARY

%destructor { delete $$; } <s>
%destructor { delete $$.os; } <objectfield>

%%

program: program '\n' programline
	| programline
	| error	'\n'		{ yyerrok; }

programline: /* empty */
	| defn
	| stmt

asgn: tPUT expr tINTO ID 		{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_assign);
		$$ = $expr;
		delete $ID; }
	| tPUT expr tINTO reference 		{
		g_lingo->code1(LC::c_assign);
		$$ = $expr; }
	// {put the number of menuItems of} menu into <expr>
	| tPUT THEMENUITEMSENTITY ID simpleexpr tINTO expr	{
		if (!$ID->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItems entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt($THEMENUITEMSENTITY[0]);
		g_lingo->codeInt($THEMENUITEMSENTITY[1]);
		$$ = $expr; }
	| tPUT expr tAFTER expr 		{ $$ = g_lingo->code1(LC::c_after); }		// D3
	| tPUT expr tBEFORE expr 		{ $$ = g_lingo->code1(LC::c_before); }		// D3
	| tSET ID tEQ expr			{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_assign);
		$$ = $expr;
		delete $ID; }
	| tSET THEENTITY tEQ expr	{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITY[0]);
		g_lingo->codeInt($THEENTITY[1]);
		$$ = $expr; }
	| tSET ID tTO expr			{
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString($ID->c_str());
		g_lingo->code1(LC::c_assign);
		$$ = $expr;
		delete $ID; }
	| tSET THEENTITY tTO expr	{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITY[0]);
		g_lingo->codeInt($THEENTITY[1]);
		$$ = $expr; }
	| tSET THEENTITYWITHID simpleexpr tTO expr	{
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITYWITHID[0]);
		g_lingo->codeInt($THEENTITYWITHID[1]);
		$$ = $expr; }
	| tSET THEENTITYWITHID simpleexpr tEQ expr	{
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt($THEENTITYWITHID[0]);
		g_lingo->codeInt($THEENTITYWITHID[1]);
		$$ = $expr; }
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
	| tSET THEOBJECTFIELD tTO expr	{
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString($THEOBJECTFIELD.os->c_str());
		g_lingo->codeInt($THEOBJECTFIELD.oe);
		delete $THEOBJECTFIELD.os;
		$$ = $expr; }

stmtoneliner: macro
	| expr
	| proc

stmtonelinerwithif: macro
	| expr
	| proc
	| ifoneliner

stmt: stmtoneliner
	| ifstmt
	// repeat while (expression = TRUE)
	//   statements
	// end repeat
	//
	| repeatwhile expr end[body] stmtlist end[end2] tENDREPEAT	{
		inst body = 0, end = 0;
		WRITE_UINT32(&body, $body - $repeatwhile);
		WRITE_UINT32(&end, $end2 - $repeatwhile);
		(*g_lingo->_currentScript)[$repeatwhile + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[$repeatwhile + 2] = end; }	/* end, if cond fails */

	// repeat with index = start to end
	//   statements
	// end repeat
	//
	| repeatwith tEQ begin[init] expr end tTO expr[finish] end stmtlist end[end3] tENDREPEAT {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, $init - $repeatwith);
		WRITE_UINT32(&finish, $finish - $repeatwith);
		WRITE_UINT32(&body, $stmtlist - $repeatwith);
		WRITE_UINT32(&end, $end3 - $repeatwith);
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[$repeatwith + 1] = init;		/* initial count value */
		(*g_lingo->_currentScript)[$repeatwith + 2] = finish;	/* final count value */
		(*g_lingo->_currentScript)[$repeatwith + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[$repeatwith + 4] = inc;		/* increment */
		(*g_lingo->_currentScript)[$repeatwith + 5] = end; }	/* end, if cond fails */

	// repeat with index = high down to low
	//   statements
	// end repeat
	//
	| repeatwith tEQ begin[init] expr end tDOWN tTO expr[finish] end stmtlist end[end3] tENDREPEAT {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, $init - $repeatwith);
		WRITE_UINT32(&finish, $finish - $repeatwith);
		WRITE_UINT32(&body, $stmtlist - $repeatwith);
		WRITE_UINT32(&end, $end3 - $repeatwith);
		WRITE_UINT32(&inc, (uint32)-1);
		(*g_lingo->_currentScript)[$repeatwith + 1] = init;		/* initial count value */
		(*g_lingo->_currentScript)[$repeatwith + 2] = finish;	/* final count value */
		(*g_lingo->_currentScript)[$repeatwith + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[$repeatwith + 4] = inc;		/* increment */
		(*g_lingo->_currentScript)[$repeatwith + 5] = end; }	/* end, if cond fails */
	| repeatwith tIN begin[list] expr end stmtlist end[end3] tENDREPEAT {
		inst list = 0, body = 0, end = 0;
		WRITE_UINT32(&list, $list - $repeatwith);
		WRITE_UINT32(&body, $stmtlist - $repeatwith);
		WRITE_UINT32(&end, $end3 - $repeatwith);
		(*g_lingo->_currentScript)[$repeatwith + 1] = list;		/* initial count value */
		(*g_lingo->_currentScript)[$repeatwith + 2] = 0;		/* final count value */
		(*g_lingo->_currentScript)[$repeatwith + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[$repeatwith + 4] = 0;		/* increment */
		(*g_lingo->_currentScript)[$repeatwith + 5] = end; }	/* end, if cond fails */

	| tNEXT tREPEAT {
		g_lingo->code1(LC::c_nextRepeat); }
	| when stmtonelinerwithif end {
		inst end = 0;
		WRITE_UINT32(&end, $end - $when);
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[$when + 1] = end; }
	| tTELL expr '\n' tellstart stmtlist end tENDTELL {
		inst end;
		WRITE_UINT32(&end, $end - $tellstart);
		(*g_lingo->_currentScript)[$tellstart + 1] = end; }
	| tTELL expr tTO tellstart stmtoneliner end {
		inst end;
		WRITE_UINT32(&end, $end - $tellstart);
		(*g_lingo->_currentScript)[$tellstart + 1] = end; }

tellstart:	  /* empty */				{
		$$ = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }

ifstmt: if expr end[endexpr] tTHEN stmtlist end[else1] elseifstmtlist end[end3] tENDIF {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, $endexpr - $if);
		WRITE_UINT32(&else1, $else1 - $if);
		WRITE_UINT32(&end, $end3 - $if);
		(*g_lingo->_currentScript)[$if + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[$if + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[$if + 3] = end;	/* end, if cond fails */

		g_lingo->processIf($if, $end3 - $if, 0); }
	| if expr end[endexpr] tTHEN stmtlist end[else1] elseifstmtlist tELSE begin stmtlist end[end3] tENDIF {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, $endexpr - $if);
		WRITE_UINT32(&else1, $else1 - $if);
		WRITE_UINT32(&end, $end3 - $if);
		(*g_lingo->_currentScript)[$if + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[$if + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[$if + 3] = end;	/* end, if cond fails */

		g_lingo->processIf($if, $end3 - $if, $begin - $if); }

elseifstmtlist:	/* nothing */
	| elseifstmtlist elseifstmt

elseifstmt: elseif expr end[endexpr] tTHEN stmtlist end {
		inst then = 0;
		WRITE_UINT32(&then, $endexpr - $elseif);
		(*g_lingo->_currentScript)[$elseif + 1] = then;	/* thenpart */

		g_lingo->codeLabel($elseif); }

ifoneliner: if expr end[endexpr] tTHEN stmtoneliner end[else1] tELSE begin stmtoneliner end[end3] tENDIF {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, $endexpr - $if);
		WRITE_UINT32(&else1, $else1 - $if);
		WRITE_UINT32(&end, $end3 - $if);
		(*g_lingo->_currentScript)[$if + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[$if + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[$if + 3] = end;	/* end, if cond fails */

		g_lingo->processIf($if, $end3 - $if, $begin - $if); }
	| if expr end[endexpr] tTHEN stmtoneliner end[end3] tENDIF {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, $endexpr - $if);
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, $end3 - $if);
		(*g_lingo->_currentScript)[$if + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[$if + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[$if + 3] = end;	/* end, if cond fails */

		g_lingo->processIf($if, $end3 - $if, $end3 - $if); }

repeatwhile:	tREPEAT tWHILE		{ $$ = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }

repeatwith:		tREPEAT tWITH ID	{
		$$ = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString($ID->c_str());
		delete $ID; }

if:	  tIF					{
		$$ = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); } // Mark beginning of the if() statement

elseif:	  tELSIF			{
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		$$ = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }

begin:	  /* nothing */		{ $$ = g_lingo->_currentScript->size(); }

end:	  /* nothing */		{ g_lingo->code1(STOP); $$ = g_lingo->_currentScript->size(); }

stmtlist: 					{ $$ = g_lingo->_currentScript->size(); }
	| stmtlist '\n'
	| stmtlist stmt

when:	  tWHEN	ID tTHEN	{
		$$ = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString($ID->c_str());
		delete $ID; }

simpleexpr: INT		{
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
	| ID		{
		$$ = g_lingo->code1(LC::c_eval);
		g_lingo->codeString($ID->c_str());
		delete $ID; }
	| list

expr: simpleexpr { $$ = $simpleexpr; }
	| reference
	| FBLTIN '(' arglist ')' {
		g_lingo->codeFunc($FBLTIN, $arglist);
		delete $FBLTIN; }
	| FBLTIN arglist	{
		g_lingo->codeFunc($FBLTIN, $arglist);
		delete $FBLTIN; }
	| ID '(' arglist ')'	{
		$$ = g_lingo->codeFunc($ID, $arglist);
		delete $ID; }
	| THEENTITY	{
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
	| THEOBJECTFIELD {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString($THEOBJECTFIELD.os->c_str());
		g_lingo->codeInt($THEOBJECTFIELD.oe);
		delete $THEOBJECTFIELD.os; }
	| THEOBJECTREF {
		g_lingo->code1(LC::c_objectrefpush);
		g_lingo->codeString($THEOBJECTREF.obj->c_str());
		g_lingo->codeString($THEOBJECTREF.field->c_str());
		delete $THEOBJECTREF.obj;
		delete $THEOBJECTREF.field; }
	| asgn
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
	| tNOT expr  %prec UNARY	{ g_lingo->code1(LC::c_not); }
	| expr '&' expr				{ g_lingo->code1(LC::c_ampersand); }
	| expr tCONCAT expr			{ g_lingo->code1(LC::c_concat); }
	| expr tCONTAINS expr		{ g_lingo->code1(LC::c_contains); }
	| expr tSTARTS expr			{ g_lingo->code1(LC::c_starts); }
	| '+' expr[arg]  %prec UNARY{ $$ = $arg; }
	| '-' expr[arg]  %prec UNARY{ $$ = $arg; g_lingo->code1(LC::c_negate); }
	| '(' expr[arg] ')'			{ $$ = $arg; }
	| tSPRITE expr tINTERSECTS expr 	{ g_lingo->code1(LC::c_intersects); }
	| tSPRITE expr tWITHIN expr		 	{ g_lingo->code1(LC::c_within); }

chunkexpr: 	tCHAR expr tOF expr			{ g_lingo->code1(LC::c_charOf); }
	| tCHAR expr tTO expr tOF expr		{ g_lingo->code1(LC::c_charToOf); }
	| tITEM expr tOF expr				{ g_lingo->code1(LC::c_itemOf); }
	| tITEM expr tTO expr tOF expr		{ g_lingo->code1(LC::c_itemToOf); }
	| tLINE expr tOF expr				{ g_lingo->code1(LC::c_lineOf); }
	| tLINE expr tTO expr tOF expr		{ g_lingo->code1(LC::c_lineToOf); }
	| tWORD expr tOF expr				{ g_lingo->code1(LC::c_wordOf); }
	| tWORD expr tTO expr tOF expr		{ g_lingo->code1(LC::c_wordToOf); }

reference: 	RBLTIN simpleexpr	{
		g_lingo->codeFunc($RBLTIN, 1);
		delete $RBLTIN; }
	| chunkexpr

proc: tPUT expr				{ g_lingo->code1(LC::c_printtop); }
	| gotofunc
	| playfunc
	| tEXIT tREPEAT			{ g_lingo->code1(LC::c_exitRepeat); }
	| tEXIT					{ g_lingo->code1(LC::c_procret); }
	| tGLOBAL { g_lingo->_indef = kStateInArgs; } globallist { g_lingo->_indef = kStateNone; }
	| tPROPERTY { g_lingo->_indef = kStateInArgs; } propertylist { g_lingo->_indef = kStateNone; }
	| tINSTANCE instancelist
	| BLTIN '(' arglist ')'			{
		g_lingo->codeFunc($BLTIN, $arglist);
		delete $BLTIN; }
	| BLTIN arglist			{
		g_lingo->codeFunc($BLTIN, $arglist);
		delete $BLTIN; }
	| tOPEN expr tWITH expr	{ g_lingo->code1(LC::c_open); }
	| tOPEN expr 			{ g_lingo->code2(LC::c_voidpush, LC::c_open); }

globallist: ID					{
		g_lingo->code1(LC::c_global);
		g_lingo->codeString($1->c_str());
		delete $ID; }
	| globallist ',' ID			{
		g_lingo->code1(LC::c_global);
		g_lingo->codeString($3->c_str());
		delete $ID; }

propertylist: ID				{
		g_lingo->code1(LC::c_property);
		g_lingo->codeString($1->c_str());
		delete $ID; }
	| propertylist ',' ID		{
		g_lingo->code1(LC::c_property);
		g_lingo->codeString($3->c_str());
		delete $ID; }

instancelist: ID				{
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString($1->c_str());
		delete $ID; }
	| instancelist ',' ID		{
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString($3->c_str());
		delete $ID; }

// go {to} {frame} whichFrame {of movie whichMovie}
// go {to} {frame "Open23" of} movie whichMovie
// go loop
// go next
// go previous
// go to {frame} whichFrame {of movie whichMovie}
// go to {frame whichFrame of} movie whichMovie
gotofunc: tGO tLOOP				{ g_lingo->code1(LC::c_gotoloop); }
	| tGO tNEXT					{ g_lingo->code1(LC::c_gotonext); }
	| tGO tPREVIOUS				{ g_lingo->code1(LC::c_gotoprevious); }
	| tGO expr 			{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
	| tGO expr gotomovie	{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
	| tGO gotomovie				{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }

gotomovie: tOF tMOVIE expr
	| tMOVIE expr

playfunc: tPLAY tDONE			{ g_lingo->code1(LC::c_playdone); }
	| tPLAY expr 			{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
	| tPLAY expr gotomovie	{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
	| tPLAY gotomovie				{
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
	| tPLAYACCEL { g_lingo->codeSetImmediate(true); } arglist	{
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc($tPLAYACCEL, $arglist);
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
defn: tMACRO { g_lingo->_indef = kStateInArgs; } ID { g_lingo->_currentFactory.clear(); }
			begin argdef '\n' argstore stmtlist 		{
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*$ID, $begin, $argdef);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete $ID; }
	| tFACTORY ID	{ g_lingo->codeFactory(*$2); delete $ID; }
	| tMETHOD { g_lingo->_indef = kStateInArgs; }
			begin argdef '\n' argstore stmtlist 		{
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*$tMETHOD, $begin, $argdef + 1, &g_lingo->_currentFactory);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete $tMETHOD; }
	| on begin argdef '\n' argstore stmtlist ENDCLAUSE endargdef {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*$on, $begin, $argdef);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd($ENDCLAUSE, $on->c_str(), false);
		delete $on;
		delete $ENDCLAUSE; }
	| on begin argdef '\n' argstore stmtlist {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*$on, $begin, $argdef);
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete $on; }

on:  tON { g_lingo->_indef = kStateInArgs; } ID { $$ = $ID; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }

argdef:  /* nothing */ 		{ $$ = 0; }
	| ID					{ g_lingo->codeArg($ID); $$ = 1; delete $ID; }
	| argdef ',' ID			{ g_lingo->codeArg($ID); $$ = $1 + 1; delete $ID; }
	| argdef '\n' ',' ID	{ g_lingo->codeArg($ID); $$ = $1 + 1; delete $ID; }

endargdef:	/* nothing */
	| ID					{ delete $ID; }
	| endargdef ',' ID		{ delete $ID; }

argstore:	  /* nothing */		{ g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }

macro: ID nonemptyarglist	{
		g_lingo->code1(LC::c_call);
		g_lingo->codeString($ID->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, $nonemptyarglist);
		g_lingo->code1(numpar);
		delete $ID; }

arglist:  /* nothing */ 	{ $$ = 0; }
	| expr					{ $$ = 1; }
	| arglist ',' expr		{ $$ = $1 + 1; }

nonemptyarglist:  expr			{ $$ = 1; }
	| nonemptyarglist ',' expr	{ $$ = $1 + 1; }

list: '[' valuelist ']'		{ $$ = $valuelist; }

valuelist:	/* nothing */	{ $$ = g_lingo->code2(LC::c_arraypush, 0); }
	| ':'					{ $$ = g_lingo->code2(LC::c_proparraypush, 0); }
	| linearlist { $$ = g_lingo->code1(LC::c_arraypush); $$ = g_lingo->codeInt($linearlist); }
	| proplist	 { $$ = g_lingo->code1(LC::c_proparraypush); $$ = g_lingo->codeInt($proplist); }

linearlist: expr			{ $$ = 1; }
	| linearlist ',' expr	{ $$ = $1 + 1; }

proplist:  proppair			{ $$ = 1; }
	| proplist ',' proppair	{ $$ = $1 + 1; }

proppair: SYMBOL ':' simpleexpr {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString($SYMBOL->c_str());
		delete $SYMBOL; }
	| STRING ':' simpleexpr {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString($STRING->c_str());
		delete $STRING; }


%%
