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

void yyparse();

%}

%%

[_[:alpha:]][_[:alnum:]]*     yylval.s = new Common::String(yytext);  return VAR;
[[:digit:]]+                  yylval.i = strtol(yytext, NULL, 10); return INT;
[-+*/%=^:,]                   return *yytext;
.|\n                          ; /* ignore all the rest */

%%

namespace Director {

int Lingo::parse(char *code) {
	YY_BUFFER_STATE bp;

	yy_delete_buffer(YY_CURRENT_BUFFER);

	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	yyparse();
	yy_delete_buffer(bp);

	return 0;
}

}
