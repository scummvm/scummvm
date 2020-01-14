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
    BLTINNOARGS = 274,
    BLTINNOARGSORONE = 275,
    BLTINONEARG = 276,
    BLTINARGLIST = 277,
    FBLTIN = 278,
    FBLTINNOARGS = 279,
    FBLTINONEARG = 280,
    FBLTINARGLIST = 281,
    RBLTIN = 282,
    RBLTINONEARG = 283,
    ID = 284,
    STRING = 285,
    HANDLER = 286,
    SYMBOL = 287,
    ENDCLAUSE = 288,
    tPLAYACCEL = 289,
    tMETHOD = 290,
    THEOBJECTFIELD = 291,
    tDOWN = 292,
    tELSE = 293,
    tELSIF = 294,
    tEXIT = 295,
    tGLOBAL = 296,
    tGO = 297,
    tIF = 298,
    tINTO = 299,
    tLOOP = 300,
    tMACRO = 301,
    tMOVIE = 302,
    tNEXT = 303,
    tOF = 304,
    tPREVIOUS = 305,
    tPUT = 306,
    tREPEAT = 307,
    tSET = 308,
    tTHEN = 309,
    tTO = 310,
    tWHEN = 311,
    tWITH = 312,
    tWHILE = 313,
    tNLELSE = 314,
    tFACTORY = 315,
    tOPEN = 316,
    tPLAY = 317,
    tDONE = 318,
    tINSTANCE = 319,
    tGE = 320,
    tLE = 321,
    tEQ = 322,
    tNEQ = 323,
    tAND = 324,
    tOR = 325,
    tNOT = 326,
    tMOD = 327,
    tAFTER = 328,
    tBEFORE = 329,
    tCONCAT = 330,
    tCONTAINS = 331,
    tSTARTS = 332,
    tCHAR = 333,
    tITEM = 334,
    tLINE = 335,
    tWORD = 336,
    tSPRITE = 337,
    tINTERSECTS = 338,
    tWITHIN = 339,
    tTELL = 340,
    tPROPERTY = 341,
    tON = 342,
    tENDIF = 343,
    tENDREPEAT = 344,
    tENDTELL = 345
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
		Common::String *s;
		int e;
	} objectfield;

#line 163 "engines/director/lingo/lingo-gr.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
