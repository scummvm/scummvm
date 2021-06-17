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
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-the.h"

using namespace Director;

int yyparse();
static const char *inputbuffer;
static uint inputlen;

// Push lines in stack
static void pushLine(uint num) {
	LingoCompiler *compiler = g_lingo->_compiler;

	if (num > inputlen)
		return;

	compiler->_lines[2] = compiler->_lines[1];
	compiler->_lines[1] = compiler->_lines[0];
	compiler->_lines[0] = &inputbuffer[num];
}

static void count() {
	LingoCompiler *compiler = g_lingo->_compiler;

	if (debugChannelSet(-1, kDebugParse))
		debug("LEXER: Read '%s' at %d:%d", yytext, compiler->_linenumber, compiler->_colnumber);

	char *p = yytext;

	while (*p) {
		if (*p == '\n' || *p == '\xC2') {
			compiler->_linenumber++;
			compiler->_colnumber = 0;

			if (*p == '\n')
				pushLine(compiler->_bytenumber + 1);
		} else {
			compiler->_colnumber++;
		}
		p++;
		compiler->_bytenumber++;
	}
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

[#]{identifier}	{ count(); yylval.s = new Common::String(yytext + 1); return tSYMBOL; }	// D3, skip '#'

after		{ count(); return tAFTER; }		// D3
and			{ count(); return tAND; }
before		{ count(); return tBEFORE; }	// D3
cast		{ count(); return tCAST; }
char		{ count(); return tCHAR; }		// D3
contains	{ count(); return tCONTAINS; }
down		{ count(); return tDOWN; }
if			{ count(); return tIF; }
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

		yylval.s = new Common::String(ptr);

		return tENDCLAUSE;
	}
^{spc}*factory { count(); return tFACTORY; }
exit		{ count(); return tEXIT; }
field		{ count(); return tFIELD; }
global		{ count(); return tGLOBAL; }
go{spc}+(to{spc}+)?frame	{ count(); return tGO; }
go{spc}+(to)?	{ count(); return tGO; }
go{spc}+loop	{ count(); return tGOLOOP; }
instance	{ count(); return tINSTANCE; }
intersects	{ count(); return tINTERSECTS;}
into		{ count(); return tINTO; }
in			{ count(); return tIN; }
item		{ count(); return tITEM; }
line		{ count(); return tLINE; }
macro		{ count(); return tMACRO;; }
method		{ count(); return tMETHOD; }
mod			{ count(); return tMOD;}
movie		{ count(); return tMOVIE; }
next		{ count(); return tNEXT; }
not			{ count(); return tNOT; }
of			{ count(); return tOF; }
on			{ count(); return tON; }		// D3
open		{ count(); return tOPEN; }
or			{ count(); return tOR; }
play{spc}+frame	{ count(); return tPLAY; }
play		{ count(); return tPLAY; }
playAccel	{ count(); return tPLAYACCEL; }
previous	{ count(); return tPREVIOUS; }
property	{ count(); return tPROPERTY; }	// D4
put			{ count(); return tPUT; }
repeat		{ count(); return tREPEAT; }
script		{ count(); return tSCRIPT; }
set			{ count(); return tSET; }
starts		{ count(); return tSTARTS; }
tell		{ count(); return tTELL; }
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

			return tTHEMENUITEMSENTITY;
		}

		yylval.s = new Common::String(Common::String::format("numberOf%s", field.c_str()));

		if (g_lingo->_builtinFuncs.contains(*yylval.s)) {
			return tTHEFUNCINOF;
		} else {
			warning("LEXER: Unhandled chunk expression '%s'", yylval.s->c_str());
			return tLEXERROR;
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
			return tTHEFUNCINOF;
		} else {
			warning("LEXER: Unhandled chunk expression '%s'", yylval.s->c_str());
			return tLEXERROR;
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
				return tLEXERROR;
			}

			if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity) {
				warning("LEXER: Unsupported field '%s' for entity '%s'", field.c_str(), ptr);
				return tLEXERROR;
			}

			yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
			yylval.e[1] = g_lingo->_theEntityFields[field]->field;

			// the <field> of menuItem <expr>" of menu <expr>
			if (g_lingo->_theEntityFields[field]->entity == kTheMenuItem)
				return tTHEMENUITEMENTITY;

			if (g_lingo->_theEntities[ptr]->hasId)
				return tTHEENTITYWITHID;
			else
				return tTHEENTITY;
		} else {
			if (g_director->getVersion() >= 400) {
				Common::String key = Common::String::format("%d%s", g_lingo->_objectEntityId, field.c_str());

				yylval.objectprop.obj = new Common::String(ptr);
				yylval.objectprop.prop = new Common::String(field);

				return tTHEOBJECTPROP;
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

					return tTHEFUNC;
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
			return tLEXERROR;
		}

		if (g_lingo->_theEntityFields[field]->entity != g_lingo->_theEntities[ptr]->entity) {
			warning("LEXER: Unsupported field '%s' for entity '%s'", field.c_str(), ptr);
			return tLEXERROR;
		}

		yylval.e[0] = g_lingo->_theEntities[ptr]->entity;
		yylval.e[1] = g_lingo->_theEntityFields[field]->field;

		if (g_lingo->_theEntities[ptr]->hasId)
			return tTHEENTITYWITHID;
		else
			return tTHEENTITY;
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
				return tTHEENTITYWITHID;
			else
				return tTHEENTITY;
		}

		if (g_lingo->_builtinFuncs.contains(ptr)) {
			if (g_lingo->_builtinFuncs[ptr].nargs == 1 && g_lingo->_builtinFuncs[ptr].maxArgs == 1) {
				yylval.s = new Common::String(ptr);
				return tTHEFUNC;
			}
		}

		warning("LEXER: Unhandled the entity '%s'", ptr);
	}
then			{ count(); return tTHEN; }
to				{ count(); return tTO; }
scummvmAssertError	{ count(); return tASSERTERROR; }
sprite			{ count(); return tSPRITE; }
with			{ count(); return tWITH; }
within			{ count(); return tWITHIN; }
when			{ count(); return tWHEN; }
while			{ count(); return tWHILE; }
window			{ count(); return tWINDOW; }
word			{ count(); return tWORD; }

[<][>]			{ count(); return tNEQ; }
[>][=]			{ count(); return tGE; }
[<][=]			{ count(); return tLE; }
[&][&]			{ count(); return tCONCAT; }
[=]				{ count(); return tEQ; }

{identifier}		{
		count();
		yylval.s = new Common::String(yytext);

		return tVARID;
	}
{constfloat}	{ count(); yylval.f = atof(yytext); return tFLOAT; }
{constinteger}	{ count(); yylval.i = strtol(yytext, NULL, 10); return tINT; }
{operator}		{ count(); return *yytext; }
{newline}		{ count(); return '\n'; }
{conststring}	{ count(); yylval.s = cleanupString(&yytext[1]); yylval.s->deleteLastChar(); return tSTRING; }
.				{ count(); }

%%

extern int yydebug;

namespace Director {

int LingoCompiler::parse(const char *code) {
	inputbuffer = code;
	_bytenumber = 0;
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
