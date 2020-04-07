/* A Bison parser, made by GNU Bison 3.5.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
    THEMENUITEMSENTITY = 274,
    FLOAT = 275,
    BLTIN = 276,
    FBLTIN = 277,
    RBLTIN = 278,
    ID = 279,
    STRING = 280,
    HANDLER = 281,
    SYMBOL = 282,
    ENDCLAUSE = 283,
    tPLAYACCEL = 284,
    tMETHOD = 285,
    THEOBJECTFIELD = 286,
    THEOBJECTREF = 287,
    tDOWN = 288,
    tELSE = 289,
    tELSIF = 290,
    tEXIT = 291,
    tGLOBAL = 292,
    tGO = 293,
    tIF = 294,
    tIN = 295,
    tINTO = 296,
    tLOOP = 297,
    tMACRO = 298,
    tMOVIE = 299,
    tNEXT = 300,
    tOF = 301,
    tPREVIOUS = 302,
    tPUT = 303,
    tREPEAT = 304,
    tSET = 305,
    tTHEN = 306,
    tTO = 307,
    tWHEN = 308,
    tWITH = 309,
    tWHILE = 310,
    tNLELSE = 311,
    tFACTORY = 312,
    tOPEN = 313,
    tPLAY = 314,
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
#line 85 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

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
