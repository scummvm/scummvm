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

%require "3.0"
%defines "engines/hypno/spider/tokens_arc.h"
%output "engines/hypno/spider/grammar_arc.cpp"
%define api.prefix {HYPNO_ARC_}

%{

#include "common/array.h"
#include "hypno/grammar.h"
#include <stdio.h>

#undef yyerror
#define yyerror	 HYPNO_ARC_xerror

Hypno::Shoot *shoot;

extern int HYPNO_ARC_lex();
extern int HYPNO_ARC_parse();
extern int yylineno;

void HYPNO_ARC_xerror(const char *str) {
	debug("ERROR: %s", str);
}

int HYPNO_ARC_wrap() {
    return 1;
}

using namespace Hypno;

%} 

%union {
	char *s; /* string value */
	int i;	 /* integer value */
}

%token<s> NAME FILENAME
%token<i> NUM
// header
%token CTOK DTOK HTOK HETOK RETTOK QTOK ENCTOK
%token PTOK ATOK VTOK OTOK NTOK RTOK ITOK SNTOK ZTOK

// body
%token F0TOK A0TOK B0TOK K0TOK P0TOK WTOK

// end
%token XTOK

// bytes??
%token CB3TOK C02TOK

%%

start: header body
       ;


header:  hline RETTOK header
       | hline
	   | RETTOK header          
	   ; 

hline:  CTOK NUM  { debug("C %d", $2); }
      | DTOK NUM  { debug("D %d", $2); }
      | PTOK NUM NUM { debug("P %d %d", $2, $3); }
      | ATOK NUM NUM { debug("A %d %d", $2, $3); }
      | VTOK NUM NUM { debug("V %d %d", $2, $3); }
      | OTOK NUM NUM { debug("O %d %d", $2, $3); }
	  | NTOK FILENAME  { 
		  g_parsedArc.background = $2; 
		  debug("N %s", $2); 
		}
	  | RTOK FILENAME  { debug("R %s", $2); }
	  | ITOK FILENAME { 
  		  g_parsedArc.player = $2; 
		  debug("I %s", $2); 
		}
	  | QTOK NUM NUM { debug("Q %d %d", $2, $3); }
	  | SNTOK FILENAME enc { debug("SN %s", $2); }
	  | HETOK C02TOK NUM NUM { debug("HE %d %d", $3, $4); }
	  | HTOK CB3TOK NUM NUM { debug("H %d %d", $3, $4); }
	  | ZTOK RETTOK { debug("Z"); }
	  ;

enc: ENCTOK
   | /* nothing */
   ;

body: bline RETTOK body
    | bline RETTOK XTOK

bline: F0TOK FILENAME { 
		shoot = new Shoot();
		shoot->animation = $2;
		debug("F0 %s", $2); 
	 	}
     | ITOK  NAME  { debug("I %s", $2); }
	 | A0TOK NUM NUM { debug("A0 %d %d", $2, $3); }
	 | B0TOK NUM NUM { debug("B0 %d %d", $2, $3); }
	 | K0TOK NUM NUM { debug("K0 %d %d", $2, $3); }
	 | P0TOK NUM NUM { debug("P0 %d %d", $2, $3); }
	 | OTOK NUM NUM { debug("O %d %d", $2, $3); }
	 | CTOK NUM  { debug("C %d", $2); } 
	 | HTOK NUM  { debug("H %d", $2); }
	 | WTOK NUM  { debug("W %d", $2); }
	 | DTOK NUM  { debug("D %d", $2); }
	 | SNTOK FILENAME enc { debug("SN %s", $2); }
	 | ZTOK {
		g_parsedArc.shoots.push_back(*shoot); 
		delete shoot; 
		shoot = nullptr;
	    debug("Z"); 
		}
     ;