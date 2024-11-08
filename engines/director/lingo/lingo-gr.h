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
    tMEMBER = 269,                 /* tMEMBER  */
    tCASTLIB = 270,                /* tCASTLIB  */
    tDELETE = 271,                 /* tDELETE  */
    tDOWN = 272,                   /* tDOWN  */
    tELSE = 273,                   /* tELSE  */
    tEXIT = 274,                   /* tEXIT  */
    tFRAME = 275,                  /* tFRAME  */
    tGLOBAL = 276,                 /* tGLOBAL  */
    tGO = 277,                     /* tGO  */
    tHILITE = 278,                 /* tHILITE  */
    tIF = 279,                     /* tIF  */
    tIN = 280,                     /* tIN  */
    tINTO = 281,                   /* tINTO  */
    tMACRO = 282,                  /* tMACRO  */
    tRETURN = 283,                 /* tRETURN  */
    tMOVIE = 284,                  /* tMOVIE  */
    tNEXT = 285,                   /* tNEXT  */
    tOF = 286,                     /* tOF  */
    tPREVIOUS = 287,               /* tPREVIOUS  */
    tPUT = 288,                    /* tPUT  */
    tREPEAT = 289,                 /* tREPEAT  */
    tSET = 290,                    /* tSET  */
    tTHEN = 291,                   /* tTHEN  */
    tTO = 292,                     /* tTO  */
    tWHEN = 293,                   /* tWHEN  */
    tWITH = 294,                   /* tWITH  */
    tWHILE = 295,                  /* tWHILE  */
    tFACTORY = 296,                /* tFACTORY  */
    tOPEN = 297,                   /* tOPEN  */
    tPLAY = 298,                   /* tPLAY  */
    tINSTANCE = 299,               /* tINSTANCE  */
    tGE = 300,                     /* tGE  */
    tLE = 301,                     /* tLE  */
    tEQ = 302,                     /* tEQ  */
    tNEQ = 303,                    /* tNEQ  */
    tAND = 304,                    /* tAND  */
    tOR = 305,                     /* tOR  */
    tNOT = 306,                    /* tNOT  */
    tMOD = 307,                    /* tMOD  */
    tAFTER = 308,                  /* tAFTER  */
    tBEFORE = 309,                 /* tBEFORE  */
    tCONCAT = 310,                 /* tCONCAT  */
    tCONTAINS = 311,               /* tCONTAINS  */
    tSTARTS = 312,                 /* tSTARTS  */
    tCHAR = 313,                   /* tCHAR  */
    tCHARS = 314,                  /* tCHARS  */
    tITEM = 315,                   /* tITEM  */
    tITEMS = 316,                  /* tITEMS  */
    tLINE = 317,                   /* tLINE  */
    tLINES = 318,                  /* tLINES  */
    tWORD = 319,                   /* tWORD  */
    tWORDS = 320,                  /* tWORDS  */
    tABBREVIATED = 321,            /* tABBREVIATED  */
    tABBREV = 322,                 /* tABBREV  */
    tABBR = 323,                   /* tABBR  */
    tLONG = 324,                   /* tLONG  */
    tSHORT = 325,                  /* tSHORT  */
    tDATE = 326,                   /* tDATE  */
    tLAST = 327,                   /* tLAST  */
    tMENU = 328,                   /* tMENU  */
    tMENUS = 329,                  /* tMENUS  */
    tMENUITEM = 330,               /* tMENUITEM  */
    tMENUITEMS = 331,              /* tMENUITEMS  */
    tNUMBER = 332,                 /* tNUMBER  */
    tTHE = 333,                    /* tTHE  */
    tTIME = 334,                   /* tTIME  */
    tXTRAS = 335,                  /* tXTRAS  */
    tCASTLIBS = 336,               /* tCASTLIBS  */
    tSOUND = 337,                  /* tSOUND  */
    tSPRITE = 338,                 /* tSPRITE  */
    tINTERSECTS = 339,             /* tINTERSECTS  */
    tWITHIN = 340,                 /* tWITHIN  */
    tTELL = 341,                   /* tTELL  */
    tPROPERTY = 342,               /* tPROPERTY  */
    tON = 343,                     /* tON  */
    tMETHOD = 344,                 /* tMETHOD  */
    tENDIF = 345,                  /* tENDIF  */
    tENDREPEAT = 346,              /* tENDREPEAT  */
    tENDTELL = 347,                /* tENDTELL  */
    tASSERTERROR = 348             /* tASSERTERROR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 118 "engines/director/lingo/lingo-gr.y"

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

#line 172 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
