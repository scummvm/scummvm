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
%option case-insensitive

%option outfile="engines/director/lingo/lingo-lex.cpp"

%{

#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/str.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-the.h"

#define register

using namespace Director;

int yyparse();

static void count() {
	if (debugChannelSet(-1, kDebugLingoParse))
		debug("LEXER: Read '%s' at %d:%d", yytext, g_lingo->_linenumber, g_lingo->_colnumber);

	char *p = yytext;

	while (*p && *p != '\n' && *p != '\r') {
		p++;
		g_lingo->_colnumber++;
	}

	while (*p == '\n' || *p == '\r') {
		if (*p == '\n') {
			g_lingo->_linenumber++;
			g_lingo->_colnumber = 0;
		}

		p++;
	}

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
operator [-+*/%^:,()><&\[\]]
newline [ \t]*[\n\r]
whitespace [ \t]

%%

{whitespace}*\xC2[\r\n]	{ count(); }
--[^\r\n]*
^{whitespace}+	{ count(); }
[\t]+			{ count(); return ' '; }

[#]{identifier}	{ count(); yylval.s = new Common::String(yytext); return SYMBOL; }	// D3

after		{ count(); return tAFTER; }		// D3
and			{ count(); return tAND; }
before		{ count(); return tBEFORE; }	// D3
char		{ count(); return tCHAR; }		// D3
contains	{ count(); return tCONTAINS; }
done		{ count(); return tDONE; }
down		{ count(); return tDOWN; }
if			{ count(); return tIF; }
else[ \t]+if { count(); return tELSIF; }
else		{ count(); return tELSE; }
end([ \t]+{identifier})?	{
		count();

		const char *ptr = &yytext[4]; // Skip 'end '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		if (!scumm_stricmp(ptr, "if"))
			return tENDIF;
		else if (!scumm_stricmp(ptr, "repeat"))
			return tENDREPEAT;
		else if (!scumm_stricmp(ptr, "tell"))
			return tENDTELL;

		yylval.s = new Common::String(ptr);

		return ENDCLAUSE;
	}
^[ \t]*factory { count(); return tFACTORY; }
exit		{ count(); return tEXIT; }
global		{ count(); return tGLOBAL; }
go[ \t]+to[ \t]+frame	{ count(); return tGO; }
go[ \t]+to	{ count(); return tGO; }
go			{ count(); return tGO; }
instance	{ count(); return tINSTANCE; }
intersects	{ count(); return tINTERSECTS; }
into		{ count(); return tINTO; }
in			{ count(); return tIN; }
item		{ count(); return tITEM; }
line		{ count(); return tLINE; }
loop		{ count(); return checkImmediate(tLOOP); }
macro		{ count(); return tMACRO; }
method[ \t]*{identifier}		{
		count();

		const char *ptr = &yytext[6]; // Skip 'method '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		yylval.s = new Common::String(ptr);

		return tMETHOD;
	}
mod			{ count(); return tMOD; }
movie		{ count(); return tMOVIE; }
next		{ count(); return tNEXT; }
not			{ count(); return tNOT; }
of			{ count(); return tOF; }
on			{ count(); return tON; }		// D3
open		{ count(); return tOPEN; }
or			{ count(); return tOR; }
play[ \t]+frame	{ count(); return tPLAY; }
play		{ count(); return tPLAY; }
playAccel	{ count(); yylval.s = new Common::String(yytext); return tPLAYACCEL; }
previous	{ count(); return tPREVIOUS; }
property	{ count(); return tPROPERTY; }	// D4
put			{ count(); return tPUT; }
repeat		{ count(); return checkImmediate(tREPEAT); }
set			{ count(); return tSET; }
sound[ \t]+(close|fadeIn|fadeOut|playFile|stop)	{
		count();

		const char *ptr = &yytext[6]; // Skip 'sound '
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		Common::String bltin = Common::String::format("sound-%s", ptr);

		if (g_lingo->_builtins.contains(bltin)) {
			yylval.s = new Common::String(bltin);
			return BLTIN;
		} else {
			warning("LEXER: Unhandled 'sound %s'", ptr);
			return LEXERROR;
		}
	}
starts		{ count(); return tSTARTS; }
tell		{ count(); return tTELL; }
the[ \t]+sqrt[ \t]+of[ \t]+	{
		count();

		yylval.e[0] = g_lingo->_theEntities["sqrt"]->entity;
		yylval.e[1] = 0;	// No field

		return THEENTITYWITHID;
	}
the[ \t]+value[ \t]+of[ \t]+	{
		count();

		yylval.s = new Common::String("value");

		return FBLTIN;
	}
the[ \t]+number[ \t]+of[ \t]+[[:alpha:]]+[ \t](in|of)[ \t]	{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		ptr += 6; // skip 'number '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		ptr += 3; // Skip 'of '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t')
			field += *ptr++;

		// the number of menuItems of menu <xx>
		if (!field.compareToIgnoreCase("menuItems")) {
			Common::String key = Common::String::format("%dnumber", kTheMenuItems);

			yylval.e[0] = kTheMenuItems;
			yylval.e[1] = g_lingo->_theEntityFields[key]->field;

			return THEMENUITEMSENTITY;
		}

		yylval.s = new Common::String(Common::String::format("numberOf%s", field.c_str()));

		if (g_lingo->_builtins.contains(*yylval.s)) {
			return FBLTIN;
		} else {
			warning("LEXER: Unhandled chunk expression '%s'", yylval.s->c_str());
			return LEXERROR;
		}
	}
the[ \t]+last[ \t][[:alpha:]]+[ \t](in|of)[ \t]	{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		ptr += 5; // skip 'last '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t')
			field += *ptr++;

		yylval.s = new Common::String(Common::String::format("last%sOf", field.c_str()));

		if (g_lingo->_builtins.contains(*yylval.s)) {
			return FBLTIN;
		} else {
			warning("LEXER: Unhandled chunk expression '%s'", yylval.s->c_str());
			return LEXERROR;
		}
	}
the[ \t]+[[:alpha:]]+[ \t]+of[ \t]+{identifier}	{
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
				warning("LEXER: Unhandled the field %s of %s", field.c_str(), ptr);
				return LEXERROR;
			}

			if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity) {
				warning("LEXER: Unsupported field '%s' for entity '%s'", field.c_str(), ptr);
				return LEXERROR;
			}

			yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
			yylval.e[1] = g_lingo->_theEntityFields[field]->field;

			// the <field> of menuItem <expr>" of menu <expr>
			if (g_lingo->_theEntityFields[field]->entity == kTheMenuItem)
				return THEMENUITEMENTITY;

			if (g_lingo->_theEntities[ptr]->hasId)
				return THEENTITYWITHID;
			else
				return THEENTITY;
		} else {
			if (g_director->getVersion() >= 4) {
				Common::String key = Common::String::format("%d%s", g_lingo->_objectEntityId, field.c_str());

				if (!g_lingo->_theEntityFields.contains(key)) { // Okay, this must be an object reference
					yylval.objectref.obj = new Common::String(ptr);
					yylval.objectref.field = new Common::String(field);

					return THEOBJECTREF;
				}

				yylval.objectfield.os = new Common::String(ptr);
				yylval.objectfield.oe = g_lingo->_theEntityFields[key]->field;

				return THEOBJECTFIELD;
			}
		}

		warning("LEXER: Unhandled the entity '%s', field '%s'", ptr, field.c_str());
	}
