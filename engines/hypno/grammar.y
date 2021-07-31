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
#include "common/array.h"
#include "hypno/grammar.h"
#include <stdio.h>

#undef yyerror
#define yyerror	 HYPNO_xerror

extern int HYPNO_lex();
extern int HYPNO_parse();
extern int yylineno;

Common::Array<uint32> smenu_idx;
Hypno::HotspotsStack stack;

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

%token<s> NAME FILENAME FLAG COMMENT GSSWITCH COMMAND
%token<i> NUM
%token HOTSTOK CUTSTOK BACKTOK RETTOK  TIMETOK PALETOK BBOXTOK OVERTOK WALNTOK MICETOK PLAYTOK ENDTOK 
%token MENUTOK SMENTOK ESCPTOK NRTOK
%token GLOBTOK TONTOK TOFFTOK

%type<s> gsswitch flag

%%

start: init lines 
       ;

init: { smenu_idx.push_back(-1);
	    stack.push_back(new Hotspots()); }

lines:   line RETTOK lines
       | line
	   | end lines            
	   ; 

end: RETTOK  { debug("implicit END"); }
    ; 

line:    MENUTOK NAME mflag  {
	     Hotspot *hot = new Hotspot(); 
		 hot->type = MakeMenu;
		 hot->stype = $2;
		 hot->smenu = NULL;
		 debug("MENU %d.", hot->type);
		 Hotspots *cur = stack.back();
		 cur->push_back(*hot);

		 // We don't care about menus, only hotspots
     	 int idx = smenu_idx.back();
		 idx++;
		 smenu_idx.pop_back();
		 smenu_idx.push_back(idx);

	  }
      |	 HOTSTOK BBOXTOK NUM NUM NUM NUM  {  
	     Hotspot *hot = new Hotspot(); 
		 hot->type = MakeHotspot;
		 hot->smenu = NULL;
		 hot->rect = Common::Rect($3, $4, $5, $6);
		 debug("HOTS %d.", hot->type);
		 Hotspots *cur = stack.back();
		 cur->push_back(*hot); 
      }
	  |  SMENTOK { 
		  // This should always point to a hotspot
		  int idx = smenu_idx.back();
		  idx++;
		  smenu_idx.pop_back();
		  smenu_idx.push_back(idx);

		  Hotspots *cur = stack.back();
		  Hotspot *hot = &(*cur)[idx];

		  smenu_idx.push_back(-1);
		  hot->smenu = new Hotspots();
	      stack.push_back(hot->smenu);
		  debug("SUBMENU"); 
		}
      |  ESCPTOK  {
			Escape *a = new Escape();
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
		  	debug("ESC SUBMENU"); }
	  |  TIMETOK NUM                               { debug("TIME %d", $2); } 
      |  BACKTOK FILENAME NUM NUM gsswitch flag    {
			Background *a = new Background();
			a->path = $2;
			a->origin = Common::Point($3, $4);
			a->condition = $5;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
		}
      |  GLOBTOK GSSWITCH NAME  { 
		    Global *a = new Global();
			a->variable = $2;
			a->command = $3;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
			debug("GLOB."); 
		}
	  |  PLAYTOK FILENAME NUM NUM gsswitch flag { 
			Play *a = new Play();
			a->path = $2;
			a->origin = Common::Point($3, $4);
			a->condition = $5;
			a->flag = $6;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);		  
		    debug("PLAY %s.", $2); }
      |  OVERTOK FILENAME NUM NUM flag { 
		  	Overlay *a = new Overlay();
			a->path = $2;
			a->origin = Common::Point($3, $4);
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
	   }
	  |  PALETOK FILENAME                          {
			Palette *a = new Palette();
			a->path = $2; 
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
		}
	  |  CUTSTOK FILENAME { 
		  	Cutscene *a = new Cutscene();
			a->path = $2;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);		  
		    debug("CUTS %s.", $2); 
		}
	  |  WALNTOK FILENAME NUM NUM gsswitch flag  { 
			WalN *a = new WalN();
			a->path = $2;
			a->origin = Common::Point($3, $4);
			a->condition = $5;
			a->flag = $6;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);		  
		    debug("WALN %s %d %d.", $2, $3, $4); } 
	  |  MICETOK FILENAME NUM {
		  	Mice *a = new Mice();
			a->path = $2; 
			a->index = $3-1;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
	  }
	  |  ENDTOK RETTOK { 
		  debug("explicit END");
		  g_parsedHots = stack.back(); 
		  stack.pop_back();
		  smenu_idx.pop_back();
		  }   		               
	  ;

mflag:  NRTOK
      | /*nothing*/
	  ;

flag:   FLAG             { $$ = $1; debug("flag: %s", $1); }
      | /* nothing */	 { $$ = scumm_strdup(""); }
	  ;

gsswitch:   GSSWITCH                      { $$ = $1; debug("switch %s", $1); }
          | /* nothing */                 { $$ = scumm_strdup(""); }
	      ;