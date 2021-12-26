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
%option never-interactive

%option outfile="engines/private/lexer.cpp"
%option prefix="PRIVATE_"

%{
#define YY_NO_UNISTD_H
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "private/private.h"
#include "private/grammar.h"
#include "private/tokens.h"

using namespace Private;
using namespace Gen;
using namespace Settings;

%}

%%
\/\/.*				    /* ignoring the comment */
\<=					    return LTE;
\>=					    return GTE;
!=					    return NEQ;
==					    return EQ;
debug				    return DEBUGTOK;
define				    return DEFINETOK;
setting				    return SETTINGTOK;
if					    return IFTOK;
else				    return ELSETOK;
goto					return GOTOTOK;
RECT					return RECT;
FALSE			        return FALSETOK;
TRUE					return TRUETOK;
NULL					return NULLTOK;
Random			        return RANDOMTOK;
[A-Za-z_][A-Za-z_0-9]*  PRIVATE_lval.s = scumm_strdup(PRIVATE_text); return NAME;
[\-]?[0-9]+		        PRIVATE_lval.sym = g_private->maps.constant(NUM, atoi(PRIVATE_text), NULL); return NUM;
\"[^\"\r\n]*\"	        PRIVATE_lval.sym = g_private->maps.constant(STRING, 0, scumm_strdup(PRIVATE_text)); return STRING;
[\r|\n]+				/* ignore return */;
[ \t]+		            /* ignore whitespace */;
.				        return *yytext;
%%

namespace Private {

int parse(const char *code) {
	g_setts->init();
	YY_BUFFER_STATE bp;
	yy_delete_buffer(YY_CURRENT_BUFFER);
	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	PRIVATE_parse();
	yy_delete_buffer(bp);
	return 0;
}

} // End of namespace Private
