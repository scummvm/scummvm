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

identifier [_[:alpha:]][_[:alnum:]]*
constfloat [[:digit:]]+\.[[:digit:]]*
constinteger [[:digit:]]+
conststring \"[^\"\n]*\"
operator [-+*/%=^:,()><]
newline [ \t]*[\n\r]+
whitespace [\t ]

%%

--[^\r\n]*
^{whitespace}+
[\t]+				{ return ' '; }

(?i:down)			{ return tDOWN; }
(?i:if)				{ return tIF; }
(?i:[\n\r]+[\t ]+else[\t ]+if)	{ return tNLELSIF; }
(?i:[\n\r]+[\t ]+else)	{ return tNLELSE; }
(?i:else)			{ return tELSE; }
(?i:end)			{ return tEND; }
(?i:exit)			{ return tEXIT; }
(?i:frame)			{ return tFRAME; }
(?i:global)			{ return tGLOBAL; }
(?i:go)				{ return tGO; }
(?i:into)			{ return tINTO; }
(?i:loop)			{ return tLOOP; }
(?i:macro)			{ return tMACRO; }
(?i:mci)			{ return tMCI; }
(?i:mciwait)		{ return tMCIWAIT; }
(?i:movie)			{ return tMOVIE; }
(?i:next)			{ return tNEXT; }
(?i:of)				{ return tOF; }
(?i:previous)		{ return tPREVIOUS; }
(?i:put)			{ return tPUT; }
(?i:repeat)			{ return tREPEAT; }
(?i:set)			{ return tSET; }
(?i:then)			{ return tTHEN; }
(?i:to)				{ return tTO; }
(?i:with)			{ return tWITH; }
(?i:while)			{ return tWHILE; }

[!][=]			{ return tNEQ; }
[>][=]			{ return tGE; }
[<][=]			{ return tLE; }

{identifier}	{
		yylval.s = new Common::String(yytext);

		if (Director::g_lingo->_builtins.contains(yytext))
			return BLTIN;

		return ID;
	}
{constfloat}	{ yylval.f = atof(yytext); return FLOAT; }
{constinteger}	{ yylval.i = strtol(yytext, NULL, 10); return INT; }
{operator}		{ return *yytext; }
{newline}		{ return '\n'; }
{conststring}	{ yylval.s = new Common::String(&yytext[1]); yylval.s->deleteLastChar(); return STRING; }
.

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
