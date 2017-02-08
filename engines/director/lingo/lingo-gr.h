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
     ID = 279,
     STRING = 280,
     HANDLER = 281,
     SYMBOL = 282,
     ENDCLAUSE = 283,
     tPLAYACCEL = 284,
     tDOWN = 285,
     tELSE = 286,
     tNLELSIF = 287,
     tEXIT = 288,
     tFRAME = 289,
     tGLOBAL = 290,
     tGO = 291,
     tIF = 292,
     tINTO = 293,
     tLOOP = 294,
     tMACRO = 295,
     tMOVIE = 296,
     tNEXT = 297,
     tOF = 298,
     tPREVIOUS = 299,
     tPUT = 300,
     tREPEAT = 301,
     tSET = 302,
     tTHEN = 303,
     tTO = 304,
     tWHEN = 305,
     tWITH = 306,
     tWHILE = 307,
     tNLELSE = 308,
     tFACTORY = 309,
     tMETHOD = 310,
     tOPEN = 311,
     tPLAY = 312,
     tDONE = 313,
     tINSTANCE = 314,
     tGE = 315,
     tLE = 316,
     tGT = 317,
     tLT = 318,
     tEQ = 319,
     tNEQ = 320,
     tAND = 321,
     tOR = 322,
     tNOT = 323,
     tMOD = 324,
     tAFTER = 325,
     tBEFORE = 326,
     tCONCAT = 327,
     tCONTAINS = 328,
     tSTARTS = 329,
     tCHAR = 330,
     tITEM = 331,
     tLINE = 332,
     tWORD = 333,
     tSPRITE = 334,
     tINTERSECTS = 335,
     tWITHIN = 336,
     tTELL = 337,
     tPROPERTY = 338,
     tON = 339,
     tME = 340
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
#define ID 279
#define STRING 280
#define HANDLER 281
#define SYMBOL 282
#define ENDCLAUSE 283
#define tPLAYACCEL 284
#define tDOWN 285
#define tELSE 286
#define tNLELSIF 287
#define tEXIT 288
#define tFRAME 289
#define tGLOBAL 290
#define tGO 291
#define tIF 292
#define tINTO 293
#define tLOOP 294
#define tMACRO 295
#define tMOVIE 296
#define tNEXT 297
#define tOF 298
#define tPREVIOUS 299
#define tPUT 300
#define tREPEAT 301
#define tSET 302
#define tTHEN 303
#define tTO 304
#define tWHEN 305
#define tWITH 306
#define tWHILE 307
#define tNLELSE 308
#define tFACTORY 309
#define tMETHOD 310
#define tOPEN 311
#define tPLAY 312
#define tDONE 313
#define tINSTANCE 314
#define tGE 315
#define tLE 316
#define tGT 317
#define tLT 318
#define tEQ 319
#define tNEQ 320
#define tAND 321
#define tOR 322
#define tNOT 323
#define tMOD 324
#define tAFTER 325
#define tBEFORE 326
#define tCONCAT 327
#define tCONTAINS 328
#define tSTARTS 329
#define tCHAR 330
#define tITEM 331
#define tLINE 332
#define tWORD 333
#define tSPRITE 334
#define tINTERSECTS 335
#define tWITHIN 336
#define tTELL 337
#define tPROPERTY 338
#define tON 339
#define tME 340




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
#line 229 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

