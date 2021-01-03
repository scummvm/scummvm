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
        int (*inst)();	/* machine instruction */
        char *s;
        int *i;
        int narg;
}

%token<s> NAME
%token<sym> STRING NUM
//%type <inst> value cond expr if
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

statements:  /* nothing */
        | statements statement


statement: GOTOTOK expr ';'
        | fcall ';'        
        | if cond statement end
        | if cond body end ELSETOK body end
        | if cond body end
        | if cond body end ELSETOK statement end
	| if cond statement end ELSETOK statement end
        | if cond statement end ELSETOK body end
        ;

body: '{' statements '}'
        ;

end:	  /* nothing */		{ code(STOP);}
	;

if: IFTOK { code(ifcode); code3(STOP, STOP, STOP); /*code(fail);*/ }
        ;

cond: '(' expr ')'	{ code(STOP); }
        ;

define:  /* nothing */
        | NAME ',' fcall ',' define  { define($NAME); }
        | NAME ',' fcall             { define($NAME); }
        | NAME ',' define            { define($NAME); }
        | NAME                       { define($NAME); }  
        ;

fcall:    GOTOTOK '(' NAME ')' {
                               code2(Private::strpush, (Private::Inst) Private::addconstant(STRING, 0, $NAME));
                               code2(Private::constpush, (Private::Inst) Private::addconstant(NUM, 1, NULL));
                               code2(Private::strpush, (Private::Inst) Private::addconstant(STRING, 0, "goto")); 
                               code1(Private::funcpush); 
                               }

        | RECTTOK '(' NUM ',' NUM ',' NUM ',' NUM ')'
        | NAME '(' params ')'  {
                               code2(Private::constpush, (Private::Inst) Private::addconstant(NUM, $params, NULL));
                               code2(Private::strpush, (Private::Inst) Private::addconstant(STRING, 0, $NAME)); 
                               code1(Private::funcpush); 
                               }
        ;

params:  /* nothing */      { $$ = 0; }
        | fcall ',' params  { $$ = $3 + 1; }
        | expr ',' params   { $$ = $3 + 1; }
        | expr              { $$ = 1; }
        | fcall             { $$ = 1; }
        ;

value:    FALSETOK { code2(Private::constpush, (Private::Inst) Private::addconstant(NUM, 0, NULL)); }
        | TRUETOK  { code2(Private::constpush, (Private::Inst) Private::addconstant(NUM, 1, NULL)); }
        |  NUM     { code2(Private::constpush, (Private::Inst)$NUM); }
        | STRING   { code2(Private::strpush, (Private::Inst)$STRING); }
        | NAME     { code1(Private::varpush); code1((Private::Inst) lookupName($NAME)); code1(Private::eval); }
        ;

expr:     value           
        | '!' value       { code1(Private::negate); }
        | value EQ value
        | value NEQ value
        | value '+' value { code1(Private::add); }
        | value '<' value { code1(Private::lt); }
        | value '>' value { code1(Private::gt); }
        | value LTE value
        | value GTE value 
        | value '+'
        | RANDOMTOK '(' NUM '%' ')'
        ;
