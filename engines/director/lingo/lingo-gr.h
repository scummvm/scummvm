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
    tMOVIE = 281,                  /* tMOVIE  */
    tNEXT = 282,                   /* tNEXT  */
    tOF = 283,                     /* tOF  */
    tPREVIOUS = 284,               /* tPREVIOUS  */
    tPUT = 285,                    /* tPUT  */
    tREPEAT = 286,                 /* tREPEAT  */
    tSET = 287,                    /* tSET  */
    tTHEN = 288,                   /* tTHEN  */
    tTO = 289,                     /* tTO  */
    tWHEN = 290,                   /* tWHEN  */
    tWITH = 291,                   /* tWITH  */
    tWHILE = 292,                  /* tWHILE  */
    tFACTORY = 293,                /* tFACTORY  */
    tOPEN = 294,                   /* tOPEN  */
    tPLAY = 295,                   /* tPLAY  */
    tINSTANCE = 296,               /* tINSTANCE  */
    tGE = 297,                     /* tGE  */
    tLE = 298,                     /* tLE  */
    tEQ = 299,                     /* tEQ  */
    tNEQ = 300,                    /* tNEQ  */
    tAND = 301,                    /* tAND  */
    tOR = 302,                     /* tOR  */
    tNOT = 303,                    /* tNOT  */
    tMOD = 304,                    /* tMOD  */
    tAFTER = 305,                  /* tAFTER  */
    tBEFORE = 306,                 /* tBEFORE  */
    tCONCAT = 307,                 /* tCONCAT  */
    tCONTAINS = 308,               /* tCONTAINS  */
    tSTARTS = 309,                 /* tSTARTS  */
    tCHAR = 310,                   /* tCHAR  */
    tCHARS = 311,                  /* tCHARS  */
    tITEM = 312,                   /* tITEM  */
    tITEMS = 313,                  /* tITEMS  */
    tLINE = 314,                   /* tLINE  */
    tLINES = 315,                  /* tLINES  */
    tWORD = 316,                   /* tWORD  */
    tWORDS = 317,                  /* tWORDS  */
    tABBREVIATED = 318,            /* tABBREVIATED  */
    tABBREV = 319,                 /* tABBREV  */
    tABBR = 320,                   /* tABBR  */
    tLONG = 321,                   /* tLONG  */
    tSHORT = 322,                  /* tSHORT  */
    tDATE = 323,                   /* tDATE  */
    tLAST = 324,                   /* tLAST  */
    tMENU = 325,                   /* tMENU  */
    tMENUS = 326,                  /* tMENUS  */
    tMENUITEM = 327,               /* tMENUITEM  */
    tMENUITEMS = 328,              /* tMENUITEMS  */
    tNUMBER = 329,                 /* tNUMBER  */
    tTHE = 330,                    /* tTHE  */
    tTIME = 331,                   /* tTIME  */
    tXTRAS = 332,                  /* tXTRAS  */
    tCASTLIBS = 333,               /* tCASTLIBS  */
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
#line 111 "engines/director/lingo/lingo-gr.y"

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

#line 169 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
