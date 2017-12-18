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
%option nounput
%option noinput
%option never-interactive

%{

#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

using namespace Director;

int yyparse();
static void count() {
	if (debugChannelSet(-1, kDebugLingoParse))
		debug("LEXER: Read '%s' at %d:%d", yytext, g_lingo->_linenumber, g_lingo->_colnumber);

	g_lingo->_colnumber += strlen(yytext);
}

static void countnl() {
	char *p = yytext;

	while(*p == '\n' || *p == '\r')
		p++;

	g_lingo->_linenumber++;
	g_lingo->_colnumber = strlen(p);
}

static int checkImmediate(int token) {
	if (g_lingo->_immediateMode) {
		yylval.s = new Common::String(yytext);

		return ID;
	}

	return token;
}

%}

identifier [_[:alpha:]][_[:alnum:]]*
constfloat [[:digit:]]+\.[[:digit:]]*
constinteger [[:digit:]]+
conststring \"[^\"\r\n]*\"
operator [-+*/%=^:,()><&\[\]]
newline [ \t]*[\n\r]
whitespace [\t ]

%%

\xC2[\r\n]	{ g_lingo->_linenumber++; g_lingo->_colnumber = 0; }
--[^\r\n]*
^{whitespace}+	{ count(); }
[\t]+						{ count(); return ' '; }

[#]{identifier}	{ count(); yylval.s = new Common::String(yytext); return SYMBOL; }	// D3

(?i:after)			{ count(); return tAFTER; }		// D3
(?i:and)				{ count(); return tAND; }
(?i:before)			{ count(); return tBEFORE; }	// D3
(?i:char)				{ count(); return tCHAR; }		// D3
(?i:contains)		{ count(); return tCONTAINS; }
(?i:done)				{ count(); return tDONE; }
(?i:down)				{ count(); return tDOWN; }
(?i:if)					{ count(); return tIF; }
(?i:[\n\r]+[\t ]*else[\t ]+if)	{ countnl(); return tNLELSIF; }
(?i:[\n\r]+[\t ]*else)	{ countnl(); return tNLELSE; }
(?i:else)				{ count(); return tELSE; }
(?i:end)([\t ]*{identifier})?	{
		count();

		const char *ptr = &yytext[4]; // Skip 'end '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		yylval.s = new Common::String(ptr);

		return ENDCLAUSE;
	}
(?i:factory)		{ count(); return tFACTORY; }
(?i:exit)				{ count(); return tEXIT; }
(?i:frame)			{ count(); return tFRAME; }
(?i:global)			{ count(); return tGLOBAL; }
(?i:go[\t ]+to)	{ count(); return tGO; }
(?i:go)					{ count(); return tGO; }
(?i:instance)		{ count(); return tINSTANCE; }
(?i:intersects)	{ count(); return tINTERSECTS; }
(?i:into)				{ count(); return tINTO; }
(?i:item)				{ count(); return tITEM; }
(?i:line)				{ count(); return tLINE; }
(?i:loop)				{ count(); return checkImmediate(tLOOP); }
(?i:macro)			{ count(); return tMACRO; }
(?i:method)			{ count(); return tMETHOD; }
(?i:mod)				{ count(); return tMOD; }
(?i:movie)			{ count(); return tMOVIE; }
(?i:next)				{ count(); return tNEXT; }
(?i:not)				{ count(); return tNOT; }
(?i:of)					{ count(); return tOF; }
(?i:on)					{ count(); return tON; }		// D3
(?i:open)				{ count(); return tOPEN; }
(?i:or)					{ count(); return tOR; }
(?i:play)				{ count(); return tPLAY; }
(?i:playAccel)	{ count(); yylval.s = new Common::String(yytext); return tPLAYACCEL; }
(?i:previous)		{ count(); return tPREVIOUS; }
(?i:property)		{ count(); return tPROPERTY; }	// D4
(?i:put)				{ count(); return tPUT; }
(?i:repeat)			{ count(); return checkImmediate(tREPEAT); }
(?i:set)				{ count(); return tSET; }
(?i:starts)			{ count(); return tSTARTS; }
(?i:tell)				{ count(); return tTELL; }
(?i:the[ \t]+last[\t ]+of[\t ]+)	{
		count();

		yylval.e[0] = g_lingo->_theEntities["last"]->entity;
		yylval.e[1] = 0;	// No field

		return THEENTITYWITHID;
	}
(?i:the[ \t]+sqrt[\t ]+of[\t ]+)	{
		count();

		yylval.e[0] = g_lingo->_theEntities["sqrt"]->entity;
		yylval.e[1] = 0;	// No field

		return THEENTITYWITHID;
	}
(?i:the[ \t]+[[:alpha:]]+[\t ]+of[\t ]+[[:alpha:]]+)	{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t')
			field += *ptr++;

		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		ptr += 3; // Skip 'of '

		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		if (g_lingo->_theEntities.contains(ptr)) {
			field = Common::String::format("%d%s", g_lingo->_theEntities[ptr]->entity, field.c_str());

			if (!g_lingo->_theEntityFields.contains(field)) {
				error("Unhandled the field %s", ptr);
			}

			if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity)
				error("Unsupported field '%s' for entity '%s'", field.c_str(), ptr);

			yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
			yylval.e[1] = g_lingo->_theEntityFields[field]->field;

			if (g_lingo->_theEntities[ptr]->hasId)
				return THEENTITYWITHID;
			else
				return THEENTITY;
		}

		warning("Unhandled the entity %s", ptr);
	}
