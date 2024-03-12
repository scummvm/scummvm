/* A Bison parser, made by GNU Bison 3.8.2.  */

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
    tINT = 259,                    /* tINT  */
    tFLOAT = 260,                  /* tFLOAT  */
    tVARID = 261,                  /* tVARID  */
    tSTRING = 262,                 /* tSTRING  */
    tSYMBOL = 263,                 /* tSYMBOL  */
    tENDCLAUSE = 264,              /* tENDCLAUSE  */
    tCAST = 265,                   /* tCAST  */
    tFIELD = 266,                  /* tFIELD  */
    tSCRIPT = 267,                 /* tSCRIPT  */
    tWINDOW = 268,                 /* tWINDOW  */
    tDELETE = 269,                 /* tDELETE  */
    tDOWN = 270,                   /* tDOWN  */
    tELSE = 271,                   /* tELSE  */
    tEXIT = 272,                   /* tEXIT  */
    tFRAME = 273,                  /* tFRAME  */
    tGLOBAL = 274,                 /* tGLOBAL  */
    tGO = 275,                     /* tGO  */
    tHILITE = 276,                 /* tHILITE  */
    tIF = 277,                     /* tIF  */
    tIN = 278,                     /* tIN  */
    tINTO = 279,                   /* tINTO  */
    tMACRO = 280,                  /* tMACRO  */
    tRETURN = 281,                 /* tRETURN  */
    tMOVIE = 282,                  /* tMOVIE  */
    tNEXT = 283,                   /* tNEXT  */
    tOF = 284,                     /* tOF  */
    tPREVIOUS = 285,               /* tPREVIOUS  */
    tPUT = 286,                    /* tPUT  */
    tREPEAT = 287,                 /* tREPEAT  */
    tSET = 288,                    /* tSET  */
    tTHEN = 289,                   /* tTHEN  */
    tTO = 290,                     /* tTO  */
    tWHEN = 291,                   /* tWHEN  */
    tWITH = 292,                   /* tWITH  */
    tWHILE = 293,                  /* tWHILE  */
    tFACTORY = 294,                /* tFACTORY  */
    tOPEN = 295,                   /* tOPEN  */
    tPLAY = 296,                   /* tPLAY  */
    tINSTANCE = 297,               /* tINSTANCE  */
    tGE = 298,                     /* tGE  */
    tLE = 299,                     /* tLE  */
    tEQ = 300,                     /* tEQ  */
    tNEQ = 301,                    /* tNEQ  */
    tAND = 302,                    /* tAND  */
    tOR = 303,                     /* tOR  */
    tNOT = 304,                    /* tNOT  */
    tMOD = 305,                    /* tMOD  */
    tAFTER = 306,                  /* tAFTER  */
    tBEFORE = 307,                 /* tBEFORE  */
    tCONCAT = 308,                 /* tCONCAT  */
    tCONTAINS = 309,               /* tCONTAINS  */
    tSTARTS = 310,                 /* tSTARTS  */
    tCHAR = 311,                   /* tCHAR  */
    tCHARS = 312,                  /* tCHARS  */
    tITEM = 313,                   /* tITEM  */
    tITEMS = 314,                  /* tITEMS  */
    tLINE = 315,                   /* tLINE  */
    tLINES = 316,                  /* tLINES  */
    tWORD = 317,                   /* tWORD  */
    tWORDS = 318,                  /* tWORDS  */
    tABBREVIATED = 319,            /* tABBREVIATED  */
    tABBREV = 320,                 /* tABBREV  */
    tABBR = 321,                   /* tABBR  */
    tLONG = 322,                   /* tLONG  */
    tSHORT = 323,                  /* tSHORT  */
    tDATE = 324,                   /* tDATE  */
    tLAST = 325,                   /* tLAST  */
    tMENU = 326,                   /* tMENU  */
    tMENUS = 327,                  /* tMENUS  */
    tMENUITEM = 328,               /* tMENUITEM  */
    tMENUITEMS = 329,              /* tMENUITEMS  */
    tNUMBER = 330,                 /* tNUMBER  */
    tTHE = 331,                    /* tTHE  */
    tTIME = 332,                   /* tTIME  */
    tXTRAS = 333,                  /* tXTRAS  */
    tCASTLIBS = 334,               /* tCASTLIBS  */
    tSOUND = 335,                  /* tSOUND  */
    tSPRITE = 336,                 /* tSPRITE  */
    tINTERSECTS = 337,             /* tINTERSECTS  */
    tWITHIN = 338,                 /* tWITHIN  */
    tTELL = 339,                   /* tTELL  */
    tPROPERTY = 340,               /* tPROPERTY  */
    tON = 341,                     /* tON  */
    tMETHOD = 342,                 /* tMETHOD  */
    tENDIF = 343,                  /* tENDIF  */
    tENDREPEAT = 344,              /* tENDREPEAT  */
    tENDTELL = 345,                /* tENDTELL  */
    tASSERTERROR = 346             /* tASSERTERROR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 119 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	Director::ChunkType chunktype;
	struct {
		Common::String *eventName;
		Common::String *stmt;
	} w;

	Director::IDList *idlist;
	Director::Node *node;
	Director::NodeList *nodelist;

#line 170 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
