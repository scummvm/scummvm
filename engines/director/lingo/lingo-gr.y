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

%debug

%{
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/hash-str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

extern int yylex();
extern int yyparse();
void yyerror(char *s) { warning("%s", s); }

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
%token tINTO tTO
%token tMCI tMCIWAIT tPUT tSET

%type<code> assign expr

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
	| statement
	| expr  			{ g_lingo->code1(g_lingo->func_printtop); }
	| /* empty */
	;

assign: tPUT expr tINTO VAR		{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($4->c_str()); g_lingo->code1(g_lingo->func_assign); $$ = $2; delete $4; }
	| tSET VAR '=' expr			{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($2->c_str()); g_lingo->code1(g_lingo->func_assign); $$ = $4; delete $2; }
	| tSET VAR tTO expr			{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($2->c_str()); g_lingo->code1(g_lingo->func_assign); $$ = $4; delete $2; }
	;

statement: expr 				{ g_lingo->code1(g_lingo->func_xpop); }
	;

expr: INT						{ g_lingo->code1(g_lingo->func_constpush); inst i; WRITE_LE_UINT32(&i, $1); $$ = g_lingo->code1(i); };
	| VAR						{ g_lingo->code1(g_lingo->func_varpush); g_lingo->codeString($1->c_str()); $$ = g_lingo->code1(g_lingo->func_eval); delete $1; }
	| expr '+' expr				{ g_lingo->code1(g_lingo->func_add); }
	| expr '-' expr				{ g_lingo->code1(g_lingo->func_sub); }
	| expr '*' expr				{ g_lingo->code1(g_lingo->func_mul); }
	| expr '/' expr				{ g_lingo->code1(g_lingo->func_div); }
	| '+' expr  %prec UNARY		{ $$ = $2; }
	| '-' expr  %prec UNARY		{ $$ = $2; g_lingo->code1(g_lingo->func_negate); }
	| '(' expr ')'				{ $$ = $2; }
	;

func: tMCI STRING			{ g_lingo->code1(g_lingo->func_mci); g_lingo->codeString($2->c_str()); delete $2; }
	| tMCIWAIT VAR			{ g_lingo->code1(g_lingo->func_mciwait); g_lingo->codeString($2->c_str()); delete $2; }
	;

%%
