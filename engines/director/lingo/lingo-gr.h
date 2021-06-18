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
    tGOLOOP = 285,                 /* tGOLOOP  */
    tIF = 286,                     /* tIF  */
    tIN = 287,                     /* tIN  */
    tINTO = 288,                   /* tINTO  */
    tMACRO = 289,                  /* tMACRO  */
    tMOVIE = 290,                  /* tMOVIE  */
    tNEXT = 291,                   /* tNEXT  */
    tOF = 292,                     /* tOF  */
    tPREVIOUS = 293,               /* tPREVIOUS  */
    tPUT = 294,                    /* tPUT  */
    tREPEAT = 295,                 /* tREPEAT  */
    tSET = 296,                    /* tSET  */
    tTHEN = 297,                   /* tTHEN  */
    tTO = 298,                     /* tTO  */
    tWHEN = 299,                   /* tWHEN  */
    tWITH = 300,                   /* tWITH  */
    tWHILE = 301,                  /* tWHILE  */
    tFACTORY = 302,                /* tFACTORY  */
    tOPEN = 303,                   /* tOPEN  */
    tPLAY = 304,                   /* tPLAY  */
    tINSTANCE = 305,               /* tINSTANCE  */
    tGE = 306,                     /* tGE  */
    tLE = 307,                     /* tLE  */
    tEQ = 308,                     /* tEQ  */
    tNEQ = 309,                    /* tNEQ  */
    tAND = 310,                    /* tAND  */
    tOR = 311,                     /* tOR  */
    tNOT = 312,                    /* tNOT  */
    tMOD = 313,                    /* tMOD  */
    tAFTER = 314,                  /* tAFTER  */
    tBEFORE = 315,                 /* tBEFORE  */
    tCONCAT = 316,                 /* tCONCAT  */
    tCONTAINS = 317,               /* tCONTAINS  */
    tSTARTS = 318,                 /* tSTARTS  */
    tCHAR = 319,                   /* tCHAR  */
    tITEM = 320,                   /* tITEM  */
    tLINE = 321,                   /* tLINE  */
    tWORD = 322,                   /* tWORD  */
    tSPRITE = 323,                 /* tSPRITE  */
    tINTERSECTS = 324,             /* tINTERSECTS  */
    tWITHIN = 325,                 /* tWITHIN  */
    tTELL = 326,                   /* tTELL  */
    tPROPERTY = 327,               /* tPROPERTY  */
    tON = 328,                     /* tON  */
    tMETHOD = 329,                 /* tMETHOD  */
    tENDIF = 330,                  /* tENDIF  */
    tENDREPEAT = 331,              /* tENDREPEAT  */
    tENDTELL = 332,                /* tENDTELL  */
    tASSERTERROR = 333             /* tASSERTERROR  */
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

#line 158 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
