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
%{

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

int yyparse();

%}

variable [_[:alpha:]][_[:alnum:]]*
constfloat [[:digit:]]+\.[[:digit:]]*
constinteger [[:digit:]]+
conststring \"[^\"\n]*\"
operator [-+*/%=^:,()]
newline [\n\r]+
whitespace [\t ]

%%

--[^\r\n]*
^{whitespace}
[\t]+			{ return ' '; }

frame			{ return tFRAME; }
go				{ return tGO; }
into			{ return tINTO; }
loop			{ return tLOOP; }
mci				{ return tMCI; }
mciwait			{ return tMCIWAIT; }
movie			{ return tMOVIE; }
next			{ return tNEXT; }
of				{ return tOF; }
previous		{ return tPREVIOUS; }
put				{ return tPUT; }
set				{ return tSET; }
to				{ return tTO; }

{variable}		{ yylval.s = new Common::String(yytext);  return VAR; }
{constfloat}	{ yylval.f = atof(yytext); return FLOAT; }
{constinteger}	{ yylval.i = strtol(yytext, NULL, 10); return INT; }
{operator}		{ return *yytext; }
{newline}		{ return '\n'; }
{conststring}	{ yylval.s = new Common::String(&yytext[1]); yylval.s->deleteLastChar(); return STRING; }

%%

extern int yydebug;

namespace Director {

int Lingo::parse(const char *code) {
	YY_BUFFER_STATE bp;

	yydebug = 0;

	yy_delete_buffer(YY_CURRENT_BUFFER);

	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	yyparse();
	yy_delete_buffer(bp);

	return 0;
}

}
