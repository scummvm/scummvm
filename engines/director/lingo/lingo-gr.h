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
    CASTNAME = 267,                /* CASTNAME  */
    CASTNUM = 268,                 /* CASTNUM  */
    FIELDNAME = 269,               /* FIELDNAME  */
    FIELDNUM = 270,                /* FIELDNUM  */
    INT = 271,                     /* INT  */
    ARGC = 272,                    /* ARGC  */
    ARGCNORET = 273,               /* ARGCNORET  */
    THEENTITY = 274,               /* THEENTITY  */
    THEENTITYWITHID = 275,         /* THEENTITYWITHID  */
    THEMENUITEMENTITY = 276,       /* THEMENUITEMENTITY  */
    THEMENUITEMSENTITY = 277,      /* THEMENUITEMSENTITY  */
    FLOAT = 278,                   /* FLOAT  */
    THEFUNC = 279,                 /* THEFUNC  */
    THEFUNCINOF = 280,             /* THEFUNCINOF  */
    VARID = 281,                   /* VARID  */
    STRING = 282,                  /* STRING  */
    SYMBOL = 283,                  /* SYMBOL  */
    ENDCLAUSE = 284,               /* ENDCLAUSE  */
    tPLAYACCEL = 285,              /* tPLAYACCEL  */
    tMETHOD = 286,                 /* tMETHOD  */
    THEOBJECTPROP = 287,           /* THEOBJECTPROP  */
    tCAST = 288,                   /* tCAST  */
    tFIELD = 289,                  /* tFIELD  */
    tSCRIPT = 290,                 /* tSCRIPT  */
    tWINDOW = 291,                 /* tWINDOW  */
    tDOWN = 292,                   /* tDOWN  */
    tELSE = 293,                   /* tELSE  */
    tELSIF = 294,                  /* tELSIF  */
    tEXIT = 295,                   /* tEXIT  */
    tGLOBAL = 296,                 /* tGLOBAL  */
    tGO = 297,                     /* tGO  */
    tGOLOOP = 298,                 /* tGOLOOP  */
    tIF = 299,                     /* tIF  */
    tIN = 300,                     /* tIN  */
    tINTO = 301,                   /* tINTO  */
    tMACRO = 302,                  /* tMACRO  */
    tMOVIE = 303,                  /* tMOVIE  */
    tNEXT = 304,                   /* tNEXT  */
    tOF = 305,                     /* tOF  */
    tPREVIOUS = 306,               /* tPREVIOUS  */
    tPUT = 307,                    /* tPUT  */
    tREPEAT = 308,                 /* tREPEAT  */
    tSET = 309,                    /* tSET  */
    tTHEN = 310,                   /* tTHEN  */
    tTO = 311,                     /* tTO  */
    tWHEN = 312,                   /* tWHEN  */
    tWITH = 313,                   /* tWITH  */
    tWHILE = 314,                  /* tWHILE  */
    tFACTORY = 315,                /* tFACTORY  */
    tOPEN = 316,                   /* tOPEN  */
    tPLAY = 317,                   /* tPLAY  */
    tINSTANCE = 318,               /* tINSTANCE  */
    tGE = 319,                     /* tGE  */
    tLE = 320,                     /* tLE  */
    tEQ = 321,                     /* tEQ  */
    tNEQ = 322,                    /* tNEQ  */
    tAND = 323,                    /* tAND  */
    tOR = 324,                     /* tOR  */
    tNOT = 325,                    /* tNOT  */
    tMOD = 326,                    /* tMOD  */
    tAFTER = 327,                  /* tAFTER  */
    tBEFORE = 328,                 /* tBEFORE  */
    tCONCAT = 329,                 /* tCONCAT  */
    tCONTAINS = 330,               /* tCONTAINS  */
    tSTARTS = 331,                 /* tSTARTS  */
    tCHAR = 332,                   /* tCHAR  */
    tITEM = 333,                   /* tITEM  */
    tLINE = 334,                   /* tLINE  */
    tWORD = 335,                   /* tWORD  */
    tSPRITE = 336,                 /* tSPRITE  */
    tINTERSECTS = 337,             /* tINTERSECTS  */
    tWITHIN = 338,                 /* tWITHIN  */
    tTELL = 339,                   /* tTELL  */
    tPROPERTY = 340,               /* tPROPERTY  */
    tON = 341,                     /* tON  */
    tENDIF = 342,                  /* tENDIF  */
    tENDREPEAT = 343,              /* tENDREPEAT  */
    tENDTELL = 344                 /* tENDTELL  */
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
