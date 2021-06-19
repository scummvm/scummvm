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
    tTHEENTITY = 261,              /* tTHEENTITY  */
    tTHEENTITYWITHID = 262,        /* tTHEENTITYWITHID  */
    tTHEMENUITEMENTITY = 263,      /* tTHEMENUITEMENTITY  */
    tTHEMENUITEMSENTITY = 264,     /* tTHEMENUITEMSENTITY  */
    tFLOAT = 265,                  /* tFLOAT  */
    tTHEFUNC = 266,                /* tTHEFUNC  */
    tTHEFUNCINOF = 267,            /* tTHEFUNCINOF  */
    tVARID = 268,                  /* tVARID  */
    tSTRING = 269,                 /* tSTRING  */
    tSYMBOL = 270,                 /* tSYMBOL  */
    tENDCLAUSE = 271,              /* tENDCLAUSE  */
    tPLAYACCEL = 272,              /* tPLAYACCEL  */
    tTHEOBJECTPROP = 273,          /* tTHEOBJECTPROP  */
    tCAST = 274,                   /* tCAST  */
    tFIELD = 275,                  /* tFIELD  */
    tSCRIPT = 276,                 /* tSCRIPT  */
    tWINDOW = 277,                 /* tWINDOW  */
    tDOWN = 278,                   /* tDOWN  */
    tELSE = 279,                   /* tELSE  */
    tELSIF = 280,                  /* tELSIF  */
    tEXIT = 281,                   /* tEXIT  */
    tFRAME = 282,                  /* tFRAME  */
    tGLOBAL = 283,                 /* tGLOBAL  */
    tGO = 284,                     /* tGO  */
    tIF = 285,                     /* tIF  */
    tIN = 286,                     /* tIN  */
    tINTO = 287,                   /* tINTO  */
    tMACRO = 288,                  /* tMACRO  */
    tMOVIE = 289,                  /* tMOVIE  */
    tNEXT = 290,                   /* tNEXT  */
    tOF = 291,                     /* tOF  */
    tPREVIOUS = 292,               /* tPREVIOUS  */
    tPUT = 293,                    /* tPUT  */
    tREPEAT = 294,                 /* tREPEAT  */
    tSET = 295,                    /* tSET  */
    tTHEN = 296,                   /* tTHEN  */
    tTO = 297,                     /* tTO  */
    tWHEN = 298,                   /* tWHEN  */
    tWITH = 299,                   /* tWITH  */
    tWHILE = 300,                  /* tWHILE  */
    tFACTORY = 301,                /* tFACTORY  */
    tOPEN = 302,                   /* tOPEN  */
    tPLAY = 303,                   /* tPLAY  */
    tINSTANCE = 304,               /* tINSTANCE  */
    tGE = 305,                     /* tGE  */
    tLE = 306,                     /* tLE  */
    tEQ = 307,                     /* tEQ  */
    tNEQ = 308,                    /* tNEQ  */
    tAND = 309,                    /* tAND  */
    tOR = 310,                     /* tOR  */
    tNOT = 311,                    /* tNOT  */
    tMOD = 312,                    /* tMOD  */
    tAFTER = 313,                  /* tAFTER  */
    tBEFORE = 314,                 /* tBEFORE  */
    tCONCAT = 315,                 /* tCONCAT  */
    tCONTAINS = 316,               /* tCONTAINS  */
    tSTARTS = 317,                 /* tSTARTS  */
    tCHAR = 318,                   /* tCHAR  */
    tITEM = 319,                   /* tITEM  */
    tLINE = 320,                   /* tLINE  */
    tWORD = 321,                   /* tWORD  */
    tSPRITE = 322,                 /* tSPRITE  */
    tINTERSECTS = 323,             /* tINTERSECTS  */
    tWITHIN = 324,                 /* tWITHIN  */
    tTELL = 325,                   /* tTELL  */
    tPROPERTY = 326,               /* tPROPERTY  */
    tON = 327,                     /* tON  */
    tMETHOD = 328,                 /* tMETHOD  */
    tENDIF = 329,                  /* tENDIF  */
    tENDREPEAT = 330,              /* tENDREPEAT  */
    tENDTELL = 331,                /* tENDTELL  */
    tASSERTERROR = 332             /* tASSERTERROR  */
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
	int e[2];	// Entity + field

	struct {
		Common::String *obj;
		Common::String *prop;
	} objectprop;

	Director::IDList *idlist;
	Director::Node *node;
	Director::NodeList *nodelist;

#line 157 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
