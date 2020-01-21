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
    THEOBJECTREF = 286,
    tDOWN = 287,
    tELSE = 288,
    tELSIF = 289,
    tEXIT = 290,
    tGLOBAL = 291,
    tGO = 292,
    tIF = 293,
    tIN = 294,
    tINTO = 295,
    tLOOP = 296,
    tMACRO = 297,
    tMOVIE = 298,
    tNEXT = 299,
    tOF = 300,
    tPREVIOUS = 301,
    tPUT = 302,
    tREPEAT = 303,
    tSET = 304,
    tTHEN = 305,
    tTO = 306,
    tWHEN = 307,
    tWITH = 308,
    tWHILE = 309,
    tNLELSE = 310,
    tFACTORY = 311,
    tOPEN = 312,
    tPLAY = 313,
    tDONE = 314,
    tINSTANCE = 315,
    tGE = 316,
    tLE = 317,
    tEQ = 318,
    tNEQ = 319,
    tAND = 320,
    tOR = 321,
    tNOT = 322,
    tMOD = 323,
    tAFTER = 324,
    tBEFORE = 325,
    tCONCAT = 326,
    tCONTAINS = 327,
    tSTARTS = 328,
    tCHAR = 329,
    tITEM = 330,
    tLINE = 331,
    tWORD = 332,
    tSPRITE = 333,
    tINTERSECTS = 334,
    tWITHIN = 335,
    tTELL = 336,
    tPROPERTY = 337,
    tON = 338,
    tENDIF = 339,
    tENDREPEAT = 340,
    tENDTELL = 341
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

	struct {
		Common::String *obj;
		Common::String *field;
	} objectref;

#line 164 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
