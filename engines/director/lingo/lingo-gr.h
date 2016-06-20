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
     UNDEF = 259,
     INT = 260,
     FLOAT = 261,
     VAR = 262,
     STRING = 263,
     tIF = 264,
     tEND = 265,
     tFRAME = 266,
     tGO = 267,
     tINTO = 268,
     tLOOP = 269,
     tMCI = 270,
     tMCIWAIT = 271,
     tMOVIE = 272,
     tNEXT = 273,
     tOF = 274,
     tPREVIOUS = 275,
     tPUT = 276,
     tSET = 277,
     tTHEN = 278,
     tTO = 279,
     tGE = 280,
     tLE = 281,
     tGT = 282,
     tLT = 283,
     tEQ = 284,
     tNEQ = 285
   };
#endif
/* Tokens.  */
#define UNARY 258
#define UNDEF 259
#define INT 260
#define FLOAT 261
#define VAR 262
#define STRING 263
#define tIF 264
#define tEND 265
#define tFRAME 266
#define tGO 267
#define tINTO 268
#define tLOOP 269
#define tMCI 270
#define tMCIWAIT 271
#define tMOVIE 272
#define tNEXT 273
#define tOF 274
#define tPREVIOUS 275
#define tPUT 276
#define tSET 277
#define tTHEN 278
#define tTO 279
#define tGE 280
#define tLE 281
#define tGT 282
#define tLT 283
#define tEQ 284
#define tNEQ 285




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
#line 116 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

