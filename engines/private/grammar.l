%option noyywrap
%option noinput
%option never-interactive

%option outfile="engines/private/lex.yy.cpp"

%{
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <stdio.h>
#include <string.h>
#include "grammar.h"
#include "grammar.tab.h"

%}

%%
\/\/.*                  /* ignoring the comment */
\<=                     return LTE;
\>=                     return GTE;
!=                      return NEQ;
==                      return EQ;
debug                   return DEBUGTOK;
define                  return DEFINETOK;
setting                 return SETTINGTOK;
if                      return IFTOK;
else                    return ELSETOK;
goto                    return GOTOTOK;
RECT                    return RECT;
FALSE                   return FALSETOK;
TRUE                    return TRUETOK;
Random                  return RANDOMTOK;
[A-Za-z_][A-Za-z_0-9]*  yylval.s = strdup(yytext); return NAME;
[\-]?[0-9]+             yylval.sym = Private::addconstant(NUM, atoi(yytext), NULL); return NUM;
\"[^\"\r\n]*\"          yylval.sym = Private::addconstant(STRING, 0, strdup(yytext)); return STRING;
[\r|\n]+                /* ignore return */;
[ \t]+                  /* ignore whitespace */;
.                       return *yytext;
%%

extern int yydebug;

namespace Private {

int parse(char *code) {
	
	initSetting();
	//_lines[0] = _lines[1] = _lines[2] = code;

	YY_BUFFER_STATE bp;

	//if (debugChannelSet(-1, kDebugParse))
	yydebug = 1;
	//else
	//	yydebug = 0;

	yy_delete_buffer(YY_CURRENT_BUFFER);

	bp = yy_scan_string(code);
	yy_switch_to_buffer(bp);
	yyparse();
        //execute(prog);
	yy_delete_buffer(bp);

	return 0;
}

} // End of namespace Private
