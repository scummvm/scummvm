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
		if (*p == '\n') {
			compiler->_linenumber++;
			compiler->_colnumber = 0;
			pushLine(compiler->_bytenumber + 1);
		} else if (*p == '\xC2' && *(p + 1) == '\xAC') { // continuation
			compiler->_linenumber++;
			compiler->_colnumber = 0;
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
		if (*s == '\xC2' && *(s + 1) == '\xAC') { // continuation
			s += 2;
			*res += ' '; // replace with space
			continue;
		}
		*res += *s;
		s++;
	}

	return res;
}

static void skipWhitespace(const char **ptr) {
	while (true) {
		if (**ptr == ' ' || **ptr == '\t') {
			*ptr += 1;
		} else if (**ptr == '\xC2' && *(*ptr + 1) == '\xAC') { // continuation
			*ptr += 2;
		} else {
			break;
		}
	}
}

static Common::String *readUntilWhitespace(const char **ptr) {
	Common::String *res = new Common::String;

	while (true) {
		if (**ptr == ' ' || **ptr == '\t') {
			break;
		}
		if (**ptr == '\xC2' && *(*ptr + 1) == '\xAC') { // continuation
			break;
		}
		*res += **ptr;
		*ptr += 1;
	}

	return res;
}

static Common::String *readUntilNull(const char **ptr) {
	Common::String *res = new Common::String;

	while (**ptr) {
		if (**ptr == '\xC2' && *(*ptr + 1) == '\xAC') { // continuation
			*ptr += 2;
			*res += ' '; // replace with space
			continue;
		}
		*res += **ptr;
		*ptr += 1;
	}

	return res;
}

%}

identifier [_[:alpha:]][_\.[:alnum:]]*
constfloat [[:digit:]]+\.[[:digit:]]*
constinteger [[:digit:]]+
conststring \"[^\"\r\n]*\"
operator [-+*/%^:,()><&\[\]]
newline (" "|\t|\xC2\xAC)*[\n\r]
spc (" "|\t|\xC2\xAC)
eventname (keyDown|keyUp|mouseDown|mouseUp|timeOut)
unparsedstmt [^\r\n]*

%%

{spc}+		{ count(); }

[#]{identifier}	{ count(); yylval.s = new Common::String(yytext + 1); return tSYMBOL; }	// D3, skip '#'

abbreviated	{ count(); return tABBREVIATED; }
abbrev		{ count(); return tABBREV; }
abbr		{ count(); return tABBR; }
after		{ count(); return tAFTER; }		// D3
and			{ count(); return tAND; }
before		{ count(); return tBEFORE; }	// D3
cast		{ count(); return tCAST; }
char		{ count(); return tCHAR; }		// D3
chars		{ count(); return tCHARS; }
contains	{ count(); return tCONTAINS; }
date		{ count(); return tDATE; }
delete		{ count(); return tDELETE; }
down		{ count(); return tDOWN; }
else		{ count(); return tELSE; }
end({spc}+{identifier})?	{
		count();

		const char *ptr = &yytext[3]; // Skip 'end '
		skipWhitespace(&ptr);

		if (!scumm_stricmp(ptr, "if"))
			return tENDIF;
		else if (!scumm_stricmp(ptr, "repeat"))
			return tENDREPEAT;
		else if (!scumm_stricmp(ptr, "tell"))
			return tENDTELL;

		yylval.s = new Common::String(ptr);

		return tENDCLAUSE;
	}
exit		{ count(); return tEXIT; }
^{spc}*factory { count(); return tFACTORY; }
field		{ count(); return tFIELD; }
frame		{ count(); return tFRAME; }
global		{ count(); return tGLOBAL; }
go{spc}+(to)?	{ count(); return tGO; }
hilite		{ count(); return tHILITE; }
if			{ count(); return tIF; }
instance	{ count(); return tINSTANCE; }
intersects	{ count(); return tINTERSECTS;}
into		{ count(); return tINTO; }
in			{ count(); return tIN; }
item		{ count(); return tITEM; }
items		{ count(); return tITEMS; }
last		{ count(); return tLAST; }
line		{ count(); return tLINE; }
lines		{ count(); return tLINES; }
long		{ count(); return tLONG; }
macro		{ count(); return tMACRO; }
menu		{ count(); return tMENU; }
menuItem	{ count(); return tMENUITEM;}
menuItems	{ count(); return tMENUITEMS; }
method		{ count(); return tMETHOD; }
mod			{ count(); return tMOD;}
movie		{ count(); return tMOVIE; }
next		{ count(); return tNEXT; }
not			{ count(); return tNOT; }
number		{ count(); return tNUMBER; }
of			{ count(); return tOF; }
on			{ count(); return tON; }		// D3
open		{ count(); return tOPEN; }
or			{ count(); return tOR; }
play		{ count(); return tPLAY; }
previous	{ count(); return tPREVIOUS; }
property	{ count(); return tPROPERTY; }	// D4
put			{ count(); return tPUT; }
repeat		{ count(); return tREPEAT; }
script		{ count(); return tSCRIPT; }
scummvmAssertError	{ count(); return tASSERTERROR; }
set			{ count(); return tSET; }
short		{ count(); return tSHORT; }
sound		{ count(); return tSOUND; }
sprite			{ count(); return tSPRITE; }
starts		{ count(); return tSTARTS; }
tell		{ count(); return tTELL; }
the			{ count(); return tTHE; }
then		{ count(); return tTHEN; }
time		{ count(); return tTIME; }
to			{ count(); return tTO; }
when{spc}+{eventname}{spc}+then{spc}+{unparsedstmt}	{
		count();

		const char *ptr =  &yytext[5]; // Skip 'when '
		skipWhitespace(&ptr);

		Common::String *eventName = readUntilWhitespace(&ptr);

		skipWhitespace(&ptr);
		ptr += 5; // Skip 'then '
		skipWhitespace(&ptr);

		Common::String *stmt = readUntilNull(&ptr);

		yylval.w.eventName = eventName;
		yylval.w.stmt = stmt;
		return tWHEN;
	}
while		{ count(); return tWHILE; }
window		{ count(); return tWINDOW; }
with		{ count(); return tWITH; }
within		{ count(); return tWITHIN; }
word		{ count(); return tWORD; }
words		{ count(); return tWORDS; }

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
