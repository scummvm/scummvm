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
     BLTINARGLIST = 274,
     ID = 275,
     STRING = 276,
     HANDLER = 277,
     tDOWN = 278,
     tELSE = 279,
     tNLELSIF = 280,
     tEND = 281,
     tEXIT = 282,
     tFRAME = 283,
     tGLOBAL = 284,
     tGO = 285,
     tIF = 286,
     tINTO = 287,
     tLOOP = 288,
     tMACRO = 289,
     tMOVIE = 290,
     tNEXT = 291,
     tOF = 292,
     tPREVIOUS = 293,
     tPUT = 294,
     tREPEAT = 295,
     tSET = 296,
     tTHEN = 297,
     tTO = 298,
     tWHEN = 299,
     tWITH = 300,
     tWHILE = 301,
     tNLELSE = 302,
     tFACTORY = 303,
     tMETHOD = 304,
     tOPEN = 305,
     tPLAY = 306,
     tDONE = 307,
     tPLAYACCEL = 308,
     tGE = 309,
     tLE = 310,
     tGT = 311,
     tLT = 312,
     tEQ = 313,
     tNEQ = 314,
     tAND = 315,
     tOR = 316,
     tNOT = 317,
     tCONCAT = 318,
     tCONTAINS = 319,
     tSTARTS = 320,
     tSPRITE = 321,
     tINTERSECTS = 322,
     tWITHIN = 323
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
#define BLTINARGLIST 274
#define ID 275
#define STRING 276
#define HANDLER 277
#define tDOWN 278
#define tELSE 279
#define tNLELSIF 280
#define tEND 281
#define tEXIT 282
#define tFRAME 283
#define tGLOBAL 284
#define tGO 285
#define tIF 286
#define tINTO 287
#define tLOOP 288
#define tMACRO 289
#define tMOVIE 290
#define tNEXT 291
#define tOF 292
#define tPREVIOUS 293
#define tPUT 294
#define tREPEAT 295
#define tSET 296
#define tTHEN 297
#define tTO 298
#define tWHEN 299
#define tWITH 300
#define tWHILE 301
#define tNLELSE 302
#define tFACTORY 303
#define tMETHOD 304
#define tOPEN 305
#define tPLAY 306
#define tDONE 307
#define tPLAYACCEL 308
#define tGE 309
#define tLE 310
#define tGT 311
#define tLT 312
#define tEQ 313
#define tNEQ 314
#define tAND 315
#define tOR 316
#define tNOT 317
#define tCONCAT 318
#define tCONTAINS 319
#define tSTARTS 320
#define tSPRITE 321
#define tINTERSECTS 322
#define tWITHIN 323




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
#line 195 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

