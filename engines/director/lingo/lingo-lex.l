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

using namespace Director;

int yyparse();
static const char *inputbuffer;
static uint inputlen;

// Push lines in stack
static void pushLine(uint num) {
	if (num > inputlen)
		return;

	g_lingo->_lines[2] = g_lingo->_lines[1];
	g_lingo->_lines[1] = g_lingo->_lines[0];
	g_lingo->_lines[0] = &inputbuffer[num];
}

static void count() {
	if (debugChannelSet(-1, kDebugParse))
		debug("LEXER: Read '%s' at %d:%d", yytext, g_lingo->_linenumber, g_lingo->_colnumber);

	char *p = yytext;

	while (*p) {
		if (*p == '\n' || *p == '\xC2') {
			g_lingo->_linenumber++;
			g_lingo->_colnumber = 0;

			if (*p == '\n')
				pushLine(g_lingo->_bytenumber + 1);
		} else {
			g_lingo->_colnumber++;
		}
		p++;
		g_lingo->_bytenumber++;
	}
}

static int checkImmediate(int token) {
	if (g_lingo->_immediateMode || g_lingo->_indef == kStateInArgs ||
			g_lingo->_methodVars->contains(yytext)) {
		yylval.s = new Common::String(yytext);

		return VARID;
	}

	return token;
}

static Common::String *cleanupString(const char *s) {
	Common::String *res = new Common::String;

	while (*s) {
		if (*s == '\xC2') {
			s++;
			*res += ' ';
			continue;
		}
		*res += *s;
		s++;
	}

	return res;
}

static int varCheck(int token, const char *name) {
	if (g_lingo->_indef == kStateInArgs || g_lingo->_methodVars->contains(name)) {
		yylval.s = new Common::String(name);

		return VARID;
	}

	return token;
}

%}

identifier [_[:alpha:]][_[:alnum:]]*
constfloat [[:digit:]]+\.[[:digit:]]*
constinteger [[:digit:]]+
conststring \"[^\"\r\n]*\"
operator [-+*/%^:,()><&\[\]]
newline [ \t\xC2]*[\n\r]
spc [ \t\xC2]

%%

{spc}+		{ count(); }

[#]{identifier}	{ count(); yylval.s = new Common::String(yytext + 1); return SYMBOL; }	// D3, skip '#'

after		{ count(); return varCheck(tAFTER, "after"); }		// D3
and			{ count(); return varCheck(tAND, "and"); }
before		{ count(); return varCheck(tBEFORE, "before"); }	// D3
cast		{ count(); return tCAST; }
char		{ count(); return varCheck(tCHAR, "char"); }		// D3
contains	{ count(); return varCheck(tCONTAINS, "contains"); }
down		{ count(); return varCheck(tDOWN, "down"); }
if			{ count(); return tIF; }
else{spc}+if { count(); return tELSIF; }
else		{ count(); return tELSE; }
end({spc}+{identifier})?	{
		count();

		const char *ptr = &yytext[3]; // Skip 'end '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
			ptr++;

		if (!scumm_stricmp(ptr, "if"))
			return tENDIF;
		else if (!scumm_stricmp(ptr, "repeat"))
			return tENDREPEAT;
		else if (!scumm_stricmp(ptr, "tell"))
			return tENDTELL;

		if (!*ptr) {	// If it is stale "end", check it
			if (varCheck(ENDCLAUSE, "end") == VARID)
				return VARID;
		}

		yylval.s = new Common::String(ptr);

		return ENDCLAUSE;
	}
^{spc}*factory { count(); return tFACTORY; }
exit		{ count(); return varCheck(tEXIT, "exit"); }
field		{ count(); return tFIELD; }
global		{ count(); return varCheck(tGLOBAL, "global"); }
go{spc}+(to{spc}+)?frame	{ count(); return tGO; }
go{spc}+(to)?	{ count(); return tGO; }
go{spc}+loop	{ count(); return tGOLOOP; }
instance	{ count(); return varCheck(tINSTANCE, "instance"); }
intersects	{ count(); return varCheck(tINTERSECTS, "intersects"); }
into		{ count(); return varCheck(tINTO, "into"); }
in			{ count(); return varCheck(tIN, "in"); }
item		{ count(); return varCheck(tITEM, "item"); }
line		{ count(); return varCheck(tLINE, "line"); }
macro		{ count(); return varCheck(tMACRO, "macro"); }
method{spc}+{identifier}		{
		count();

		const char *ptr = &yytext[6]; // Skip 'method '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
			ptr++;

		yylval.s = new Common::String(ptr);

		return tMETHOD;
	}
mod			{ count(); return varCheck(tMOD, "mod"); }
movie		{ count(); return varCheck(tMOVIE, "movie"); }
next		{ count(); return varCheck(tNEXT, "next"); }
not			{ count(); return varCheck(tNOT, "not"); }
of			{ count(); return varCheck(tOF, "of"); }
on			{ count(); return varCheck(tON, "on"); }		// D3
open		{ count(); return varCheck(tOPEN, "open"); }
or			{ count(); return varCheck(tOR, "or"); }
play{spc}+frame	{ count(); return tPLAY; }
play		{ count(); return varCheck(tPLAY, "play"); }
playAccel	{ count(); yylval.s = new Common::String(yytext); return tPLAYACCEL; }
previous	{ count(); return varCheck(tPREVIOUS, "previous"); }
property	{ count(); return varCheck(tPROPERTY, "property"); }	// D4
put			{ count(); return varCheck(tPUT, "put"); }
repeat		{ count(); return checkImmediate(tREPEAT); }
script		{ count(); return tSCRIPT; }
set			{ count(); return varCheck(tSET, "set"); }
starts		{ count(); return varCheck(tSTARTS, "starts"); }
tell		{ count(); return varCheck(tTELL, "tell"); }
the{spc}+number{spc}+of{spc}+[[:alpha:]]+{spc}+(in|of){spc}+ {
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2') ptr++;

		ptr += 6; // skip 'number '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2') ptr++;

		ptr += 3; // Skip 'of '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2') ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t' && *ptr != '\xC2')
			field += *ptr++;

		// the number of menuItems of menu <xx>
		if (!field.compareToIgnoreCase("menuItems")) {
			Common::String key = Common::String::format("%dnumber", kTheMenuItems);

			yylval.e[0] = kTheMenuItems;
			yylval.e[1] = g_lingo->_theEntityFields[key]->field;

			return THEMENUITEMSENTITY;
		}

		yylval.s = new Common::String(Common::String::format("numberOf%s", field.c_str()));

		if (g_lingo->_builtinFuncs.contains(*yylval.s)) {
			return THEFUNCINOF;
		} else {
			warning("LEXER: Unhandled chunk expression '%s'", yylval.s->c_str());
			return LEXERROR;
		}
	}
the{spc}+last{spc}+[[:alpha:]]+{spc}+(in|of){spc}+	{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		ptr += 5; // skip 'last '
		while (*ptr == ' ' || *ptr == '\t') ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t')
			field += *ptr++;

		yylval.s = new Common::String(Common::String::format("last%sOf", field.c_str()));

		if (g_lingo->_builtinFuncs.contains(*yylval.s)) {
			return THEFUNCINOF;
		} else {
			warning("LEXER: Unhandled chunk expression '%s'", yylval.s->c_str());
			return LEXERROR;
		}
	}
the{spc}+[[:alpha:]]+{spc}+of{spc}+{identifier}	{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
			ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t' && *ptr != '\xC2')
			field += *ptr++;

		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
			ptr++;

		ptr += 3; // Skip 'of '

		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
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
			if (g_director->getVersion() >= 400) {
				Common::String key = Common::String::format("%d%s", g_lingo->_objectEntityId, field.c_str());

				yylval.objectprop.obj = new Common::String(ptr);
				yylval.objectprop.prop = new Common::String(field);

				return THEOBJECTPROP;
			} else if (g_lingo->_builtinFuncs.contains(field)) { // Let's hope they don't do it in D4+
				if (g_lingo->_builtinFuncs[field].nargs == 1 && g_lingo->_builtinFuncs[field].maxArgs == 1) {
					yylval.s = new Common::String(field);
					Common::String rest(ptr);

					// It is the dreaded 'the <builtin> of <expr>'
					// with 'expr' starting with VARID, which leads to clash
					//
					// put 'of VARID' back.

					for (int i = rest.size() - 1; i >= 0; i--)
						unput(rest[i]);

					unput(' '); unput('f'); unput('o');

					return THEFUNC;
				}
			}
		}

		warning("LEXER: Unhandled the entity '%s', field '%s'", ptr, field.c_str());
	}
