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
     tGLOBAL = 271,
     tGO = 272,
     tIF = 273,
     tINTO = 274,
     tLOOP = 275,
     tMACRO = 276,
     tMCI = 277,
     tMCIWAIT = 278,
     tMOVIE = 279,
     tNEXT = 280,
     tOF = 281,
     tPREVIOUS = 282,
     tPUT = 283,
     tREPEAT = 284,
     tSET = 285,
     tTHEN = 286,
     tTO = 287,
     tWITH = 288,
     tWHILE = 289,
     tGE = 290,
     tLE = 291,
     tGT = 292,
     tLT = 293,
     tEQ = 294,
     tNEQ = 295
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
#define tGLOBAL 271
#define tGO 272
#define tIF 273
#define tINTO 274
#define tLOOP 275
#define tMACRO 276
#define tMCI 277
#define tMCIWAIT 278
#define tMOVIE 279
#define tNEXT 280
#define tOF 281
#define tPREVIOUS 282
#define tPUT 283
#define tREPEAT 284
#define tSET 285
#define tTHEN 286
#define tTO 287
#define tWITH 288
#define tWHILE 289
#define tGE 290
#define tLE 291
#define tGT 292
#define tLT 293
#define tEQ 294
#define tNEQ 295




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
#line 137 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

