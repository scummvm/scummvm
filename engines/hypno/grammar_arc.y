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
%defines "engines/hypno/tokens_arc.h"
%output "engines/hypno/grammar_arc.cpp"
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
	error("ERROR: %s", str);
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

%token<s> NAME FILENAME BNTOK SNTOK
%token<i> NUM
// header
%token YXTOK CTOK DTOK HTOK HETOK RETTOK QTOK ENCTOK
%token PTOK FTOK TTOK TPTOK ATOK VTOK OTOK O1TOK NTOK RTOK ITOK ZTOK

// body
%token FNTOK NONETOK A0TOK K0TOK P0TOK WTOK

// end
%token XTOK

// bytes??
%token CB3TOK C02TOK

%%

start: YXTOK header body
      ;


header:  hline RETTOK header
       | hline
	   | RETTOK header      
	   ; 

hline:  CTOK NUM  { debug("C %d", $2); }
      | FTOK NUM { debug("F %d", $2); }
      | DTOK NUM  { debug("D %d", $2); }
      | PTOK NUM NUM { debug("P %d %d", $2, $3); }
      | ATOK NUM NUM { debug("A %d %d", $2, $3); }
      | VTOK NUM NUM { debug("V %d %d", $2, $3); }
      | OTOK NUM NUM { debug("O %d %d", $2, $3); }
	  | O1TOK NUM NUM { debug("O1 %d %d", $2, $3); }
	  | TPTOK FILENAME NUM FILENAME { 
		  debug("Tp %s %d %s", $2, $3, $4); 
		}
	  | TTOK FILENAME NUM { 
		  debug("T %s %d", $2, $3); 
		}
	  | TTOK NONETOK NUM { 
		  debug("T NONE %d", $3); 
		}
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
	  | BNTOK FILENAME {
		  if (Common::String("B0") == $1)
		  	g_parsedArc.intro = $2;
		  else if(Common::String("B1") == $1 || Common::String("B@") == $1)
		    g_parsedArc.winVideos.push_back($2);
		  else if(Common::String("B3") == $1 || Common::String("B3") == $1)
		    g_parsedArc.defeatVideos.push_back($2);
		 	
		  debug("BN %s", $2); 
		}
	  | SNTOK FILENAME enc {
		  if (Common::String("S0") == $1)
		  	g_parsedArc.music = $2;
		  else if (Common::String("S1") == $1)
		  	g_parsedArc.shootSound = $2;
		   
		  debug("SN %s", $2); 
		}
	  | HETOK C02TOK NUM NUM { debug("HE %d %d", $3, $4); }
	  | HTOK CB3TOK NUM NUM { 
		  g_parsedArc.health = $3;
		  debug("H %d %d", $3, $4); }
	  | ZTOK RETTOK { debug("Z"); }
	  ;

enc: ENCTOK
   | /* nothing */
   ;

body: bline RETTOK body
    | bline RETTOK XTOK

bline: FNTOK FILENAME { 
		shoot = new Shoot();
		shoot->animation = $2;
		debug("FN %s", $2); 
	 	}
	 | FNTOK NONETOK { 
		shoot = new Shoot();
		shoot->animation = "NONE";
		debug("FN NONE"); 
	 	}
	 | FTOK FILENAME { 
		shoot = new Shoot();
		shoot->animation = $2;
		debug("FN %s", $2); 
	 	}
     | ITOK  NAME  { 
		 shoot->name = $2;
		 debug("I %s", $2); 
	   }
     | ITOK  BNTOK  {  // Workaround for NAME == B1
		 shoot->name = $2;
		 debug("I %s", $2); 
	   }
	 | A0TOK NUM NUM { 
		 shoot->position = Common::Point($2, $3);
		 debug("A0 %d %d", $2, $3); 
		}
	 | RTOK NUM NUM  { debug("R %d %d", $2, $3); }
	 | BNTOK NUM NUM { 

		 debug("BN %d %d", $2, $3); }
	 | K0TOK NUM NUM { 
		 shoot->explosionFrame = $3;
		 debug("K0 %d %d", $2, $3);
		}
	 | P0TOK NUM NUM {
		debug("P0 %d %d", $2, $3); 
	   }
	 | OTOK NUM NUM { 
		debug("O %d %d", $2, $3); 
	   }
	 | CTOK NUM  { debug("C %d", $2); } 
	 | HTOK NUM  { debug("H %d", $2); }
	 | WTOK NUM  { debug("W %d", $2); }
	 | DTOK NUM  { 
		 shoot->damage = $2;
		 debug("D %d", $2); 
		}
	 | SNTOK FILENAME enc { 
		  if (Common::String("S1") == $1)
		  	shoot->endSound = $2;
		  else if (Common::String("S2") == $1)
		  	shoot->startSound = $2;
		 
		 debug("SN %s", $2); }
	 | NTOK { debug("N"); }
	 | ZTOK {
		g_parsedArc.shoots.push_back(*shoot); 
		//delete shoot; 
		//shoot = nullptr;
	    debug("Z"); 
		}
     ;