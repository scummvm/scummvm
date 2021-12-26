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

%option noyywrap
%option noinput
%option nounput
%option yylineno
%option never-interactive

%option outfile="engines/hypno/lexer_mis.cpp"
%option prefix="HYPNO_MIS_"

%{
#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "hypno/hypno.h"
#include "hypno/grammar.h"
#include "hypno/tokens_mis.h"

%}

%%
\;.+						/* return COMMENT; */
\/\/.+						/* return COMMENT; */
MENU						return MENUTOK;
AMBI						return AMBITOK;
BACK						return BACKTOK;
CUTS						return CUTSTOK;
GLOB						return GLOBTOK;
PALE						return PALETOK;
HOTS						return HOTSTOK;
MICE						return MICETOK;
END							return ENDTOK;
TIME						return TIMETOK;
OVER						return OVERTOK;
SMEN						return SMENTOK;
ESCP						return ESCPTOK;
PLAY						return PLAYTOK;
TALK						return TALKTOK;
INACTIVE					return INACTOK;
4DBOX						return FDTOK;
BOXX						return BOXXTOK;
MPTR						return MPTRTOK;
ESCAPE						return ESCAPETOK;
SECOND						return SECONDTOK;
INTRO						return INTROTOK;
INTR						return INTRTOK;
SWPT						return SWPTTOK;
DEFAULT						return DEFAULTTOK;
WAL[0-1]					HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return WALNTOK;
\|S[A-Za-z_0-9\\\.]+		HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PS;
\|G[A-Za-z_0-9\\\.]+		HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PG;
\|P[A-Za-z_0-9\\\.]+		HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PP;
\|I[A-Za-z_0-9\\\.]+		HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PI;
\|H[0-9]+					HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PH;
\|A[0-9]+					HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PA;
\|D[0-9]+					HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PD;
\|F[0-9]+					HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return PF;
\|E							return PE;
\|L							return PL;
GS_[A-Z_0-9]+				HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return GSSWITCH;
\/BBOX\=					return BBOXTOK;
\/[A-Za-z_0-9]*				HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return FLAG;
[A-Za-z_][A-Za-z_0-9]*		HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return NAME;
[A-Za-z][A-Za-z_0-9\\\.]*	HYPNO_MIS_lval.s = scumm_strdup(HYPNO_MIS_text); return FILENAME;
[\-]?[0-9]+					HYPNO_MIS_lval.i = atoi(HYPNO_MIS_text); return NUM;
[\n|\r\n]					return RETTOK;
[ \t]+						/* ignore whitespace */;
.							debugC(1, Hypno::kHypnoDebugParser, "<no match: %c>", *yytext); return *yytext;
%%

namespace Hypno {

int parse_mis(const char *code) {
	YY_BUFFER_STATE bp;
	yy_delete_buffer(YY_CURRENT_BUFFER);
	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	HYPNO_MIS_parse();
	yy_delete_buffer(bp);
	return 0;
}

} // End of namespace Hypno