the{spc}+[[:alpha:]]+{spc}+(date|time)		{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
			ptr++;

		Common::String field;
		while (*ptr != ' ' && *ptr != '\t' && *ptr != '\xC2')
			field += *ptr++;

		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
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
the{spc}+[[:alpha:]]+		{
		count();

		const char *ptr = &yytext[4]; // Skip 'the '
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\xC2')
			ptr++;

		if (g_lingo->_theEntities.contains(ptr)) {
			yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
			yylval.e[1] = 0;	// No field

			if (g_lingo->_theEntities[ptr]->hasId)
				return THEENTITYWITHID;
			else
				return THEENTITY;
		}

		if (g_lingo->_builtinFuncs.contains(ptr)) {
			if (g_lingo->_builtinFuncs[ptr].nargs == 1 && g_lingo->_builtinFuncs[ptr].maxArgs == 1) {
				yylval.s = new Common::String(ptr);
				return THEFUNC;
			}
		}

		warning("LEXER: Unhandled the entity '%s'", ptr);
	}
then			{ count(); return tTHEN; }
to				{ count(); return varCheck(tTO, "to"); }
scummvmAssertError	{ count(); return varCheck(tASSERTERROR, "scummvmAssertError"); }
sprite			{ count(); return varCheck(tSPRITE, "sprite"); }
with			{ count(); return varCheck(tWITH, "with"); }
within			{ count(); return varCheck(tWITHIN, "within"); }
when			{ count(); return varCheck(tWHEN, "when"); }
while			{ count(); return varCheck(tWHILE, "while"); }
window			{ count(); return tWINDOW; }
word			{ count(); return varCheck(tWORD, "word"); }

[<][>]			{ count(); return tNEQ; }
[>][=]			{ count(); return tGE; }
[<][=]			{ count(); return tLE; }
[&][&]			{ count(); return tCONCAT; }
[=]				{ count(); return tEQ; }

{identifier}		{
		count();
		yylval.s = new Common::String(yytext);

		return VARID;
	}
{constfloat}	{ count(); yylval.f = atof(yytext); return FLOAT; }
{constinteger}	{ count(); yylval.i = strtol(yytext, NULL, 10); return INT; }
{operator}		{ count(); return *yytext; }
{newline}		{ count(); return '\n'; }
{conststring}	{ count(); yylval.s = cleanupString(&yytext[1]); yylval.s->deleteLastChar(); return STRING; }
.				{ count(); }

%%

extern int yydebug;

namespace Director {

int Lingo::parse(const char *code) {
	inputbuffer = code;
	g_lingo->_bytenumber = 0;
	inputlen = strlen(code);

	_lines[0] = _lines[1] = _lines[2] = code;

	YY_BUFFER_STATE bp;

	if (debugChannelSet(-1, kDebugParse))
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
