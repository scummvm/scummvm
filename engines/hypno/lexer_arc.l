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

%option outfile="engines/hypno/lexer_arc.cpp"
%option prefix="HYPNO_ARC_"

%{
#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_getc
#define YYERROR_VERBOSE

#include "hypno/hypno.h"
#include "hypno/grammar.h"
#include "hypno/tokens_arc.h"

%}

%%
NONE						return NONETOK;
C							return CTOK;
D							return DTOK;
HE							return HETOK;
HL							return HLTOK;
HU							return HUTOK;
H[1-2]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return H12TOK;
H							return HTOK;
P							return PTOK;
AB							return ABTOK;
AL							return ALTOK;
AV							return AVTOK;
A							return ATOK;
V							return VTOK;
O							return OTOK;
O[0-1]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return ONTOK;
L							return LTOK;
N							return NTOK;
NR							return NRTOK;
N\*							return NSTOK;
M							return MTOK;
R							return RTOK;
R0							return R0TOK;
R1							return R1TOK;
I							return ITOK;
I1							return I1TOK;
J0							return J0TOK;
J							return JTOK;
K							return KTOK;
G							return GTOK;
Q							return QTOK;
U							return UTOK;
Z							return ZTOK;
W							return WTOK;
X							return XTOK;
T							return TTOK;
Tp							return TPTOK;
Ts							return TSTOK;
Ta							return TATOK;
F[0-9]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return FNTOK;
F							return FTOK;
S[0-9]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return SNTOK;
S[A-C]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return SNTOK;
A0							return A0TOK;
B[0-9A-F]					HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return BNTOK;
K[0-9]						return KNTOK;
P0							return P0TOK;
Y[A-Z0-9]					HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return YXTOK;
22[k|K]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return ENCTOK;
11[k|K]						HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return ENCTOK;
[\-]?[0-9]+					HYPNO_ARC_lval.i = atoi(HYPNO_ARC_text); return NUM;
[A-Za-z_][A-Za-z_0-9]*[0-9\.]*	HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return NAME;
[A-Za-z][\-A-Za-z_0-9\\\.]+	HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return FILENAME;
[0-9][\-A-Za-z_0-9\\\.]+	HYPNO_ARC_lval.s = scumm_strdup(HYPNO_ARC_text); return FILENAME;
320\,200					return RESTOK;
[\n|\r\n]					return RETTOK;
\;.+\r						/* ignore comment */
[ \t]+						/* ignore whitespace */;
.							HYPNO_ARC_lval.i = HYPNO_ARC_text[0]; return BYTE;
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

