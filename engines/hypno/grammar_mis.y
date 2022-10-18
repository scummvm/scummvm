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

%require "3.0"
%defines "engines/hypno/tokens_mis.h"
%output "engines/hypno/grammar_mis.cpp"
%define api.prefix {HYPNO_MIS_}

%{

#include "common/array.h"
#include "hypno/hypno.h"
//#include <stdio.h>

#undef yyerror
#define yyerror	 HYPNO_MIS_xerror

extern int HYPNO_MIS_lex();
extern int yylineno;

Common::Array<uint32> *smenu_idx = nullptr;
Hypno::HotspotsStack *stack = nullptr;
Hypno::Talk *talk_action = nullptr;

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

%token<s> NAME FILENAME FLAG COMMENT GSSWITCH COMMAND WALNTOK ENCTOK
%token<i> NUM
%token HOTSTOK CUTSTOK BACKTOK INTRTOK RETTOK TIMETOK PALETOK BBOXTOK OVERTOK MICETOK SONDTOK PLAYTOK ENDTOK
%token MENUTOK SMENTOK ESCPTOK NRTOK AMBITOK SWPTTOK MPTRTOK
%token GLOBTOK TONTOK TOFFTOK
%token TALKTOK INACTOK FDTOK BOXXTOK ESCAPETOK SECONDTOK INTROTOK DEFAULTTOK
%token<s> PG PA PD PH PF PE PP PI PL PS

%type<s> gsswitch flag mflag

%%

start: init lines
	;

init: {
	if (smenu_idx)
		delete smenu_idx;
	smenu_idx = new Common::Array<uint32>();
	smenu_idx->push_back(-1);
	if (stack)
		delete stack;
	stack = new Hypno::HotspotsStack();
	stack->push_back(new Hotspots());
}

lines: line lines
	| /* nothing */
	;


line: MENUTOK mflag mflag mflag {
		Hotspot hot(MakeMenu);
		debugC(1, kHypnoDebugParser, "MENU %s %s", $2, $3);
		hot.flags[0] = $2;
		hot.flags[1] = $3;
		hot.flags[2] = $4;

		Hotspots *cur = stack->back();
		cur->push_back(hot);

		// We don't care about menus, only hotspots
		int idx = smenu_idx->back();
		idx++;
		smenu_idx->pop_back();
		smenu_idx->push_back(idx);
	}
	| MENUTOK FILENAME {
		Hotspot hot(MakeMenu);
		debugC(1, kHypnoDebugParser, "MENU %s", $2);
		hot.background = $2;

		Hotspots *cur = stack->back();
		cur->push_back(hot);

		// We don't care about menus, only hotspots
		int idx = smenu_idx->back();
		idx++;
		smenu_idx->pop_back();
		smenu_idx->push_back(idx);
	}
	| HOTSTOK BBOXTOK NUM NUM NUM NUM  {
		Hotspot hot(MakeHotspot, Common::Rect($3, $4, $5, $6));
		debugC(1, kHypnoDebugParser, "HOTS %d.", hot.type);
		Hotspots *cur = stack->back();
		cur->push_back(hot);
	}
	|  SMENTOK {
		// This should always point to a hotspot
		int idx = smenu_idx->back();
		idx++;
		smenu_idx->pop_back();
		smenu_idx->push_back(idx);

		Hotspots *cur = stack->back();
		Hotspot *hot = &(*cur)[idx];

		smenu_idx->push_back(-1);
		hot->smenu = new Hotspots();
		stack->push_back(hot->smenu);
		debugC(1, kHypnoDebugParser, "SUBMENU");
	}
	|  ESCPTOK  {
		Escape *a = new Escape();
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "ESC SUBMENU"); }
	|  TIMETOK NUM  mflag {
		Timer *a = new Timer($2, $3);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "TIME %d %s", $2, $3); }
	|  SWPTTOK NUM {
		SwapPointer *a = new SwapPointer($2);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "SWPT %d", $2); }
	|  BACKTOK FILENAME NUM NUM gsswitch flag flag {
		Background *a = new Background($2, Common::Point($3, $4), $5, $6, $7);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "BACK");
	}
	|  GLOBTOK GSSWITCH NAME  {
		Global *a = new Global($2, $3);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "GLOB");
	}
	|  AMBITOK FILENAME NUM NUM flag {
		Ambient *a = new Ambient($2, Common::Point($3, $4), $5);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "AMBI %d %d", $3, $4); }
	|  PLAYTOK FILENAME NUM NUM gsswitch flag {
		Play *a = new Play($2, Common::Point($3, $4), $5, $6);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "PLAY %s.", $2); }
	|  SONDTOK FILENAME ENCTOK {
		//Play *a = new Play($2, Common::Point($3, $4), $5, $6);
		//Hotspots *cur = stack->back();
		//Hotspot *hot = &cur->back();
		//hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "SOND %s.", $2); }
	|  SONDTOK FILENAME {
		//Play *a = new Play($2, Common::Point($3, $4), $5, $6);
		//Hotspots *cur = stack->back();
		//Hotspot *hot = &cur->back();
		//hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "SOND %s.", $2); }
	|  OVERTOK FILENAME NUM NUM flag {
		Overlay *a = new Overlay($2, Common::Point($3, $4), $5);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
	}
	|  PALETOK FILENAME {
		Palette *a = new Palette($2);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "PALE");
	}
	|  INTRTOK FILENAME NUM NUM {
		Intro *a = new Intro(Common::String("cine/") + $2);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "INTRO %s %d %d", $2, $3, $4);
	}
	|  INTRTOK FILENAME {
		Intro *a = new Intro(Common::String("cine/") + $2);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "INTRO %s", $2);
	}
	|  CUTSTOK FILENAME {
		Cutscene *a = new Cutscene($2);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "CUTS %s", $2);
	}
	|  WALNTOK FILENAME NUM NUM gsswitch flag  {
		WalN *a = new WalN($1, $2, Common::Point($3, $4), $5, $6);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "WALN %s %d %d", $2, $3, $4); }
	|  MICETOK FILENAME NUM {
		Mice *a = new Mice($2, $3-1);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
	}
	|  MPTRTOK FILENAME NUM NUM NUM NUM NUM {
		debugC(1, kHypnoDebugParser, "MPTR %s %d %d %d %d %d", $2, $3, $4, $5, $6, $7);
	}
	|  TALKTOK alloctalk talk {
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(talk_action);
		talk_action = nullptr;
		debugC(1, kHypnoDebugParser, "TALK"); }
	|  ENDTOK anything RETTOK {
		debugC(1, kHypnoDebugParser, "explicit END");
		g_parsedHots = stack->back();
		stack->pop_back();
		smenu_idx->pop_back();
	}
	|	RETTOK { debugC(1, kHypnoDebugParser, "implicit END"); }
	;