(?i:the[ \t]+[[:alpha:]]+[ \t+](date|time))		{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t')
			field += *ptr++;

		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		field = Common::String::format("%d%s", g_lingo->_theEntities[ptr]->entity, field.c_str());

		if (!g_lingo->_theEntityFields.contains(field)) {
			error("Unhandled the field %s", ptr);
		}

		if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity)
			error("Unsupported field '%s' for entity '%s'", field.c_str(), ptr);

		yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
		yylval.e[1] = g_lingo->_theEntityFields[field]->field;

		if (g_lingo->_theEntities[ptr]->hasId)
			return THEENTITYWITHID;
		else
			return THEENTITY;
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
		}

		warning("Unhandled the entity %s", ptr);
	}
(?i:then)				{ count(); return tTHEN; }
(?i:then[\n\r]+)		{ count(); return tTHENNL; }
(?i:to)					{ count(); return tTO; }
(?i:sprite)			{ count(); return tSPRITE; }
(?i:with)				{ count(); return tWITH; }
(?i:within)			{ count(); return tWITHIN; }
(?i:when)				{ count(); return tWHEN; }
(?i:while)			{ count(); return tWHILE; }
(?i:word)				{ count(); return tWORD; }

[<][>]					{ count(); return tNEQ; }
[>][=]					{ count(); return tGE; }
[<][=]					{ count(); return tLE; }
[&][&]					{ count(); return tCONCAT; }

{identifier}		{
		count();
		yylval.s = new Common::String(yytext);

		if (g_lingo->_ignoreMe && yylval.s->equalsIgnoreCase("me"))
			return ID;

		if (g_lingo->_twoWordBuiltins.contains(yytext))
			return TWOWORDBUILTIN;

		// Special treatment of 'me'. First parameter is method name
		if (!g_lingo->_currentFactory.empty()) {
			if (yylval.s->equalsIgnoreCase("me"))
				return tME;
		}

		if (g_lingo->_builtins.contains(yytext)) {
			int type = g_lingo->_builtins[yytext]->type;
			if ((type == BLTIN || type == FBLTIN || type == RBLTIN) && g_lingo->_builtins[yytext]->parens == false) {
				if (type == RBLTIN) {
					if (g_lingo->_builtins[yytext]->nargs != 1 || g_lingo->_builtins[yytext]->maxArgs != 1)
						error("Incorrectly set RBLTIN %s", yytext);

					return RBLTINONEARG;
				}
				if (g_lingo->_builtins[yytext]->nargs == 0) {
					if (g_lingo->_builtins[yytext]->maxArgs == 0)
						return type == BLTIN ? BLTINNOARGS : FBLTINNOARGS;
					else if (g_lingo->_builtins[yytext]->maxArgs == 1)
						return BLTINNOARGSORONE;
					else
						return type == BLTIN ? BLTINARGLIST : FBLTINARGLIST;
				} else if (g_lingo->_builtins[yytext]->nargs == 1 &&
							g_lingo->_builtins[yytext]->maxArgs == 1) {
					return type == BLTIN ? BLTINONEARG : FBLTINONEARG;
				} else if (g_lingo->_builtins[yytext]->nargs == -1) {
					return type == BLTIN ? BLTINARGLIST : FBLTINARGLIST;
				} else {
					return type == BLTIN ? BLTINARGLIST : FBLTINARGLIST;
				}
			}
		}

		return ID;
	}
{constfloat}		{ count(); yylval.f = atof(yytext); return FLOAT; }
{constinteger}	{ count(); yylval.i = strtol(yytext, NULL, 10); return INT; }
{operator}			{ count(); return *yytext; }
{newline}				{ return '\n'; }
{conststring}		{ count(); yylval.s = new Common::String(&yytext[1]); yylval.s->deleteLastChar(); return STRING; }
.				{ count(); }

%%

extern int yydebug;

namespace Director {

int Lingo::parse(const char *code) {
	YY_BUFFER_STATE bp;

	if (debugChannelSet(-1, kDebugLingoParse))
		yydebug = 1;
	else
		yydebug = 0;

	yy_delete_buffer(YY_CURRENT_BUFFER);

	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	yyparse();
	yy_delete_buffer(bp);

	return 0;
}

}
