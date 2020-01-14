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
    tMETHOD = 291,
    THEOBJECTFIELD = 292,
    tDOWN = 293,
    tELSE = 294,
    tELSIF = 295,
    tEXIT = 296,
    tGLOBAL = 297,
    tGO = 298,
    tIF = 299,
    tINTO = 300,
    tLOOP = 301,
    tMACRO = 302,
    tMOVIE = 303,
    tNEXT = 304,
    tOF = 305,
    tPREVIOUS = 306,
    tPUT = 307,
    tREPEAT = 308,
    tSET = 309,
    tTHEN = 310,
    tTO = 311,
    tWHEN = 312,
    tWITH = 313,
    tWHILE = 314,
    tNLELSE = 315,
    tFACTORY = 316,
    tOPEN = 317,
    tPLAY = 318,
    tDONE = 319,
    tINSTANCE = 320,
    tGE = 321,
    tLE = 322,
    tEQ = 323,
    tNEQ = 324,
    tAND = 325,
    tOR = 326,
    tNOT = 327,
    tMOD = 328,
    tAFTER = 329,
    tBEFORE = 330,
    tCONCAT = 331,
    tCONTAINS = 332,
    tSTARTS = 333,
    tCHAR = 334,
    tITEM = 335,
    tLINE = 336,
    tWORD = 337,
    tSPRITE = 338,
    tINTERSECTS = 339,
    tWITHIN = 340,
    tTELL = 341,
    tPROPERTY = 342,
    tON = 343,
    tENDIF = 344,
    tENDREPEAT = 345,
    tENDTELL = 346
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 87 "engines/director/lingo/lingo-gr.y"

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

#line 164 "engines/director/lingo/lingo-gr.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED  */
