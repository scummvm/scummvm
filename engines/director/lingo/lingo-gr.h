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
     tNLELSIF = 270,
     tENDIF = 271,
     tENDREPEAT = 272,
     tEXIT = 273,
     tFRAME = 274,
     tGLOBAL = 275,
     tGO = 276,
     tIF = 277,
     tINTO = 278,
     tLOOP = 279,
     tMACRO = 280,
     tMCI = 281,
     tMCIWAIT = 282,
     tMOVIE = 283,
     tNEXT = 284,
     tOF = 285,
     tPREVIOUS = 286,
     tPUT = 287,
     tREPEAT = 288,
     tSET = 289,
     tTHEN = 290,
     tTO = 291,
     tWITH = 292,
     tWHILE = 293,
     tNLELSE = 294,
     tGE = 295,
     tLE = 296,
     tGT = 297,
     tLT = 298,
     tEQ = 299,
     tNEQ = 300,
     tAND = 301,
     tOR = 302,
     tNOT = 303,
     tCONCAT = 304,
     tCONTAINS = 305,
     tSTARTS = 306
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
#define tNLELSIF 270
#define tENDIF 271
#define tENDREPEAT 272
#define tEXIT 273
#define tFRAME 274
#define tGLOBAL 275
#define tGO 276
#define tIF 277
#define tINTO 278
#define tLOOP 279
#define tMACRO 280
#define tMCI 281
#define tMCIWAIT 282
#define tMOVIE 283
#define tNEXT 284
#define tOF 285
#define tPREVIOUS 286
#define tPUT 287
#define tREPEAT 288
#define tSET 289
#define tTHEN 290
#define tTO 291
#define tWITH 292
#define tWHILE 293
#define tNLELSE 294
#define tGE 295
#define tLE 296
#define tGT 297
#define tLT 298
#define tEQ 299
#define tNEQ 300
#define tAND 301
#define tOR 302
#define tNOT 303
#define tCONCAT 304
#define tCONTAINS 305
#define tSTARTS 306




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 66 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int	i;
	double f;
	int code;
	int	narg;	/* number of arguments */
}
/* Line 1529 of yacc.c.  */
#line 159 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

