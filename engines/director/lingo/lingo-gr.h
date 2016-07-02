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
     BLTIN = 263,
     ID = 264,
     STRING = 265,
     HANDLER = 266,
     tDOWN = 267,
     tELSE = 268,
     tEND = 269,
     tEXIT = 270,
     tFRAME = 271,
     tGLOBAL = 272,
     tGO = 273,
     tIF = 274,
     tINTO = 275,
     tLOOP = 276,
     tMACRO = 277,
     tMCI = 278,
     tMCIWAIT = 279,
     tMOVIE = 280,
     tNEXT = 281,
     tOF = 282,
     tPREVIOUS = 283,
     tPUT = 284,
     tREPEAT = 285,
     tSET = 286,
     tTHEN = 287,
     tTO = 288,
     tWITH = 289,
     tWHILE = 290,
     tGE = 291,
     tLE = 292,
     tGT = 293,
     tLT = 294,
     tEQ = 295,
     tNEQ = 296
   };
#endif
/* Tokens.  */
#define UNARY 258
#define VOID 259
#define VAR 260
#define INT 261
#define FLOAT 262
#define BLTIN 263
#define ID 264
#define STRING 265
#define HANDLER 266
#define tDOWN 267
#define tELSE 268
#define tEND 269
#define tEXIT 270
#define tFRAME 271
#define tGLOBAL 272
#define tGO 273
#define tIF 274
#define tINTO 275
#define tLOOP 276
#define tMACRO 277
#define tMCI 278
#define tMCIWAIT 279
#define tMOVIE 280
#define tNEXT 281
#define tOF 282
#define tPREVIOUS 283
#define tPUT 284
#define tREPEAT 285
#define tSET 286
#define tTHEN 287
#define tTO 288
#define tWITH 289
#define tWHILE 290
#define tGE 291
#define tLE 292
#define tGT 293
#define tLT 294
#define tEQ 295
#define tNEQ 296




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
#line 139 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

