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
     RBLTIN = 280,
     RBLTINONEARG = 281,
     ID = 282,
     STRING = 283,
     HANDLER = 284,
     SYMBOL = 285,
     ENDCLAUSE = 286,
     tPLAYACCEL = 287,
     tDOWN = 288,
     tELSE = 289,
     tNLELSIF = 290,
     tEXIT = 291,
     tFRAME = 292,
     tGLOBAL = 293,
     tGO = 294,
     tIF = 295,
     tINTO = 296,
     tLOOP = 297,
     tMACRO = 298,
     tMOVIE = 299,
     tNEXT = 300,
     tOF = 301,
     tPREVIOUS = 302,
     tPUT = 303,
     tREPEAT = 304,
     tSET = 305,
     tTHEN = 306,
     tTO = 307,
     tWHEN = 308,
     tWITH = 309,
     tWHILE = 310,
     tNLELSE = 311,
     tFACTORY = 312,
     tMETHOD = 313,
     tOPEN = 314,
     tPLAY = 315,
     tDONE = 316,
     tINSTANCE = 317,
     tGE = 318,
     tLE = 319,
     tGT = 320,
     tLT = 321,
     tEQ = 322,
     tNEQ = 323,
     tAND = 324,
     tOR = 325,
     tNOT = 326,
     tMOD = 327,
     tAFTER = 328,
     tBEFORE = 329,
     tCONCAT = 330,
     tCONTAINS = 331,
     tSTARTS = 332,
     tCHAR = 333,
     tITEM = 334,
     tLINE = 335,
     tWORD = 336,
     tSPRITE = 337,
     tINTERSECTS = 338,
     tWITHIN = 339,
     tTELL = 340,
     tPROPERTY = 341,
     tON = 342,
     tME = 343
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
#define RBLTIN 280
#define RBLTINONEARG 281
#define ID 282
#define STRING 283
#define HANDLER 284
#define SYMBOL 285
#define ENDCLAUSE 286
#define tPLAYACCEL 287
#define tDOWN 288
#define tELSE 289
#define tNLELSIF 290
#define tEXIT 291
#define tFRAME 292
#define tGLOBAL 293
#define tGO 294
#define tIF 295
#define tINTO 296
#define tLOOP 297
#define tMACRO 298
#define tMOVIE 299
#define tNEXT 300
#define tOF 301
#define tPREVIOUS 302
#define tPUT 303
#define tREPEAT 304
#define tSET 305
#define tTHEN 306
#define tTO 307
#define tWHEN 308
#define tWITH 309
#define tWHILE 310
#define tNLELSE 311
#define tFACTORY 312
#define tMETHOD 313
#define tOPEN 314
#define tPLAY 315
#define tDONE 316
#define tINSTANCE 317
#define tGE 318
#define tLE 319
#define tGT 320
#define tLT 321
#define tEQ 322
#define tNEQ 323
#define tAND 324
#define tOR 325
#define tNOT 326
#define tMOD 327
#define tAFTER 328
#define tBEFORE 329
#define tCONCAT 330
#define tCONTAINS 331
#define tSTARTS 332
#define tCHAR 333
#define tITEM 334
#define tLINE 335
#define tWORD 336
#define tSPRITE 337
#define tINTERSECTS 338
#define tWITHIN 339
#define tTELL 340
#define tPROPERTY 341
#define tON 342
#define tME 343




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
#line 235 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

