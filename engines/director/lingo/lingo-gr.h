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
     tDOWN = 264,
     tELSE = 265,
     tEND = 266,
     tFRAME = 267,
     tGO = 268,
     tIF = 269,
     tINTO = 270,
     tLOOP = 271,
     tMCI = 272,
     tMCIWAIT = 273,
     tMOVIE = 274,
     tNEXT = 275,
     tOF = 276,
     tPREVIOUS = 277,
     tPUT = 278,
     tREPEAT = 279,
     tSET = 280,
     tTHEN = 281,
     tTO = 282,
     tWITH = 283,
     tWHILE = 284,
     tGE = 285,
     tLE = 286,
     tGT = 287,
     tLT = 288,
     tEQ = 289,
     tNEQ = 290
   };
#endif
/* Tokens.  */
#define UNARY 258
#define VOID 259
#define INT 260
#define FLOAT 261
#define VAR 262
#define STRING 263
#define tDOWN 264
#define tELSE 265
#define tEND 266
#define tFRAME 267
#define tGO 268
#define tIF 269
#define tINTO 270
#define tLOOP 271
#define tMCI 272
#define tMCIWAIT 273
#define tMOVIE 274
#define tNEXT 275
#define tOF 276
#define tPREVIOUS 277
#define tPUT 278
#define tREPEAT 279
#define tSET 280
#define tTHEN 281
#define tTO 282
#define tWITH 283
#define tWHILE 284
#define tGE 285
#define tLE 286
#define tGT 287
#define tLT 288
#define tEQ 289
#define tNEQ 290




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
#line 126 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