anything: NAME anything
	|  // nothing
	;

alloctalk: {
	assert(talk_action == nullptr);
	talk_action = new Talk();
	talk_action->escape = false;
	talk_action->active = true;
}

talk: INACTOK talk {
		talk_action->active = false;
		debugC(1, kHypnoDebugParser, "inactive"); }
	| FDTOK talk { debugC(1, kHypnoDebugParser, "inactive"); }
	| BACKTOK FILENAME NUM NUM gsswitch flag {
		talk_action->background = $2;
		talk_action->backgroundPos = Common::Point($3, $4);
		debugC(1, kHypnoDebugParser, "BACK in TALK"); }
	| BOXXTOK NUM NUM {
		talk_action->boxPos = Common::Point($2, $3);
		debugC(1, kHypnoDebugParser, "BOXX %d %d", $2, $3); }
	| ESCAPETOK {
		talk_action->escape = true;
		debugC(1, kHypnoDebugParser, "ESCAPE"); }
	| SECONDTOK FILENAME NUM NUM flag {
		talk_action->second = $2;
		talk_action->secondPos = Common::Point($3, $4);
		debugC(1, kHypnoDebugParser, "SECOND %s %d %d '%s'", $2, $3, $4, $5); }
	| INTROTOK FILENAME NUM NUM {
		talk_action->intro = $2;
		talk_action->introPos = Common::Point($3, $4);
		debugC(1, kHypnoDebugParser, "INTRO %s %d %d", $2, $3, $4); }
	| DEFAULTTOK FILENAME NUM NUM {
		// Unsure how this is different from second
		talk_action->second = $2;
		talk_action->secondPos = Common::Point($3, $4);
		debugC(1, kHypnoDebugParser, "DEFAULT %s %d %d", $2, $3, $4); }
	| PG talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "G";
		talk_cmd.path = $1+2;
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s", $1); }
	| PH talk { debugC(1, kHypnoDebugParser, "%s", $1); }
	| PF talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "F";
		talk_cmd.num = atoi($1+2)-1;
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s", $1); }
	| PA talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "A";
		talk_cmd.num = atoi($1+2)-1;
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "|A%d", talk_cmd.num); }
	| PD talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "D";
		talk_cmd.num = atoi($1+2)-1;
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s", $1); }
	| PP NUM NUM flag talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "P";
		talk_cmd.path = $1+2;
		talk_cmd.position = Common::Point($2, $3);
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s %d %d '%s'", $1, $2, $3, $4); }
	| PI NUM NUM talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "I";
		talk_cmd.path = $1+2;
		talk_cmd.position = Common::Point($2, $3);
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s %d %d", $1, $2, $3); }
	| PS talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "S";
		talk_cmd.variable = $1+2;
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s", $1); }
	| PL talk {
		TalkCommand talk_cmd;
		talk_cmd.command = "L";
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "|L"); }
	| PE { debugC(1, kHypnoDebugParser, "|E"); }
	| /*nothing*/
	;

mflag: NAME { $$ = $1; }
	| /* nothing */	{ $$ = scumm_strdup(""); }
	;

flag:   FLAG 		{ $$ = $1; debugC(1, kHypnoDebugParser, "flag: %s", $1); }
	| /* nothing */	{ $$ = scumm_strdup(""); }
	;

gsswitch: GSSWITCH 	{ $$ = $1; debugC(1, kHypnoDebugParser, "switch %s", $1); }
	| /* nothing */ { $$ = scumm_strdup(""); }
	;

