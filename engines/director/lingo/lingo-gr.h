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
     VAR = 260,
     INT = 261,
     FLOAT = 262,
     ID = 263,
     STRING = 264,
     HANDLER = 265,
     tDOWN = 266,
     tELSE = 267,
     tEND = 268,
     tEXIT = 269,
     tFRAME = 270,
     tGO = 271,
     tIF = 272,
     tINTO = 273,
     tLOOP = 274,
     tMACRO = 275,
     tMCI = 276,
     tMCIWAIT = 277,
     tMOVIE = 278,
     tNEXT = 279,
     tOF = 280,
     tPREVIOUS = 281,
     tPUT = 282,
     tREPEAT = 283,
     tSET = 284,
     tTHEN = 285,
     tTO = 286,
     tWITH = 287,
     tWHILE = 288,
     tGE = 289,
     tLE = 290,
     tGT = 291,
     tLT = 292,
     tEQ = 293,
     tNEQ = 294
   };
#endif
/* Tokens.  */
#define UNARY 258
#define VOID 259
#define VAR 260
#define INT 261
#define FLOAT 262
#define ID 263
#define STRING 264
#define HANDLER 265
#define tDOWN 266
#define tELSE 267
#define tEND 268
#define tEXIT 269
#define tFRAME 270
#define tGO 271
#define tIF 272
#define tINTO 273
#define tLOOP 274
#define tMACRO 275
#define tMCI 276
#define tMCIWAIT 277
#define tMOVIE 278
#define tNEXT 279
#define tOF 280
#define tPREVIOUS 281
#define tPUT 282
#define tREPEAT 283
#define tSET 284
#define tTHEN 285
#define tTO 286
#define tWITH 287
#define tWHILE 288
#define tGE 289
#define tLE 290
#define tGT 291
#define tLT 292
#define tEQ 293
#define tNEQ 294




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
#line 135 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

