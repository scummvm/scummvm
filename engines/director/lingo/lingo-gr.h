/* A Bison parser, made by GNU Bison 3.5.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNARY = 258,
    CASTREF = 259,
    VOID = 260,
    VAR = 261,
    POINT = 262,
    RECT = 263,
    ARRAY = 264,
    OBJECT = 265,
    REFERENCE = 266,
    LEXERROR = 267,
    INT = 268,
    ARGC = 269,
    ARGCNORET = 270,
    THEENTITY = 271,
    THEENTITYWITHID = 272,
    THEMENUITEMENTITY = 273,
    FLOAT = 274,
    BLTIN = 275,
    FBLTIN = 276,
    RBLTIN = 277,
    ID = 278,
    STRING = 279,
    HANDLER = 280,
    SYMBOL = 281,
    ENDCLAUSE = 282,
    tPLAYACCEL = 283,
    tMETHOD = 284,
    THEOBJECTFIELD = 285,
    tDOWN = 286,
    tELSE = 287,
    tELSIF = 288,
    tEXIT = 289,
    tGLOBAL = 290,
    tGO = 291,
    tIF = 292,
    tIN = 293,
    tINTO = 294,
    tLOOP = 295,
    tMACRO = 296,
    tMOVIE = 297,
    tNEXT = 298,
    tOF = 299,
    tPREVIOUS = 300,
    tPUT = 301,
    tREPEAT = 302,
    tSET = 303,
    tTHEN = 304,
    tTO = 305,
    tWHEN = 306,
    tWITH = 307,
    tWHILE = 308,
    tNLELSE = 309,
    tFACTORY = 310,
    tOPEN = 311,
    tPLAY = 312,
    tDONE = 313,
    tINSTANCE = 314,
    tGE = 315,
    tLE = 316,
    tEQ = 317,
    tNEQ = 318,
    tAND = 319,
    tOR = 320,
    tNOT = 321,
    tMOD = 322,
    tAFTER = 323,
    tBEFORE = 324,
    tCONCAT = 325,
    tCONTAINS = 326,
    tSTARTS = 327,
    tCHAR = 328,
    tITEM = 329,
    tLINE = 330,
    tWORD = 331,
    tSPRITE = 332,
    tINTERSECTS = 333,
    tWITHIN = 334,
    tTELL = 335,
    tPROPERTY = 336,
    tON = 337,
    tENDIF = 338,
    tENDREPEAT = 339,
    tENDTELL = 340
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 86 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Common::Array<double> *arr;

	struct {
		Common::String *os;
		int oe;
	} objectfield;

#line 158 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