the[ \t]+[[:alpha:]]+[ \t+](date|time)		{
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
			warning("LEXER: Unhandled the field %s", ptr);
			return LEXERROR;
		}

		if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity) {
			warning("LEXER: Unsupported field '%s' for entity '%s'", field.c_str(), ptr);
			return LEXERROR;
		}

		yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
		yylval.e[1] = g_lingo->_theEntityFields[field]->field;

		if (g_lingo->_theEntities[ptr]->hasId)
			return THEENTITYWITHID;
		else
			return THEENTITY;
	}
the[ \t]+[[:alpha:]]+		{
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

		warning("LEXER: Unhandled the entity '%s'", ptr);
	}
then			{ count(); return tTHEN; }
to				{ count(); return tTO; }
sprite			{ count(); return tSPRITE; }
with			{ count(); return tWITH; }
within			{ count(); return tWITHIN; }
when			{ count(); return tWHEN; }
while			{ count(); return tWHILE; }
word			{ count(); return tWORD; }

[<][>]				{ count(); return tNEQ; }
[>][=]				{ count(); return tGE; }
[<][=]				{ count(); return tLE; }
[&][&]				{ count(); return tCONCAT; }
[=]					{ count(); return tEQ; }

{identifier}		{
		count();
		yylval.s = new Common::String(yytext);

		if (g_lingo->_ignoreMe && yylval.s->equalsIgnoreCase("me"))
			return ID;

		// When we are defining arguments, allow any string
		if (g_lingo->_indef == kStateInArgs)
			return ID;

		// First of all, match against argument list
		if (g_lingo->_indef == kStateInDef)
			if (g_lingo->isInArgStack(yylval.s))
				return ID;

		if (g_lingo->_builtins.contains(yytext)) {
			int type = g_lingo->_builtins[yytext]->type;
			if ((type == BLTIN || type == FBLTIN || type == RBLTIN) && g_lingo->_builtins[yytext]->parens == false) {
				if (type == RBLTIN) {
					if (g_lingo->_builtins[yytext]->nargs != 1 || g_lingo->_builtins[yytext]->maxArgs != 1) {
						warning("Incorrectly set RBLTIN %s", yytext);
						return LEXERROR;
					}

					return RBLTIN;
				}

				return type;
			}
		}

		return ID;
	}
{constfloat}	{ count(); yylval.f = atof(yytext); return FLOAT; }
{constinteger}	{ count(); yylval.i = strtol(yytext, NULL, 10); return INT; }
{operator}		{ count(); return *yytext; }
{newline}		{ count(); return '\n'; }
{conststring}	{ count(); yylval.s = new Common::String(&yytext[1]); yylval.s->deleteLastChar(); return STRING; }
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

} // End of namespace Director
