%require "3.6"
%defines "engines/private/grammar.tab.h"
%output "engines/private/grammar.tab.cpp"

%{
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "private/grammar.h"
#define	code2(c1,c2)	code(c1); code(c2)
#define	code3(c1,c2,c3)	code(c1); code(c2); code(c3)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int yydebug=1;

extern FILE *yyin;
extern int yylex();
extern int yyparse();

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

/*
int parse(char *filename)
{
        yyin = fopen(filename, "r");
        yyparse();
        return 0;
}

int main() {
	initcode();
        yyparse();
        execute(prog);
        return 0;
}*/

typedef int (*Inst)();

%}

%union {
	struct Symbol	*sym;	/* symbol table pointer */
        char *s;
        int *i;
}

%token<s> NAME
%token<sym> STRING NUM
%token LTE GTE NEQ EQ IFTOK ELSETOK GOTOTOK DEBUGTOK DEFINETOK SETTINGTOK RANDOMTOK 

%%

lines:   line lines
       | line
       ;

line:     DEBUGTOK '{' debug '}'             { printf("debug\n"); }
        | DEFINETOK NAME '{' define '}'      { printf("define %s\n", $NAME); }
        | SETTINGTOK NAME '{' statements '}' { printf("setting %s\n", $NAME); }
        ;

debug: /* nothing */
        | NAME ',' debug
        ;

statements:  /* nothing */
        | statements statement


statement: GOTOTOK expr ';' statements 
        | fcall ';' statements
        | IFTOK '(' expr ')' statement statements
        | IFTOK '(' expr ')' '{' statements '}' ELSETOK '{' statements '}' statements  
        | IFTOK '(' expr ')' '{' statements '}' statements
        | IFTOK '(' expr ')' '{' statements '}' ELSETOK statement statements
        | IFTOK '(' expr ')' statement ELSETOK statement statements
        | IFTOK '(' expr ')' statement ELSETOK '{' statements '}'
        ;

define:  /* nothing */
        | NAME ',' fcall ',' define  { }
        | NAME ',' fcall             { }
        | NAME ',' define            { install($NAME, NAME, 0, NULL); }
        | NAME                       { install($NAME, NAME, 0, NULL); }  
        ;

fcall:    GOTOTOK '(' params ')'
        | NAME '(' params ')'
        ;

params:  /* nothing */ 
        | params ',' fcall
        | params ',' expr
        | expr
        | fcall 
        ;

value:    NUM    { code2(constpush, (Inst)$NUM); }
        | STRING { code2(strpush, (Inst)$STRING); }
        | NAME   { code3(varpush, (Inst)lookup($1), eval); }
        ;

expr:     value          
        | '!' value       { code(negate); }
        | value EQ value
        | value NEQ value
        | value '+' value { code(add); }
        | value '<' value
        | value '>' value
        | value LTE value
        | value GTE value 
        | value '+'
        | RANDOMTOK '(' NUM '%' ')'
        ;
