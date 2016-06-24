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
     HANDLER = 264,
     tDOWN = 265,
     tELSE = 266,
     tEND = 267,
     tEXIT = 268,
     tFRAME = 269,
     tGO = 270,
     tIF = 271,
     tINTO = 272,
     tLOOP = 273,
     tMACRO = 274,
     tMCI = 275,
     tMCIWAIT = 276,
     tMOVIE = 277,
     tNEXT = 278,
     tOF = 279,
     tPREVIOUS = 280,
     tPUT = 281,
     tREPEAT = 282,
     tSET = 283,
     tTHEN = 284,
     tTO = 285,
     tWITH = 286,
     tWHILE = 287,
     tGE = 288,
     tLE = 289,
     tGT = 290,
     tLT = 291,
     tEQ = 292,
     tNEQ = 293
   };
#endif
/* Tokens.  */
#define UNARY 258
#define VOID 259
#define INT 260
#define FLOAT 261
#define VAR 262
#define STRING 263
#define HANDLER 264
#define tDOWN 265
#define tELSE 266
#define tEND 267
#define tEXIT 268
#define tFRAME 269
#define tGO 270
#define tIF 271
#define tINTO 272
#define tLOOP 273
#define tMACRO 274
#define tMCI 275
#define tMCIWAIT 276
#define tMOVIE 277
#define tNEXT 278
#define tOF 279
#define tPREVIOUS 280
#define tPUT 281
#define tREPEAT 282
#define tSET 283
#define tTHEN 284
#define tTO 285
#define tWITH 286
#define tWHILE 287
#define tGE 288
#define tLE 289
#define tGT 290
#define tLT 291
#define tEQ 292
#define tNEQ 293




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
#line 133 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

