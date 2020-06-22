/* A Bison parser, made by GNU Bison 3.6.3.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    UNARY = 258,                   /* UNARY  */
    CASTREF = 259,                 /* CASTREF  */
    VOID = 260,                    /* VOID  */
    VAR = 261,                     /* VAR  */
    POINT = 262,                   /* POINT  */
    RECT = 263,                    /* RECT  */
    ARRAY = 264,                   /* ARRAY  */
    OBJECT = 265,                  /* OBJECT  */
    FIELDREF = 266,                /* FIELDREF  */
    LEXERROR = 267,                /* LEXERROR  */
    PARRAY = 268,                  /* PARRAY  */
    INT = 269,                     /* INT  */
    ARGC = 270,                    /* ARGC  */
    ARGCNORET = 271,               /* ARGCNORET  */
    THEENTITY = 272,               /* THEENTITY  */
    THEENTITYWITHID = 273,         /* THEENTITYWITHID  */
    THEMENUITEMENTITY = 274,       /* THEMENUITEMENTITY  */
    THEMENUITEMSENTITY = 275,      /* THEMENUITEMSENTITY  */
    FLOAT = 276,                   /* FLOAT  */
    BLTIN = 277,                   /* BLTIN  */
    FBLTIN = 278,                  /* FBLTIN  */
    RBLTIN = 279,                  /* RBLTIN  */
    THEFBLTIN = 280,               /* THEFBLTIN  */
    ID = 281,                      /* ID  */
    STRING = 282,                  /* STRING  */
    HANDLER = 283,                 /* HANDLER  */
    SYMBOL = 284,                  /* SYMBOL  */
    ENDCLAUSE = 285,               /* ENDCLAUSE  */
    tPLAYACCEL = 286,              /* tPLAYACCEL  */
    tMETHOD = 287,                 /* tMETHOD  */
    THEOBJECTPROP = 288,           /* THEOBJECTPROP  */
    tDOWN = 289,                   /* tDOWN  */
    tELSE = 290,                   /* tELSE  */
    tELSIF = 291,                  /* tELSIF  */
    tEXIT = 292,                   /* tEXIT  */
    tGLOBAL = 293,                 /* tGLOBAL  */
    tGO = 294,                     /* tGO  */
    tGOLOOP = 295,                 /* tGOLOOP  */
    tIF = 296,                     /* tIF  */
    tIN = 297,                     /* tIN  */
    tINTO = 298,                   /* tINTO  */
    tMACRO = 299,                  /* tMACRO  */
    tMOVIE = 300,                  /* tMOVIE  */
    tNEXT = 301,                   /* tNEXT  */
    tOF = 302,                     /* tOF  */
    tPREVIOUS = 303,               /* tPREVIOUS  */
    tPUT = 304,                    /* tPUT  */
    tREPEAT = 305,                 /* tREPEAT  */
    tSET = 306,                    /* tSET  */
    tTHEN = 307,                   /* tTHEN  */
    tTO = 308,                     /* tTO  */
    tWHEN = 309,                   /* tWHEN  */
    tWITH = 310,                   /* tWITH  */
    tWHILE = 311,                  /* tWHILE  */
    tFACTORY = 312,                /* tFACTORY  */
    tOPEN = 313,                   /* tOPEN  */
    tPLAY = 314,                   /* tPLAY  */
    tINSTANCE = 315,               /* tINSTANCE  */
    tGE = 316,                     /* tGE  */
    tLE = 317,                     /* tLE  */
    tEQ = 318,                     /* tEQ  */
    tNEQ = 319,                    /* tNEQ  */
    tAND = 320,                    /* tAND  */
    tOR = 321,                     /* tOR  */
    tNOT = 322,                    /* tNOT  */
    tMOD = 323,                    /* tMOD  */
    tAFTER = 324,                  /* tAFTER  */
    tBEFORE = 325,                 /* tBEFORE  */
    tCONCAT = 326,                 /* tCONCAT  */
    tCONTAINS = 327,               /* tCONTAINS  */
    tSTARTS = 328,                 /* tSTARTS  */
    tCHAR = 329,                   /* tCHAR  */
    tITEM = 330,                   /* tITEM  */
    tLINE = 331,                   /* tLINE  */
    tWORD = 332,                   /* tWORD  */
    tSPRITE = 333,                 /* tSPRITE  */
    tINTERSECTS = 334,             /* tINTERSECTS  */
    tWITHIN = 335,                 /* tWITHIN  */
    tTELL = 336,                   /* tTELL  */
    tPROPERTY = 337,               /* tPROPERTY  */
    tON = 338,                     /* tON  */
    tENDIF = 339,                  /* tENDIF  */
    tENDREPEAT = 340,              /* tENDREPEAT  */
    tENDTELL = 341                 /* tENDTELL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 167 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

	struct {
		Common::String *obj;
		Common::String *prop;
	} objectprop;

#line 165 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
