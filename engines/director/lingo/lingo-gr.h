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
     tEXIT = 267,
     tFRAME = 268,
     tGO = 269,
     tIF = 270,
     tINTO = 271,
     tLOOP = 272,
     tMACRO = 273,
     tMCI = 274,
     tMCIWAIT = 275,
     tMOVIE = 276,
     tNEXT = 277,
     tOF = 278,
     tPREVIOUS = 279,
     tPUT = 280,
     tREPEAT = 281,
     tSET = 282,
     tTHEN = 283,
     tTO = 284,
     tWITH = 285,
     tWHILE = 286,
     tGE = 287,
     tLE = 288,
     tGT = 289,
     tLT = 290,
     tEQ = 291,
     tNEQ = 292
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
#define tEXIT 267
#define tFRAME 268
#define tGO 269
#define tIF 270
#define tINTO 271
#define tLOOP 272
#define tMACRO 273
#define tMCI 274
#define tMCIWAIT 275
#define tMOVIE 276
#define tNEXT 277
#define tOF 278
#define tPREVIOUS 279
#define tPUT 280
#define tREPEAT 281
#define tSET 282
#define tTHEN 283
#define tTO 284
#define tWITH 285
#define tWHILE 286
#define tGE 287
#define tLE 288
#define tGT 289
#define tLT 290
#define tEQ 291
#define tNEQ 292




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
#line 131 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

