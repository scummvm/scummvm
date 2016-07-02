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
     CASTREF = 258,
     UNARY = 259,
     VOID = 260,
     VAR = 261,
     INT = 262,
     FLOAT = 263,
     BLTIN = 264,
     ID = 265,
     STRING = 266,
     HANDLER = 267,
     tDOWN = 268,
     tELSE = 269,
     tEND = 270,
     tEXIT = 271,
     tFRAME = 272,
     tGLOBAL = 273,
     tGO = 274,
     tIF = 275,
     tINTO = 276,
     tLOOP = 277,
     tMACRO = 278,
     tMCI = 279,
     tMCIWAIT = 280,
     tMOVIE = 281,
     tNEXT = 282,
     tOF = 283,
     tPREVIOUS = 284,
     tPUT = 285,
     tREPEAT = 286,
     tSET = 287,
     tTHEN = 288,
     tTO = 289,
     tWITH = 290,
     tWHILE = 291,
     tGE = 292,
     tLE = 293,
     tGT = 294,
     tLT = 295,
     tEQ = 296,
     tNEQ = 297
   };
#endif
/* Tokens.  */
#define CASTREF 258
#define UNARY 259
#define VOID 260
#define VAR 261
#define INT 262
#define FLOAT 263
#define BLTIN 264
#define ID 265
#define STRING 266
#define HANDLER 267
#define tDOWN 268
#define tELSE 269
#define tEND 270
#define tEXIT 271
#define tFRAME 272
#define tGLOBAL 273
#define tGO 274
#define tIF 275
#define tINTO 276
#define tLOOP 277
#define tMACRO 278
#define tMCI 279
#define tMCIWAIT 280
#define tMOVIE 281
#define tNEXT 282
#define tOF 283
#define tPREVIOUS 284
#define tPUT 285
#define tREPEAT 286
#define tSET 287
#define tTHEN 288
#define tTO 289
#define tWITH 290
#define tWHILE 291
#define tGE 292
#define tLE 293
#define tGT 294
#define tLT 295
#define tEQ 296
#define tNEQ 297




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
#line 141 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

