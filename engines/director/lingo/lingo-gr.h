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
    INT = 267,
    ARGC = 268,
    ARGCNORET = 269,
    THEENTITY = 270,
    THEENTITYWITHID = 271,
    THEMENUITEMENTITY = 272,
    FLOAT = 273,
    BLTIN = 274,
    FBLTIN = 275,
    RBLTIN = 276,
    ID = 277,
    STRING = 278,
    HANDLER = 279,
    SYMBOL = 280,
    ENDCLAUSE = 281,
    tPLAYACCEL = 282,
    tMETHOD = 283,
    THEOBJECTFIELD = 284,
    tDOWN = 285,
    tELSE = 286,
    tELSIF = 287,
    tEXIT = 288,
    tGLOBAL = 289,
    tGO = 290,
    tIF = 291,
    tINTO = 292,
    tLOOP = 293,
    tMACRO = 294,
    tMOVIE = 295,
    tNEXT = 296,
    tOF = 297,
    tPREVIOUS = 298,
    tPUT = 299,
    tREPEAT = 300,
    tSET = 301,
    tTHEN = 302,
    tTO = 303,
    tWHEN = 304,
    tWITH = 305,
    tWHILE = 306,
    tNLELSE = 307,
    tFACTORY = 308,
    tOPEN = 309,
    tPLAY = 310,
    tDONE = 311,
    tINSTANCE = 312,
    tGE = 313,
    tLE = 314,
    tEQ = 315,
    tNEQ = 316,
    tAND = 317,
    tOR = 318,
    tNOT = 319,
    tMOD = 320,
    tAFTER = 321,
    tBEFORE = 322,
    tCONCAT = 323,
    tCONTAINS = 324,
    tSTARTS = 325,
    tCHAR = 326,
    tITEM = 327,
    tLINE = 328,
    tWORD = 329,
    tSPRITE = 330,
    tINTERSECTS = 331,
    tWITHIN = 332,
    tTELL = 333,
    tPROPERTY = 334,
    tON = 335,
    tENDIF = 336,
    tENDREPEAT = 337,
    tENDTELL = 338
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

#line 156 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
