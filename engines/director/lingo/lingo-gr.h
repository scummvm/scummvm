/* A Bison parser, made by GNU Bison 3.4.  */

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

#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED
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
    TWOWORDBUILTIN = 278,
    FBLTIN = 279,
    FBLTINNOARGS = 280,
    FBLTINONEARG = 281,
    FBLTINARGLIST = 282,
    RBLTIN = 283,
    RBLTINONEARG = 284,
    ID = 285,
    STRING = 286,
    HANDLER = 287,
    SYMBOL = 288,
    ENDCLAUSE = 289,
    tPLAYACCEL = 290,
    tDOWN = 291,
    tELSE = 292,
    tNLELSIF = 293,
    tEXIT = 294,
    tFRAME = 295,
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
    tTHENNL = 310,
    tTO = 311,
    tWHEN = 312,
    tWITH = 313,
    tWHILE = 314,
    tNLELSE = 315,
    tFACTORY = 316,
    tMETHOD = 317,
    tOPEN = 318,
    tPLAY = 319,
    tDONE = 320,
    tINSTANCE = 321,
    tGE = 322,
    tLE = 323,
    tGT = 324,
    tLT = 325,
    tEQ = 326,
    tNEQ = 327,
    tAND = 328,
    tOR = 329,
    tNOT = 330,
    tMOD = 331,
    tAFTER = 332,
    tBEFORE = 333,
    tCONCAT = 334,
    tCONTAINS = 335,
    tSTARTS = 336,
    tCHAR = 337,
    tITEM = 338,
    tLINE = 339,
    tWORD = 340,
    tSPRITE = 341,
    tINTERSECTS = 342,
    tWITHIN = 343,
    tTELL = 344,
    tPROPERTY = 345,
    tON = 346,
    tME = 347
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 79 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Common::Array<double> *arr;

#line 160 "engines/director/lingo/lingo-gr.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED  */
