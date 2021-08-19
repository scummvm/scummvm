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

%option outfile="engines/hypno/spider/lexer_arc.cpp"
%option prefix="HYPNO_ARC_"

%{
#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "hypno/hypno.h"
#include "hypno/grammar.h"
#include "hypno/spider/tokens_arc.h"

%}

%%
NONE						return NONETOK;
C				    		return CTOK;
D                           return DTOK;
HE                          return HETOK;
H                           return HTOK;
P							return PTOK;
A							return ATOK;
V							return VTOK;
O 							return OTOK;
N 							return NTOK;
R 							return RTOK;
I 							return ITOK;
Q							return QTOK;
Z							return ZTOK;
W							return WTOK;
X 							return XTOK;
T							return TTOK;
F[0-9]						return FNTOK;
F							return FTOK;
S[0-9]						return SNTOK;
A0							return A0TOK;
B[0-9]						return BNTOK;
K0							return K0TOK;
P0							return P0TOK;
YB							return YBTOK;
YM							return YMTOK;
22[k|K]						return ENCTOK;
[A-Za-z_][A-Za-z_0-9]*  	HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return NAME;
[A-Za-z][A-Za-z_0-9\\\.]*  	HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return FILENAME;
[\-]?[0-9]+		        	HYPNO_ARC_lval.i = atoi(HYPNO_ARC_text); return NUM;
\n							return RETTOK;
\xb3                        return CB3TOK;
\x02                        return C02TOK;
[ \t]+		            	/* ignore whitespace */;
.				        	debug("notparsed: 0x%d",*yytext); return *yytext;
%%

namespace Hypno {

int parse_arc(const char *code) {
	YY_BUFFER_STATE bp;
	yy_delete_buffer(YY_CURRENT_BUFFER);
	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	HYPNO_ARC_parse();
	yy_delete_buffer(bp);
	return 0;
}

} // End of namespace Hypno
