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
     tMACRO = 272,
     tMCI = 273,
     tMCIWAIT = 274,
     tMOVIE = 275,
     tNEXT = 276,
     tOF = 277,
     tPREVIOUS = 278,
     tPUT = 279,
     tREPEAT = 280,
     tSET = 281,
     tTHEN = 282,
     tTO = 283,
     tWITH = 284,
     tWHILE = 285,
     tGE = 286,
     tLE = 287,
     tGT = 288,
     tLT = 289,
     tEQ = 290,
     tNEQ = 291
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
#define tMACRO 272
#define tMCI 273
#define tMCIWAIT 274
#define tMOVIE 275
#define tNEXT 276
#define tOF 277
#define tPREVIOUS 278
#define tPUT 279
#define tREPEAT 280
#define tSET 281
#define tTHEN 282
#define tTO 283
#define tWITH 284
#define tWHILE 285
#define tGE 286
#define tLE 287
#define tGT 288
#define tLT 289
#define tEQ 290
#define tNEQ 291




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 65 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int	i;
	float f;
	int code;
	int	narg;	/* number of arguments */
}
/* Line 1529 of yacc.c.  */
#line 129 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

