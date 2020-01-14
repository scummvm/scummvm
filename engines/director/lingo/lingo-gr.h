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
    FLOAT = 272,
    BLTIN = 273,
    FBLTIN = 274,
    RBLTIN = 275,
    ID = 276,
    STRING = 277,
    HANDLER = 278,
    SYMBOL = 279,
    ENDCLAUSE = 280,
    tPLAYACCEL = 281,
    tMETHOD = 282,
    THEOBJECTFIELD = 283,
    tDOWN = 284,
    tELSE = 285,
    tELSIF = 286,
    tEXIT = 287,
    tGLOBAL = 288,
    tGO = 289,
    tIF = 290,
    tINTO = 291,
    tLOOP = 292,
    tMACRO = 293,
    tMOVIE = 294,
    tNEXT = 295,
    tOF = 296,
    tPREVIOUS = 297,
    tPUT = 298,
    tREPEAT = 299,
    tSET = 300,
    tTHEN = 301,
    tTO = 302,
    tWHEN = 303,
    tWITH = 304,
    tWHILE = 305,
    tNLELSE = 306,
    tFACTORY = 307,
    tOPEN = 308,
    tPLAY = 309,
    tDONE = 310,
    tINSTANCE = 311,
    tGE = 312,
    tLE = 313,
    tEQ = 314,
    tNEQ = 315,
    tAND = 316,
    tOR = 317,
    tNOT = 318,
    tMOD = 319,
    tAFTER = 320,
    tBEFORE = 321,
    tCONCAT = 322,
    tCONTAINS = 323,
    tSTARTS = 324,
    tCHAR = 325,
    tITEM = 326,
    tLINE = 327,
    tWORD = 328,
    tSPRITE = 329,
    tINTERSECTS = 330,
    tWITHIN = 331,
    tTELL = 332,
    tPROPERTY = 333,
    tON = 334,
    tENDIF = 335,
    tENDREPEAT = 336,
    tENDTELL = 337
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

#line 155 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
