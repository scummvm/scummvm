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
     POINT = 262,
     RECT = 263,
     ARRAY = 264,
     INT = 265,
     THEENTITY = 266,
     THEENTITYWITHID = 267,
     FLOAT = 268,
     BLTIN = 269,
     BLTINNOARGS = 270,
     ID = 271,
     STRING = 272,
     HANDLER = 273,
     tDOWN = 274,
     tELSE = 275,
     tNLELSIF = 276,
     tEND = 277,
     tEXIT = 278,
     tFRAME = 279,
     tGLOBAL = 280,
     tGO = 281,
     tIF = 282,
     tINTO = 283,
     tLOOP = 284,
     tMACRO = 285,
     tMCI = 286,
     tMCIWAIT = 287,
     tMOVIE = 288,
     tNEXT = 289,
     tOF = 290,
     tPREVIOUS = 291,
     tPUT = 292,
     tREPEAT = 293,
     tSET = 294,
     tTHEN = 295,
     tTO = 296,
     tWHEN = 297,
     tWITH = 298,
     tWHILE = 299,
     tNLELSE = 300,
     tFACTORY = 301,
     tMETHOD = 302,
     tGE = 303,
     tLE = 304,
     tGT = 305,
     tLT = 306,
     tEQ = 307,
     tNEQ = 308,
     tAND = 309,
     tOR = 310,
     tNOT = 311,
     tCONCAT = 312,
     tCONTAINS = 313,
     tSTARTS = 314,
     tSPRITE = 315,
     tINTERSECTS = 316,
     tWITHIN = 317
   };
#endif
/* Tokens.  */
#define CASTREF 258
#define UNARY 259
#define VOID 260
#define VAR 261
#define POINT 262
#define RECT 263
#define ARRAY 264
#define INT 265
#define THEENTITY 266
#define THEENTITYWITHID 267
#define FLOAT 268
#define BLTIN 269
#define BLTINNOARGS 270
#define ID 271
#define STRING 272
#define HANDLER 273
#define tDOWN 274
#define tELSE 275
#define tNLELSIF 276
#define tEND 277
#define tEXIT 278
#define tFRAME 279
#define tGLOBAL 280
#define tGO 281
#define tIF 282
#define tINTO 283
#define tLOOP 284
#define tMACRO 285
#define tMCI 286
#define tMCIWAIT 287
#define tMOVIE 288
#define tNEXT 289
#define tOF 290
#define tPREVIOUS 291
#define tPUT 292
#define tREPEAT 293
#define tSET 294
#define tTHEN 295
#define tTO 296
#define tWHEN 297
#define tWITH 298
#define tWHILE 299
#define tNLELSE 300
#define tFACTORY 301
#define tMETHOD 302
#define tGE 303
#define tLE 304
#define tGT 305
#define tLT 306
#define tEQ 307
#define tNEQ 308
#define tAND 309
#define tOR 310
#define tNOT 311
#define tCONCAT 312
#define tCONTAINS 313
#define tSTARTS 314
#define tSPRITE 315
#define tINTERSECTS 316
#define tWITHIN 317




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 69 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Common::Array<double> *arr;
}
/* Line 1529 of yacc.c.  */
#line 183 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

