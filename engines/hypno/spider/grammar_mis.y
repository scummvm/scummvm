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
%defines "engines/hypno/spider/tokens_mis.h"
%output "engines/hypno/spider/grammar_mis.cpp"
%define api.prefix {HYPNO_MIS_}

%{

#include "common/array.h"
#include "hypno/grammar.h"
#include <stdio.h>

#undef yyerror
#define yyerror	 HYPNO_MIS_xerror

extern int HYPNO_MIS_lex();
extern int HYPNO_MIS_parse();
extern int yylineno;

Common::Array<uint32> smenu_idx;
Hypno::HotspotsStack stack;

void HYPNO_MIS_xerror(const char *str) {
	error("ERROR: %s", str);
}

int HYPNO_MIS_wrap() {
    return 1;
}

using namespace Hypno;

%} 

%union {
	char *s; /* string value */
	int i;	 /* integer value */
}

%token<s> NAME FILENAME FLAG COMMENT GSSWITCH COMMAND
%token<i> NUM
%token HOTSTOK CUTSTOK BACKTOK RETTOK  TIMETOK PALETOK BBOXTOK OVERTOK WALNTOK MICETOK PLAYTOK ENDTOK 
%token MENUTOK SMENTOK ESCPTOK NRTOK AMBITOK
%token GLOBTOK TONTOK TOFFTOK
%token TALKTOK INACTOK FDTOK BOXXTOK
%token<s> PG PA PD PH PF PE PP PI

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
	  |  TIMETOK NUM  { debug("TIME %d", $2); } 
      |  BACKTOK FILENAME NUM NUM gsswitch flag {
			Background *a = new Background();
			a->path = $2;
			a->origin = Common::Point($3, $4);
			a->condition = $5;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
			debug("BACK");
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
		|  AMBITOK FILENAME NUM NUM flag { 
			Ambient *a = new Ambient();
			a->path = $2;
			a->origin = Common::Point($3, $4);
			a->flag = $5;
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);			
			debug("AMBI %d %d.", $3, $4); }
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
	  |  PALETOK FILENAME {
			Palette *a = new Palette();
			a->path = $2; 
			Hotspots *cur = stack.back();
		    Hotspot *hot = &cur->back();
			hot->actions.push_back(a);
			debug("PALE");
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
	  |  TALKTOK talk { debug("TALK"); }
	  |  ENDTOK RETTOK { 
		  debug("explicit END");
		  g_parsedHots = stack.back(); 
		  stack.pop_back();
		  smenu_idx.pop_back();
		  }   		               
	  ;

talk:   INACTOK talk { debug("inactive"); }
      | FDTOK talk { debug("inactive"); }
	  | BACKTOK FILENAME NUM NUM gsswitch flag { debug("BACK in TALK"); }
	  | BOXXTOK NUM NUM { debug("BOXX %d %d", $2, $3); }
      | PG talk { debug("%s", $1); }
      | PH talk { debug("%s", $1); }
      | PF talk { debug("%s", $1); }
      | PA talk { debug("%s", $1); } 
      | PD talk { debug("%s", $1); }
      | PP NUM NUM talk { debug("%s %d %d", $1, $2, $3); }
      | PI NUM NUM talk { debug("%s %d %d", $1, $2, $3); }
	  | PE { debug("|E"); }
      | /*nothing*/ { debug("nothing in talk"); }
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