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

%{
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/hash-str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

extern int yylex();
extern int yyparse();
void yyerror(char *s) { error("%s", s); }

using namespace Director;

%}

%union {
	Common::String *s;
	int	i;
	float f;
	int code;
}

%token UNARY UNDEF
%token<i> INT
%token<f> FLOAT
%token<s> VAR STRING
%token tIF tEND tFRAME tGO tINTO tLOOP tMCI tMCIWAIT tMOVIE tNEXT tOF tPREVIOUS
%token tPUT tSET tTHEN tTO
%token tGE tLE tGT tLT tEQ tNEQ

%type<code> assign cond expr if end stmtlist
%type<s> gotoframe gotomovie

%right '='
%left '+' '-'
%left '*' '/' '%'
%right UNARY

%%

program: programline '\n' program
	| programline
	;

programline:
	| func
	| assign			{ g_lingo->code1(g_lingo->func_xpop); }
	| stmt
	| expr  			{ g_lingo->code1(g_lingo->func_printtop); }
	| error				{ yyerrok; }
	| /* empty */
	;

assign: tPUT expr tINTO VAR		{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($4->c_str()); g_lingo->code1(g_lingo->func_assign); $$ = $2; delete $4; }
	| tSET VAR '=' expr			{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($2->c_str()); g_lingo->code1(g_lingo->func_assign); $$ = $4; delete $2; }
	| tSET VAR tTO expr			{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($2->c_str()); g_lingo->code1(g_lingo->func_assign); $$ = $4; delete $2; }
	;

stmt: expr 				{ g_lingo->code1(g_lingo->func_xpop); }
	| if cond tTHEN stmtlist end tEND tIF {
		inst then, end;
		WRITE_LE_UINT32(&then, $4);
		WRITE_LE_UINT32(&end, $5);
		(*g_lingo->_currentScript)[$1 + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[$1 + 3] = end; }	/* end, if cond fails */
	;

expr: INT						{ g_lingo->code1(g_lingo->func_constpush); inst i; WRITE_LE_UINT32(&i, $1); $$ = g_lingo->code1(i); };
	| VAR						{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($1->c_str()); $$ = g_lingo->code1(g_lingo->func_eval); delete $1; }
	| expr '+' expr				{ g_lingo->code1(g_lingo->func_add); }
	| expr '-' expr				{ g_lingo->code1(g_lingo->func_sub); }
	| expr '*' expr				{ g_lingo->code1(g_lingo->func_mul); }
	| expr '/' expr				{ g_lingo->code1(g_lingo->func_div); }
	| expr '>' expr				{ g_lingo->code1(g_lingo->func_gt); }
	| expr '<' expr				{ g_lingo->code1(g_lingo->func_lt); }
	| expr tNEQ expr			{ g_lingo->code1(g_lingo->func_neq); }
	| expr tGE expr				{ g_lingo->code1(g_lingo->func_ge); }
	| expr tLE expr				{ g_lingo->code1(g_lingo->func_le); }
	| '+' expr  %prec UNARY		{ $$ = $2; }
	| '-' expr  %prec UNARY		{ $$ = $2; g_lingo->code1(g_lingo->func_negate); }
	| '(' expr ')'				{ $$ = $2; }
	;

cond:	   expr 				{ g_lingo->code1(STOP); }
	| expr '=' expr				{ g_lingo->code2(g_lingo->func_eq, STOP); }
	;
if:	  tIF	{ $$ = g_lingo->code1(g_lingo->func_ifcode); g_lingo->code3(STOP,STOP,STOP); }
	;
end:	  /* nothing */		{ g_lingo->code1(STOP); $$ = g_lingo->_currentScript->size(); }
	;
stmtlist: /* nothing */		{ $$ = g_lingo->_currentScript->size(); }
	| stmtlist '\n'
	| stmtlist stmt
	;

func: tMCI STRING			{ g_lingo->code1(g_lingo->func_mci); g_lingo->codeString($2->c_str()); delete $2; }
	| tMCIWAIT VAR			{ g_lingo->code1(g_lingo->func_mciwait); g_lingo->codeString($2->c_str()); delete $2; }
	| tPUT expr				{ g_lingo->code1(g_lingo->func_printtop); }
	| gotofunc
	;

// go {to} {frame} whichFrame {of movie whichMovie}
// go {to} {frame "Open23" of} movie whichMovie
// go loop
// go next
// go previous
// go to {frame} whichFrame {of movie whichMovie}
// go to {frame whichFrame of} movie whichMovie

gotofunc: tGO tLOOP				{ g_lingo->code1(g_lingo->func_gotoloop); }
	| tGO tNEXT					{ g_lingo->code1(g_lingo->func_gotonext); }
	| tGO tPREVIOUS				{ g_lingo->code1(g_lingo->func_gotoprevious); }
	| tGO gotoframe 			{ g_lingo->code1(g_lingo->func_goto); g_lingo->codeString($2->c_str()); g_lingo->codeString(""); delete $2; }
	| tGO gotoframe gotomovie	{ g_lingo->code1(g_lingo->func_goto); g_lingo->codeString($2->c_str()); g_lingo->codeString($3->c_str()); delete $2; delete $3; }
	| tGO gotomovie				{ g_lingo->code1(g_lingo->func_goto); g_lingo->codeString(""); g_lingo->codeString($2->c_str()); delete $2; }
	;

gotoframe: tTO tFRAME STRING	{ $$ = $3; }
	| tFRAME STRING				{ $$ = $2; }
	| tTO STRING				{ $$ = $2; }
	| STRING					{ $$ = $1; }
	;

gotomovie: tOF tMOVIE STRING	{ $$ = $3; }
	| tMOVIE STRING				{ $$ = $2; }
	| tTO tMOVIE STRING			{ $$ = $3; }
	;

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


%%
