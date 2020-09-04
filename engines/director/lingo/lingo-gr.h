/* A Bison parser, made by GNU Bison 3.7.1.  */

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
    VOID = 259,                    /* VOID  */
    VAR = 260,                     /* VAR  */
    POINT = 261,                   /* POINT  */
    RECT = 262,                    /* RECT  */
    ARRAY = 263,                   /* ARRAY  */
    OBJECT = 264,                  /* OBJECT  */
    LEXERROR = 265,                /* LEXERROR  */
    PARRAY = 266,                  /* PARRAY  */
    CASTREF = 267,                 /* CASTREF  */
    FIELDREF = 268,                /* FIELDREF  */
    CHUNKREF = 269,                /* CHUNKREF  */
    INT = 270,                     /* INT  */
    ARGC = 271,                    /* ARGC  */
    ARGCNORET = 272,               /* ARGCNORET  */
    THEENTITY = 273,               /* THEENTITY  */
    THEENTITYWITHID = 274,         /* THEENTITYWITHID  */
    THEMENUITEMENTITY = 275,       /* THEMENUITEMENTITY  */
    THEMENUITEMSENTITY = 276,      /* THEMENUITEMSENTITY  */
    FLOAT = 277,                   /* FLOAT  */
    THEFUNC = 278,                 /* THEFUNC  */
    THEFUNCINOF = 279,             /* THEFUNCINOF  */
    VARID = 280,                   /* VARID  */
    STRING = 281,                  /* STRING  */
    SYMBOL = 282,                  /* SYMBOL  */
    ENDCLAUSE = 283,               /* ENDCLAUSE  */
    tPLAYACCEL = 284,              /* tPLAYACCEL  */
    tMETHOD = 285,                 /* tMETHOD  */
    THEOBJECTPROP = 286,           /* THEOBJECTPROP  */
    tCAST = 287,                   /* tCAST  */
    tFIELD = 288,                  /* tFIELD  */
    tSCRIPT = 289,                 /* tSCRIPT  */
    tWINDOW = 290,                 /* tWINDOW  */
    tDOWN = 291,                   /* tDOWN  */
    tELSE = 292,                   /* tELSE  */
    tELSIF = 293,                  /* tELSIF  */
    tEXIT = 294,                   /* tEXIT  */
    tGLOBAL = 295,                 /* tGLOBAL  */
    tGO = 296,                     /* tGO  */
    tGOLOOP = 297,                 /* tGOLOOP  */
    tIF = 298,                     /* tIF  */
    tIN = 299,                     /* tIN  */
    tINTO = 300,                   /* tINTO  */
    tMACRO = 301,                  /* tMACRO  */
    tMOVIE = 302,                  /* tMOVIE  */
    tNEXT = 303,                   /* tNEXT  */
    tOF = 304,                     /* tOF  */
    tPREVIOUS = 305,               /* tPREVIOUS  */
    tPUT = 306,                    /* tPUT  */
    tREPEAT = 307,                 /* tREPEAT  */
    tSET = 308,                    /* tSET  */
    tTHEN = 309,                   /* tTHEN  */
    tTO = 310,                     /* tTO  */
    tWHEN = 311,                   /* tWHEN  */
    tWITH = 312,                   /* tWITH  */
    tWHILE = 313,                  /* tWHILE  */
    tFACTORY = 314,                /* tFACTORY  */
    tOPEN = 315,                   /* tOPEN  */
    tPLAY = 316,                   /* tPLAY  */
    tINSTANCE = 317,               /* tINSTANCE  */
    tGE = 318,                     /* tGE  */
    tLE = 319,                     /* tLE  */
    tEQ = 320,                     /* tEQ  */
    tNEQ = 321,                    /* tNEQ  */
    tAND = 322,                    /* tAND  */
    tOR = 323,                     /* tOR  */
    tNOT = 324,                    /* tNOT  */
    tMOD = 325,                    /* tMOD  */
    tAFTER = 326,                  /* tAFTER  */
    tBEFORE = 327,                 /* tBEFORE  */
    tCONCAT = 328,                 /* tCONCAT  */
    tCONTAINS = 329,               /* tCONTAINS  */
    tSTARTS = 330,                 /* tSTARTS  */
    tCHAR = 331,                   /* tCHAR  */
    tITEM = 332,                   /* tITEM  */
    tLINE = 333,                   /* tLINE  */
    tWORD = 334,                   /* tWORD  */
    tSPRITE = 335,                 /* tSPRITE  */
    tINTERSECTS = 336,             /* tINTERSECTS  */
    tWITHIN = 337,                 /* tWITHIN  */
    tTELL = 338,                   /* tTELL  */
    tPROPERTY = 339,               /* tPROPERTY  */
    tON = 340,                     /* tON  */
    tENDIF = 341,                  /* tENDIF  */
    tENDREPEAT = 342,              /* tENDREPEAT  */
    tENDTELL = 343,                /* tENDTELL  */
    tASSERTERROR = 344             /* tASSERTERROR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 180 "engines/director/lingo/lingo-gr.y"

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

#line 168 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
