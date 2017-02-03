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
     OBJECT = 265,
     INT = 266,
     THEENTITY = 267,
     THEENTITYWITHID = 268,
     FLOAT = 269,
     BLTIN = 270,
     BLTINNOARGS = 271,
     BLTINNOARGSORONE = 272,
     BLTINONEARG = 273,
     BLTINARGLIST = 274,
     TWOWORDBUILTIN = 275,
     ID = 276,
     STRING = 277,
     HANDLER = 278,
     SYMBOL = 279,
     ENDCLAUSE = 280,
     tPLAYACCEL = 281,
     tDOWN = 282,
     tELSE = 283,
     tNLELSIF = 284,
     tEXIT = 285,
     tFRAME = 286,
     tGLOBAL = 287,
     tGO = 288,
     tIF = 289,
     tINTO = 290,
     tLOOP = 291,
     tMACRO = 292,
     tMOVIE = 293,
     tNEXT = 294,
     tOF = 295,
     tPREVIOUS = 296,
     tPUT = 297,
     tREPEAT = 298,
     tSET = 299,
     tTHEN = 300,
     tTO = 301,
     tWHEN = 302,
     tWITH = 303,
     tWHILE = 304,
     tNLELSE = 305,
     tFACTORY = 306,
     tMETHOD = 307,
     tOPEN = 308,
     tPLAY = 309,
     tDONE = 310,
     tINSTANCE = 311,
     tGE = 312,
     tLE = 313,
     tGT = 314,
     tLT = 315,
     tEQ = 316,
     tNEQ = 317,
     tAND = 318,
     tOR = 319,
     tNOT = 320,
     tMOD = 321,
     tAFTER = 322,
     tBEFORE = 323,
     tCONCAT = 324,
     tCONTAINS = 325,
     tSTARTS = 326,
     tCHAR = 327,
     tITEM = 328,
     tLINE = 329,
     tWORD = 330,
     tSPRITE = 331,
     tINTERSECTS = 332,
     tWITHIN = 333,
     tON = 334,
     tME = 335
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
#define OBJECT 265
#define INT 266
#define THEENTITY 267
#define THEENTITYWITHID 268
#define FLOAT 269
#define BLTIN 270
#define BLTINNOARGS 271
#define BLTINNOARGSORONE 272
#define BLTINONEARG 273
#define BLTINARGLIST 274
#define TWOWORDBUILTIN 275
#define ID 276
#define STRING 277
#define HANDLER 278
#define SYMBOL 279
#define ENDCLAUSE 280
#define tPLAYACCEL 281
#define tDOWN 282
#define tELSE 283
#define tNLELSIF 284
#define tEXIT 285
#define tFRAME 286
#define tGLOBAL 287
#define tGO 288
#define tIF 289
#define tINTO 290
#define tLOOP 291
#define tMACRO 292
#define tMOVIE 293
#define tNEXT 294
#define tOF 295
#define tPREVIOUS 296
#define tPUT 297
#define tREPEAT 298
#define tSET 299
#define tTHEN 300
#define tTO 301
#define tWHEN 302
#define tWITH 303
#define tWHILE 304
#define tNLELSE 305
#define tFACTORY 306
#define tMETHOD 307
#define tOPEN 308
#define tPLAY 309
#define tDONE 310
#define tINSTANCE 311
#define tGE 312
#define tLE 313
#define tGT 314
#define tLT 315
#define tEQ 316
#define tNEQ 317
#define tAND 318
#define tOR 319
#define tNOT 320
#define tMOD 321
#define tAFTER 322
#define tBEFORE 323
#define tCONCAT 324
#define tCONTAINS 325
#define tSTARTS 326
#define tCHAR 327
#define tITEM 328
#define tLINE 329
#define tWORD 330
#define tSPRITE 331
#define tINTERSECTS 332
#define tWITHIN 333
#define tON 334
#define tME 335




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 79 "engines/director/lingo/lingo-gr.y"
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
#line 219 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

