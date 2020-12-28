%{
#include <stdio.h>
#include <string.h>

int yydebug=1;

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

int main()
{
	yyparse();
        return 0;
}

%}

%union {
   char *s;
   int i;
}

%token<s> NAME
%token<s> STRING
%token<i> NUM
%token LTE GTE NEQ EQ IFTOK ELSETOK GOTOTOK DEBUGTOK DEFINETOK SETTINGTOK RANDOMTOK 

%%

lines:   lines '\r' line
       | line
       ;

line:     /* nothing */
        | line DEBUGTOK '{' debug '}'             { printf("debug\n"); }
        | line DEFINETOK NAME '{' define '}'      { printf("define %s\n", $NAME); }
        | line SETTINGTOK NAME '{' statements '}' { printf("setting %s\n", $NAME); }
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
        | NAME ',' fcall ',' define
        | NAME ',' fcall 
        | NAME ',' define
        | NAME
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

value:    NUM { printf("%d", $NUM); }
        | STRING  { printf("%s", $STRING); }
        | NAME { printf("%s", $NAME); }
        ;

expr:     value  
        | '!' value
        | value EQ value
        | value NEQ value
        | value '+' value
        | value '<' value
        | value '>' value
        | value LTE value
        | value GTE value 
        | value '+'
        | RANDOMTOK '(' NUM '%' ')'
        ;
