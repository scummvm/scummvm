%{
#include <stdio.h>
#include <string.h>
#include "private.tab.h"
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
Random                  return RANDOMTOK;
[A-Za-z_][A-Za-z_0-9]*  yylval.s=strdup(yytext); return NAME;
[\-]?[0-9]+             yylval.i=atoi(yytext); return NUM;
\"[^\"\r\n]*\"          yylval.s=strdup(yytext); return STRING;
[\r|\n]+                /* ignore return */;
[ \t]+                  /* ignore whitespace */;
.                       return *yytext;
%%
