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
     THEENTITY = 263,
     THEENTITYWITHID = 264,
     FLOAT = 265,
     BLTIN = 266,
     ID = 267,
     STRING = 268,
     HANDLER = 269,
     tDOWN = 270,
     tELSE = 271,
     tNLELSIF = 272,
     tEND = 273,
     tEXIT = 274,
     tFRAME = 275,
     tGLOBAL = 276,
     tGO = 277,
     tIF = 278,
     tINTO = 279,
     tLOOP = 280,
     tMACRO = 281,
     tMCI = 282,
     tMCIWAIT = 283,
     tMOVIE = 284,
     tNEXT = 285,
     tOF = 286,
     tPREVIOUS = 287,
     tPUT = 288,
     tREPEAT = 289,
     tSET = 290,
     tTHEN = 291,
     tTO = 292,
     tWITH = 293,
     tWHILE = 294,
     tNLELSE = 295,
     tFACTORY = 296,
     tMETHOD = 297,
     tGE = 298,
     tLE = 299,
     tGT = 300,
     tLT = 301,
     tEQ = 302,
     tNEQ = 303,
     tAND = 304,
     tOR = 305,
     tNOT = 306,
     tCONCAT = 307,
     tCONTAINS = 308,
     tSTARTS = 309
   };
#endif
/* Tokens.  */
#define CASTREF 258
#define UNARY 259
#define VOID 260
#define VAR 261
#define INT 262
#define THEENTITY 263
#define THEENTITYWITHID 264
#define FLOAT 265
#define BLTIN 266
#define ID 267
#define STRING 268
#define HANDLER 269
#define tDOWN 270
#define tELSE 271
#define tNLELSIF 272
#define tEND 273
#define tEXIT 274
#define tFRAME 275
#define tGLOBAL 276
#define tGO 277
#define tIF 278
#define tINTO 279
#define tLOOP 280
#define tMACRO 281
#define tMCI 282
#define tMCIWAIT 283
#define tMOVIE 284
#define tNEXT 285
#define tOF 286
#define tPREVIOUS 287
#define tPUT 288
#define tREPEAT 289
#define tSET 290
#define tTHEN 291
#define tTO 292
#define tWITH 293
#define tWHILE 294
#define tNLELSE 295
#define tFACTORY 296
#define tMETHOD 297
#define tGE 298
#define tLE 299
#define tGT 300
#define tLT 301
#define tEQ 302
#define tNEQ 303
#define tAND 304
#define tOR 305
#define tNOT 306
#define tCONCAT 307
#define tCONTAINS 308
#define tSTARTS 309




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 69 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int	i;
	double f;
	int e[2];	// Entity + field
	int code;
	int	narg;	/* number of arguments */
}
/* Line 1529 of yacc.c.  */
#line 166 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

