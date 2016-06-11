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

%{
#include "common/hash-str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> vars;

extern int yylex();
extern int yyparse();
void yyerror(char *s) { error("%s", s); }

%}

%union { int i; Common::String *s; }

%token UNARY
%token<i> INT
%token<s> VAR
%type<i> expr

%right '='
%left '+' '-'
%left '*' '/' '%'
%right UNARY

%%

list: statement
    | list statement
    ;

statement: expr ','
    | expr ':'          { warning("%d", $1); }
    ;

expr: INT               { $$ = $1; }
    | VAR               { $$ = vars[*$1];      delete $1; }
    | VAR '=' expr      { $$ = vars[*$1] = $3; delete $1; }
    | expr '+' expr     { $$ = $1 + $3; }
    | expr '-' expr     { $$ = $1 - $3; }
    | expr '*' expr     { $$ = $1 * $3; }
    | expr '/' expr     { $$ = $1 / $3; }
    | expr '%' expr     { $$ = $1 % $3; }
    | '+' expr  %prec UNARY    { $$ =  $2; }
    | '-' expr  %prec UNARY    { $$ = -$2; }
    | '(' expr ')'             { $$ =  $2; }
    ;

%%
