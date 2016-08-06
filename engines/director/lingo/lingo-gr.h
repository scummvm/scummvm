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
     CASTREF = 259,
     VOID = 260,
     VAR = 261,
     POINT = 262,
     RECT = 263,
     ARRAY = 264,
     SYMBOL = 265,
     INT = 266,
     THEENTITY = 267,
     THEENTITYWITHID = 268,
     FLOAT = 269,
     BLTIN = 270,
     BLTINNOARGS = 271,
     BLTINNOARGSORONE = 272,
     BLTINONEARG = 273,
     ID = 274,
     STRING = 275,
     HANDLER = 276,
     tDOWN = 277,
     tELSE = 278,
     tNLELSIF = 279,
     tEND = 280,
     tEXIT = 281,
     tFRAME = 282,
     tGLOBAL = 283,
     tGO = 284,
     tIF = 285,
     tINTO = 286,
     tLOOP = 287,
     tMACRO = 288,
     tMCI = 289,
     tMCIWAIT = 290,
     tMOVIE = 291,
     tNEXT = 292,
     tOF = 293,
     tPREVIOUS = 294,
     tPUT = 295,
     tREPEAT = 296,
     tSET = 297,
     tTHEN = 298,
     tTO = 299,
     tWHEN = 300,
     tWITH = 301,
     tWHILE = 302,
     tNLELSE = 303,
     tFACTORY = 304,
     tMETHOD = 305,
     tGE = 306,
     tLE = 307,
     tGT = 308,
     tLT = 309,
     tEQ = 310,
     tNEQ = 311,
     tAND = 312,
     tOR = 313,
     tNOT = 314,
     tCONCAT = 315,
     tCONTAINS = 316,
     tSTARTS = 317,
     tSPRITE = 318,
     tINTERSECTS = 319,
     tWITHIN = 320
   };
#endif
/* Tokens.  */
#define UNARY 258
#define CASTREF 259
#define VOID 260
#define VAR 261
#define POINT 262
#define RECT 263
#define ARRAY 264
#define SYMBOL 265
#define INT 266
#define THEENTITY 267
#define THEENTITYWITHID 268
#define FLOAT 269
#define BLTIN 270
#define BLTINNOARGS 271
#define BLTINNOARGSORONE 272
#define BLTINONEARG 273
#define ID 274
#define STRING 275
#define HANDLER 276
#define tDOWN 277
#define tELSE 278
#define tNLELSIF 279
#define tEND 280
#define tEXIT 281
#define tFRAME 282
#define tGLOBAL 283
#define tGO 284
#define tIF 285
#define tINTO 286
#define tLOOP 287
#define tMACRO 288
#define tMCI 289
#define tMCIWAIT 290
#define tMOVIE 291
#define tNEXT 292
#define tOF 293
#define tPREVIOUS 294
#define tPUT 295
#define tREPEAT 296
#define tSET 297
#define tTHEN 298
#define tTO 299
#define tWHEN 300
#define tWITH 301
#define tWHILE 302
#define tNLELSE 303
#define tFACTORY 304
#define tMETHOD 305
#define tGE 306
#define tLE 307
#define tGT 308
#define tLT 309
#define tEQ 310
#define tNEQ 311
#define tAND 312
#define tOR 313
#define tNOT 314
#define tCONCAT 315
#define tCONTAINS 316
#define tSTARTS 317
#define tSPRITE 318
#define tINTERSECTS 319
#define tWITHIN 320




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
#line 189 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

