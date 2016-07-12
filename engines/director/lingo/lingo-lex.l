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

using namespace Director;

int yyparse();
static void count() {
	g_lingo->_colnumber += strlen(yytext);
}

static void countnl() {
	char *p = yytext;

	while(*p == '\n' || *p == '\r')
		p++;

	g_lingo->_linenumber++;
	g_lingo->_colnumber = strlen(p);
}

%}

identifier [_[:alpha:]][_[:alnum:]]*
constfloat [[:digit:]]+\.[[:digit:]]*
constinteger [[:digit:]]+
conststring \"[^\"\n]*\"
operator [-+*/%=^:,()><&]
newline [ \t]*[\n\r]+
whitespace [\t ]

%%

--[^\r\n]*
^{whitespace}+		{ count(); }
[\t]+				{ count(); return ' '; }

(?i:and)			{ count(); return tAND; }
(?i:contains)		{ count(); return tCONTAINS; }
(?i:down)			{ count(); return tDOWN; }
(?i:if)				{ count(); return tIF; }
(?i:[\n\r]+[\t ]*else[\t ]+if)	{ countnl(); return tNLELSIF; }
(?i:[\n\r]+[\t ]*else)	{ countnl(); return tNLELSE; }
(?i:else)			{ count(); return tELSE; }
(?i:end)			{ count(); return tEND; }
(?i:exit)			{ count(); return tEXIT; }
(?i:frame)			{ count(); return tFRAME; }
(?i:global)			{ count(); return tGLOBAL; }
(?i:go)				{ count(); return tGO; }
(?i:into)			{ count(); return tINTO; }
(?i:loop)			{ count(); return tLOOP; }
(?i:macro)			{ count(); return tMACRO; }
(?i:mci)			{ count(); return tMCI; }
(?i:mciwait)		{ count(); return tMCIWAIT; }
(?i:movie)			{ count(); return tMOVIE; }
(?i:next)			{ count(); return tNEXT; }
(?i:not)			{ count(); return tNOT; }
(?i:of)				{ count(); return tOF; }
(?i:or)				{ count(); return tOR; }
(?i:previous)		{ count(); return tPREVIOUS; }
(?i:put)			{ count(); return tPUT; }
(?i:repeat)			{ count(); return tREPEAT; }
(?i:set)			{ count(); return tSET; }
(?i:starts)			{ count(); return tSTARTS; }
(?i:the[ \t]+[[:alpha:]]+[\t ]+of[\t ]+[[:alpha:]]+)	{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t')
			field += *ptr++;

		if (!g_lingo->_theEntityFields.contains(field)) {
			error("Unhandled the field %s", ptr);
		}

		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		ptr += 3; // Skip 'of '

		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		if (g_lingo->_theEntities.contains(ptr)) {
			if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity)
				error("Unsupported field '%s' for entity '%s'", field.c_str(), ptr);

			yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
			yylval.e[1] = g_lingo->_theEntityFields[field]->field;

			if (g_lingo->_theEntities[ptr]->hasId)
				return THEENTITYWITHID;
			else
				return THEENTITY;
		}

		error("Unhandled the entity %s", ptr);
	}
(?i:the[ \t]+[[:alpha:]]+)		{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		if (g_lingo->_theEntities.contains(ptr)) {
			yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
			yylval.e[1] = 0;	// No field

			if (g_lingo->_theEntities[ptr]->hasId)
				return THEENTITYWITHID;
			else
				return THEENTITY;
		} else if (g_lingo->_theEntityFields.contains(ptr)) {
			error("the field without entity: %s", ptr);
		}

		error("Unhandled the entity %s", ptr);
	}
(?i:then)			{ count(); return tTHEN; }
(?i:to)				{ count(); return tTO; }
(?i:with)			{ count(); return tWITH; }
(?i:while)			{ count(); return tWHILE; }

[<][>]				{ count(); return tNEQ; }
[>][=]				{ count(); return tGE; }
[<][=]				{ count(); return tLE; }
[&][&]				{ count(); return tCONCAT; }

{identifier}	{
		count();
		yylval.s = new Common::String(yytext);

		if (g_lingo->_builtins.contains(yytext))
			return BLTIN;

		return ID;
	}
{constfloat}	{ count(); yylval.f = atof(yytext); return FLOAT; }
{constinteger}	{ count(); yylval.i = strtol(yytext, NULL, 10); return INT; }
{operator}		{ count(); return *yytext; }
{newline}		{ return '\n'; }
{conststring}	{ count(); yylval.s = new Common::String(&yytext[1]); yylval.s->deleteLastChar(); return STRING; }
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
