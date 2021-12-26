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
%defines "engines/private/tokens.h"
%output "engines/private/grammar.cpp"
%define api.prefix {PRIVATE_}

%{

#include "private/private.h"
#include "private/grammar.h"

#undef yyerror
#define yyerror	 PRIVATE_xerror

#define code1(c1)       code(c1);
#define code2(c1,c2)    code(c1); code(c2)
#define code3(c1,c2,c3) code(c1); code(c2); code(c3)

using namespace Private;
using namespace Gen;
using namespace Settings;

extern int PRIVATE_lex();
extern int PRIVATE_parse();

void PRIVATE_xerror(const char *str) {
}

int PRIVATE_wrap() {
    return 1;
}


%}

%union {
	Private::Symbol *sym; /* symbol table pointer */
	int (**inst)();       /* machine instruction */
	char *s;	      /* string value */
	int *i;	       /* integer value */
	int narg;	     /* auxiliary value to count function arguments */
}

%token<s> NAME
%token<sym> STRING NUM
%type <inst> body if startp cond end expr statements statement fcall value
%token LTE GTE NEQ EQ FALSETOK TRUETOK NULLTOK IFTOK ELSETOK RECT GOTOTOK DEBUGTOK DEFINETOK SETTINGTOK RANDOMTOK
%type<narg> params

%%

lines:   line lines
       | line
       ;

line:     DEBUGTOK '{' debug '}'	     { /* Not used in the game */ }
	| DEFINETOK NAME '{' define '}'      { g_private->maps.installAll($NAME); }
	| SETTINGTOK NAME '{' statements '}' { g_setts->save($NAME);
					       g_setts->init(); }
	;

debug: /* nothing */
	| NAME ',' debug
	;

statements:  /* nothing */     { $$ = g_vm->_progp; }
	| statement statements


statement: GOTOTOK NAME ';' {
	$$ = g_vm->_progp;
	code2(strpush, (Inst) g_private->maps.constant(STRING, 0, $NAME));
	code2(constpush, (Inst) g_private->maps.constant(NUM, 1, NULL));
	code2(strpush, (Inst) g_private->maps.constant(STRING, 0, "goto"));
	code1(funcpush);
	}
	| fcall ';'	 { $$ = $1; }
	| if cond body end {
		/* else-less if */
		($1)[1] = (Inst)$3;     /* thenpart */
		($1)[3] = (Inst)$4;
		}		       /* end, if cond fails */
	| if cond body end ELSETOK body end {
		/* if with else */
		($1)[1] = (Inst)$3;     /* thenpart */
		($1)[2] = (Inst)$6;     /* elsepart */
		($1)[3] = (Inst)$7;
		}		       /* end, if cond fails */
	;

body:	 statement      { $$ = $1; }
	| '{' statements '}' { $$ = $2; }
	;

end:      /* nothing */      { code1(STOP); $$ = g_vm->_progp; }
	;

if: IFTOK { $$ = code1(ifcode); code3(STOP, STOP, STOP); }
	;

cond: '(' expr ')'      { code1(STOP); $$ = $2; }
	;

define:  /* nothing */
	| NAME ',' RECT '(' NUM ',' NUM ',' NUM ',' NUM ')' ',' define  {
	  Common::Rect *r = new Common::Rect($5->u.val, $7->u.val, $9->u.val, $11->u.val);
	  assert(r->isValidRect());
	  g_private->maps.defineSymbol($NAME, r);
	  }
	| NAME ',' RECT '(' NUM ',' NUM ',' NUM ',' NUM ')' {
	  Common::Rect *r = new Common::Rect($5->u.val, $7->u.val, $9->u.val, $11->u.val);
	  g_private->maps.defineSymbol($NAME, r);
	  }
	| NAME ',' define { g_private->maps.defineSymbol($NAME, NULL); }
	| NAME	    { g_private->maps.defineSymbol($NAME, NULL); }
	;

fcall:    GOTOTOK '(' NAME ')' {
			       $$ = g_vm->_progp;
			       code2(strpush, (Inst) g_private->maps.constant(STRING, 0, $NAME));
			       code2(constpush, (Inst) g_private->maps.constant(NUM, 1, NULL));
			       code2(strpush, (Inst) g_private->maps.constant(STRING, 0, "goto"));
			       code1(funcpush);
			       }

	| RECT '(' NUM ',' NUM ',' NUM ',' NUM ')' { $$ = g_vm->_progp; }
	| NAME '(' startp params ')'  {
			       $$ = $startp;
			       code2(constpush, (Inst) g_private->maps.constant(NUM, $params, NULL));
			       code2(strpush, (Inst) g_private->maps.constant(STRING, 0, $NAME));
			       code1(funcpush);
			       }
	;

startp: /*nothing*/ { $$ = g_vm->_progp; }
	;

params:   /* nothing */     { $$ = 0; }
	| fcall ',' params  { $$ = $3 + 1; }
	| expr ',' params   { $$ = $3 + 1; }
	| expr	{ $$ = 1; }
	| fcall       { $$ = 1; }
	;

value:    NULLTOK  { code2(constpush, (Inst) g_private->maps.constant(NUM, 0, NULL)); }
	| FALSETOK { code2(constpush, (Inst) g_private->maps.constant(NUM, 0, NULL)); }
	| TRUETOK  { code2(constpush, (Inst) g_private->maps.constant(NUM, 1, NULL)); }
	| NUM      { code2(constpush, (Inst)$NUM); }
	| STRING   { code2(strpush, (Inst)$STRING); }
	| NAME     { code1(varpush); code1((Inst) g_private->maps.lookupName($NAME)); code1(eval); }
	;

expr:     value	   { $$ = $1; }
	| '!' value       { code1(negate); $$ = $2; }
	| value EQ value  { code1(eq); }
	| value NEQ value { code1(ne); }
	| value '+' value { code1(add); }
	| value '<' value { code1(lt); }
	| value '>' value { code1(gt); }
	| value LTE value { code1(le); }
	| value GTE value { code1(ge); }
	| value '+'       { $$ = $1; } // unclear what it should do
	| RANDOMTOK '(' NUM '%' ')' { code3(constpush, (Inst)$NUM, randbool); }
	;
