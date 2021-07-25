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

%option noyywrap
%option noinput
%option yylineno
%option never-interactive

%option outfile="engines/hypno/lexer.cpp"
%option prefix="HYPNO_"

%{
#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "hypno/hypno.h"
#include "hypno/grammar.h"
#include "hypno/tokens.h"

%}

%%
\;[A-Za-z_0-9 \t\*]*	    /* return COMMENT; */
MENU				    	return MENUTOK;
BACK                    	return BACKTOK;
CUTS                    	return CUTSTOK;
GLOB						return GLOBTOK;
PALE						return PALETOK;
HOTS                        return HOTSTOK;
WAL[0-1]                    return WALNTOK;
MICE                        return MICETOK;
END                         return ENDTOK;
TIME						return TIMETOK;
OVER                        return OVERTOK;
SMEN                        return SMENTOK;
ESCP                        return ESCPTOK;
PLAY						return PLAYTOK;
TURNON						return TONTOK;
TURNOFF						return TOFFTOK;
NO_RETURN                   return NRTOK;
GS_[A-Za-z_0-9]+            HYPNO_lval.s = scumm_strdup(HYPNO_text); return GSSWITCH;
\/BITMAP					return BITMAPTOK;
\/BBOX\=					return BBOXTOK;
\/[A-Za-z_0-9]*  			HYPNO_lval.s = scumm_strdup(HYPNO_text); return FLAG;
[A-Za-z_][A-Za-z_0-9]*  	HYPNO_lval.s = scumm_strdup(HYPNO_text); return NAME;
[A-Za-z][A-Za-z_0-9\\\.]*  	HYPNO_lval.s = scumm_strdup(HYPNO_text); return FILENAME;
[\-]?[0-9]+		        	HYPNO_lval.i = atoi(HYPNO_text); return NUM;
\r\n						return RETTOK;
[ \t]+		            	/* ignore whitespace */;
.				        	debug("<no match>"); return *yytext;
%%

namespace Hypno {

int parse(const char *code) {
	YY_BUFFER_STATE bp;
	yy_delete_buffer(YY_CURRENT_BUFFER);
	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	HYPNO_parse();
	yy_delete_buffer(bp);
	return 0;
}

} // End of namespace Hypno
