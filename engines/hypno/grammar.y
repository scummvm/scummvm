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

%require "3.0"
%defines "engines/hypno/tokens.h"
%output "engines/hypno/grammar.cpp"
%define api.prefix {HYPNO_}

%{

//#include "private/private.h"
#include "hypno/grammar.h"
#include <stdio.h>

#undef yyerror
#define yyerror	 HYPNO_xerror

extern int HYPNO_lex();
extern int HYPNO_parse();
extern int yylineno;

void HYPNO_xerror(const char *str) {
	debug("ERROR: %s", str);
}

int HYPNO_wrap() {
    return 1;
}

using namespace Hypno;

%} 


%union {
	char *s;	     	/* string value */
	int i;	         	/* integer value */
}

%token<s> NAME FILENAME FLAG COMMENT
%token<i> NUM
%token HOTSTOK CUTSTOK BITMAPTOK BACKTOK RETTOK  TIMETOK PALETOK BBOXTOK OVERTOK WALNTOK MICETOK PLAYTOK ENDTOK 
%token MENUTOK SMENTOK ESCPTOK NRTOK
%token GLOBTOK TONTOK TOFFTOK GSSWITCH

%%

lines:   line RETTOK lines
       | line
	   | end lines            
	   ; 

end: RETTOK  { debug("implicit END"); }
    ; 

line:    MENUTOK NAME mflag  {
	     assert(hot == NULL);
	     hot = new Hotspot(); 
		 hot->type = MakeMenu;
		 debug("MENU %d.", hot->type); 
	  }
      |	 HOTSTOK BBOXTOK NUM NUM NUM NUM  { 
         if (hot != NULL)
		    hots.push_back(*hot);

	     hot = new Hotspot(); 
		 hot->type = MakeHotspot;
		 hot->x0 = $3;
		 debug("HOTS %d", hot->x0); 
      }
	  |  SMENTOK { 
		  hot = NULL;
		  debug("SUBMENU"); }
      |  ESCPTOK                                   { debug("ESC SUBMENU"); }
	  |  TIMETOK NUM                               { debug("TIME %d", $2); } 
      |  BACKTOK FILENAME NUM NUM gsswitch flag    {
		  	Background *a = new Background();
		  	a->path = $2;
		  	a->origin = Common::Point($3, $4);
		  	hot->actions.push_back(a);
		}
      |  GLOBTOK gsswitch command                  { debug("GLOB."); }
	  |  PLAYTOK FILENAME NUM NUM gsswitch flag    { debug("PLAY %s.", $2); }
      |  OVERTOK FILENAME NUM NUM flag    		   { debug("OVER %s.", $2); }
	  |  PALETOK FILENAME                          {
			Palette *a = new Palette();
			a->path = $2; 
		    hot->actions.push_back(a);
		}
	  |  CUTSTOK FILENAME                          { debug("CUTS %s.", $2); }
	  |  WALNTOK FILENAME NUM NUM gsswitch flag    { debug("WALN %s %d %d.", $2, $3, $4); } 
	  |  MICETOK FILENAME NUM {
		  	  //Mice a;
			  //a.path = $2; 
			  //a.index = $3;
		      //hot->actions.push_back(a);
	  }
	  |  ENDTOK RETTOK { 
		  debug("explicit END"); 
		  if (hot == NULL)
			error("Invalid END during MIS parsing");
		  hots.push_back(*hot);
		  }   		               
	  ;

mflag:  NRTOK
      | /*nothing*/
	  ;

flag:   BITMAPTOK                         { debug("flag: BITMAP"); }
      | /* nothing */
	  ;

gsswitch:   GSSWITCH                      { debug("flag: GS_SWITCH"); }
          | /* nothing */
	      ;

command:   TONTOK 
         | TOFFTOK
		 ;