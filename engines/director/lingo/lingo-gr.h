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
     FBLTIN = 276,
     FBLTINNOARGS = 277,
     FBLTINONEARG = 278,
     FBLTINARGLIST = 279,
     ID = 280,
     STRING = 281,
     HANDLER = 282,
     SYMBOL = 283,
     ENDCLAUSE = 284,
     tPLAYACCEL = 285,
     tDOWN = 286,
     tELSE = 287,
     tNLELSIF = 288,
     tEXIT = 289,
     tFRAME = 290,
     tGLOBAL = 291,
     tGO = 292,
     tIF = 293,
     tINTO = 294,
     tLOOP = 295,
     tMACRO = 296,
     tMOVIE = 297,
     tNEXT = 298,
     tOF = 299,
     tPREVIOUS = 300,
     tPUT = 301,
     tREPEAT = 302,
     tSET = 303,
     tTHEN = 304,
     tTO = 305,
     tWHEN = 306,
     tWITH = 307,
     tWHILE = 308,
     tNLELSE = 309,
     tFACTORY = 310,
     tMETHOD = 311,
     tOPEN = 312,
     tPLAY = 313,
     tDONE = 314,
     tINSTANCE = 315,
     tGE = 316,
     tLE = 317,
     tGT = 318,
     tLT = 319,
     tEQ = 320,
     tNEQ = 321,
     tAND = 322,
     tOR = 323,
     tNOT = 324,
     tMOD = 325,
     tAFTER = 326,
     tBEFORE = 327,
     tCONCAT = 328,
     tCONTAINS = 329,
     tSTARTS = 330,
     tCHAR = 331,
     tITEM = 332,
     tLINE = 333,
     tWORD = 334,
     tSPRITE = 335,
     tINTERSECTS = 336,
     tWITHIN = 337,
     tTELL = 338,
     tPROPERTY = 339,
     tON = 340,
     tME = 341
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
#define FBLTIN 276
#define FBLTINNOARGS 277
#define FBLTINONEARG 278
#define FBLTINARGLIST 279
#define ID 280
#define STRING 281
#define HANDLER 282
#define SYMBOL 283
#define ENDCLAUSE 284
#define tPLAYACCEL 285
#define tDOWN 286
#define tELSE 287
#define tNLELSIF 288
#define tEXIT 289
#define tFRAME 290
#define tGLOBAL 291
#define tGO 292
#define tIF 293
#define tINTO 294
#define tLOOP 295
#define tMACRO 296
#define tMOVIE 297
#define tNEXT 298
#define tOF 299
#define tPREVIOUS 300
#define tPUT 301
#define tREPEAT 302
#define tSET 303
#define tTHEN 304
#define tTO 305
#define tWHEN 306
#define tWITH 307
#define tWHILE 308
#define tNLELSE 309
#define tFACTORY 310
#define tMETHOD 311
#define tOPEN 312
#define tPLAY 313
#define tDONE 314
#define tINSTANCE 315
#define tGE 316
#define tLE 317
#define tGT 318
#define tLT 319
#define tEQ 320
#define tNEQ 321
#define tAND 322
#define tOR 323
#define tNOT 324
#define tMOD 325
#define tAFTER 326
#define tBEFORE 327
#define tCONCAT 328
#define tCONTAINS 329
#define tSTARTS 330
#define tCHAR 331
#define tITEM 332
#define tLINE 333
#define tWORD 334
#define tSPRITE 335
#define tINTERSECTS 336
#define tWITHIN 337
#define tTELL 338
#define tPROPERTY 339
#define tON 340
#define tME 341




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
#line 231 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

