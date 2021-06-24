/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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
    tUNARY = 258,                  /* tUNARY  */
    tLEXERROR = 259,               /* tLEXERROR  */
    tINT = 260,                    /* tINT  */
    tFLOAT = 261,                  /* tFLOAT  */
    tVARID = 262,                  /* tVARID  */
    tSTRING = 263,                 /* tSTRING  */
    tSYMBOL = 264,                 /* tSYMBOL  */
    tENDCLAUSE = 265,              /* tENDCLAUSE  */
    tCAST = 266,                   /* tCAST  */
    tFIELD = 267,                  /* tFIELD  */
    tSCRIPT = 268,                 /* tSCRIPT  */
    tWINDOW = 269,                 /* tWINDOW  */
    tDELETE = 270,                 /* tDELETE  */
    tDOWN = 271,                   /* tDOWN  */
    tELSE = 272,                   /* tELSE  */
    tELSIF = 273,                  /* tELSIF  */
    tEXIT = 274,                   /* tEXIT  */
    tFRAME = 275,                  /* tFRAME  */
    tGLOBAL = 276,                 /* tGLOBAL  */
    tGO = 277,                     /* tGO  */
    tHILITE = 278,                 /* tHILITE  */
    tIF = 279,                     /* tIF  */
    tIN = 280,                     /* tIN  */
    tINTO = 281,                   /* tINTO  */
    tMACRO = 282,                  /* tMACRO  */
    tMOVIE = 283,                  /* tMOVIE  */
    tNEXT = 284,                   /* tNEXT  */
    tOF = 285,                     /* tOF  */
    tPREVIOUS = 286,               /* tPREVIOUS  */
    tPUT = 287,                    /* tPUT  */
    tREPEAT = 288,                 /* tREPEAT  */
    tSET = 289,                    /* tSET  */
    tTHEN = 290,                   /* tTHEN  */
    tTO = 291,                     /* tTO  */
    tWHEN = 292,                   /* tWHEN  */
    tWITH = 293,                   /* tWITH  */
    tWHILE = 294,                  /* tWHILE  */
    tFACTORY = 295,                /* tFACTORY  */
    tOPEN = 296,                   /* tOPEN  */
    tPLAY = 297,                   /* tPLAY  */
    tINSTANCE = 298,               /* tINSTANCE  */
    tGE = 299,                     /* tGE  */
    tLE = 300,                     /* tLE  */
    tEQ = 301,                     /* tEQ  */
    tNEQ = 302,                    /* tNEQ  */
    tAND = 303,                    /* tAND  */
    tOR = 304,                     /* tOR  */
    tNOT = 305,                    /* tNOT  */
    tMOD = 306,                    /* tMOD  */
    tAFTER = 307,                  /* tAFTER  */
    tBEFORE = 308,                 /* tBEFORE  */
    tCONCAT = 309,                 /* tCONCAT  */
    tCONTAINS = 310,               /* tCONTAINS  */
    tSTARTS = 311,                 /* tSTARTS  */
    tCHAR = 312,                   /* tCHAR  */
    tCHARS = 313,                  /* tCHARS  */
    tITEM = 314,                   /* tITEM  */
    tITEMS = 315,                  /* tITEMS  */
    tLINE = 316,                   /* tLINE  */
    tLINES = 317,                  /* tLINES  */
    tWORD = 318,                   /* tWORD  */
    tWORDS = 319,                  /* tWORDS  */
    tABBREVIATED = 320,            /* tABBREVIATED  */
    tABBREV = 321,                 /* tABBREV  */
    tABBR = 322,                   /* tABBR  */
    tLONG = 323,                   /* tLONG  */
    tSHORT = 324,                  /* tSHORT  */
    tCASTMEMBERS = 325,            /* tCASTMEMBERS  */
    tDATE = 326,                   /* tDATE  */
    tLAST = 327,                   /* tLAST  */
    tMENU = 328,                   /* tMENU  */
    tMENUITEM = 329,               /* tMENUITEM  */
    tMENUITEMS = 330,              /* tMENUITEMS  */
    tNUMBER = 331,                 /* tNUMBER  */
    tTHE = 332,                    /* tTHE  */
    tTIME = 333,                   /* tTIME  */
    tSOUND = 334,                  /* tSOUND  */
    tSPRITE = 335,                 /* tSPRITE  */
    tINTERSECTS = 336,             /* tINTERSECTS  */
    tWITHIN = 337,                 /* tWITHIN  */
    tTELL = 338,                   /* tTELL  */
    tPROPERTY = 339,               /* tPROPERTY  */
    tON = 340,                     /* tON  */
    tMETHOD = 341,                 /* tMETHOD  */
    tENDIF = 342,                  /* tENDIF  */
    tENDREPEAT = 343,              /* tENDREPEAT  */
    tENDTELL = 344,                /* tENDTELL  */
    tASSERTERROR = 345             /* tASSERTERROR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 112 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	Director::ChunkType chunktype;

	Director::IDList *idlist;
	Director::Node *node;
	Director::NodeList *nodelist;

#line 165 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
