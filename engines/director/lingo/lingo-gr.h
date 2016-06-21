/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     UNARY = 258,
     VOID = 259,
     INT = 260,
     FLOAT = 261,
     VAR = 262,
     STRING = 263,
     tIF = 264,
     tELSE = 265,
     tEND = 266,
     tFRAME = 267,
     tGO = 268,
     tINTO = 269,
     tLOOP = 270,
     tMCI = 271,
     tMCIWAIT = 272,
     tMOVIE = 273,
     tNEXT = 274,
     tOF = 275,
     tPREVIOUS = 276,
     tPUT = 277,
     tREPEAT = 278,
     tSET = 279,
     tTHEN = 280,
     tTO = 281,
     tWHILE = 282,
     tGE = 283,
     tLE = 284,
     tGT = 285,
     tLT = 286,
     tEQ = 287,
     tNEQ = 288
   };
#endif
/* Tokens.  */
#define UNARY 258
#define VOID 259
#define INT 260
#define FLOAT 261
#define VAR 262
#define STRING 263
#define tIF 264
#define tELSE 265
#define tEND 266
#define tFRAME 267
#define tGO 268
#define tINTO 269
#define tLOOP 270
#define tMCI 271
#define tMCIWAIT 272
#define tMOVIE 273
#define tNEXT 274
#define tOF 275
#define tPREVIOUS 276
#define tPUT 277
#define tREPEAT 278
#define tSET 279
#define tTHEN 280
#define tTO 281
#define tWHILE 282
#define tGE 283
#define tLE 284
#define tGT 285
#define tLT 286
#define tEQ 287
#define tNEQ 288




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 65 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int	i;
	float f;
	int code;
}
/* Line 1529 of yacc.c.  */
#line 122 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

