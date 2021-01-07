%require "3.6"
%defines "engines/private/grammar.tab.h"
%output "engines/private/grammar.tab.cpp"

%{

#include "grammar.h"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#define	code1(c1)	Private::code(c1);
#define	code2(c1,c2)	Private::code(c1); Private::code(c2)
#define	code3(c1,c2,c3)	Private::code(c1); Private::code(c2); Private::code(c3)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int yydebug=1;

using namespace Private;

extern int yylex();
extern int yyparse();

void yyerror(const char *str)
{
	//fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

%}

%union {
	struct Symbol	*sym;	/* symbol table pointer */
        int (**inst)();	/* machine instruction */
        char *s;
        int *i;
        int narg;
}

%token<s> NAME
%token<sym> STRING NUM
%type <inst> body if startp cond end expr statements statement fcall value
%token LTE GTE NEQ EQ FALSETOK TRUETOK IFTOK ELSETOK RECTTOK GOTOTOK DEBUGTOK DEFINETOK SETTINGTOK RANDOMTOK 
%type<narg> params

%%

lines:   line lines
       | line
       ;

line:     DEBUGTOK '{' debug '}'             { /* Not used in the game */ }
        | DEFINETOK NAME '{' define '}'      { installall($NAME); }
        | SETTINGTOK NAME '{' statements '}' { saveSetting($NAME); initSetting(); }
        ;

debug: /* nothing */
        | NAME ',' debug
        ;

statements:  /* nothing */     { $$ = progp; }
        | statement statements


statement: GOTOTOK expr ';' { /*TODO*/ }
        | fcall ';'         { }   
        | if cond body end {
                //$$ = $1;
         	/* else-less if */
		($1)[1] = (Inst)$3;	/* thenpart */
		($1)[3] = (Inst)$4; 
                }	/* end, if cond fails */
        | if cond body end ELSETOK body end { 
                //$$ = $1;
                /* if with else */
		($1)[1] = (Inst)$3;	/* thenpart */
		($1)[2] = (Inst)$6;	/* elsepart */
		($1)[3] = (Inst)$7; 
                }	/* end, if cond fails */
        ;

body:         statement      { $$ = $1; }
        | '{' statements '}' { $$ = $2; }
        ;

end:	  /* nothing */		{ code(STOP); $$ = progp; }
	;

if: IFTOK { $$ = code(ifcode); code1(STOP); code1(STOP); code1(STOP); } //code3(STOP, STOP, STOP); }
        ;

cond: '(' expr ')'	{ code(STOP); $$ = $2; }
        ;

define:  /* nothing */
        | NAME ',' fcall ',' define  { define($NAME); }
        | NAME ',' fcall             { define($NAME); }
        | NAME ',' define            { define($NAME); }
        | NAME                       { define($NAME); }  
        ;

fcall:    GOTOTOK '(' NAME ')' {
                               $$ = progp;
                               code2(strpush, (Private::Inst) Private::addconstant(STRING, 0, $NAME));
                               code2(constpush, (Private::Inst) Private::addconstant(NUM, 1, NULL));
                               code2(strpush, (Private::Inst) Private::addconstant(STRING, 0, "goto")); 
                               code1(funcpush);
                               }

        | RECTTOK '(' NUM ',' NUM ',' NUM ',' NUM ')' { $$ = progp; }
        | NAME '(' startp params ')'  {
                               $$ = $startp;
                               code2(constpush, (Private::Inst) addconstant(NUM, $params, NULL));
                               code2(strpush, (Private::Inst) addconstant(STRING, 0, $NAME)); 
                               code1(funcpush);
                               }
        ;

startp: /*nothing*/ { $$ = progp; }
        ;

params:   /* nothing */     { $$ = 0; }
        | fcall ',' params  { $$ = $3 + 1; }
        | expr ',' params   { $$ = $3 + 1; }
        | expr        { $$ = 1; }
        | fcall       { $$ = 1; }
        ;

value:    FALSETOK { code2(Private::constpush, (Private::Inst) Private::addconstant(NUM, 0, NULL)); }
        | TRUETOK  { code2(Private::constpush, (Private::Inst) Private::addconstant(NUM, 1, NULL)); }
        |  NUM     { code2(Private::constpush, (Private::Inst)$NUM); }
        | STRING   { code2(Private::strpush, (Private::Inst)$STRING); }
        | NAME     { code1(Private::varpush); code1((Private::Inst) lookupName($NAME)); code1(Private::eval); }
        ;

expr:     value           { $$ = $1; } 
        | '!' value       { code1(Private::negate); $$ = $2; }
        | value EQ value
        | value NEQ value
        | value '+' value { code1(Private::add); }
        | value '<' value { code1(Private::lt); }
        | value '>' value { code1(Private::gt); }
        | value LTE value { code1(Private::le); }
        | value GTE value { code1(Private::ge); }
        | value '+' 
        | RANDOMTOK '(' NUM '%' ')'
        ;
