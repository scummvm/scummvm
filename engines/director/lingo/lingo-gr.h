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
     EOSTREAM = 258,
     UNARY = 259,
     CASTREF = 260,
     VOID = 261,
     VAR = 262,
     POINT = 263,
     RECT = 264,
     ARRAY = 265,
     OBJECT = 266,
     REFERENCE = 267,
     INT = 268,
     THEENTITY = 269,
     THEENTITYWITHID = 270,
     FLOAT = 271,
     BLTIN = 272,
     BLTINNOARGS = 273,
     BLTINNOARGSORONE = 274,
     BLTINONEARG = 275,
     BLTINARGLIST = 276,
     TWOWORDBUILTIN = 277,
     FBLTIN = 278,
     FBLTINNOARGS = 279,
     FBLTINONEARG = 280,
     FBLTINARGLIST = 281,
     RBLTIN = 282,
     RBLTINONEARG = 283,
     ID = 284,
     STRING = 285,
     HANDLER = 286,
     SYMBOL = 287,
     ENDCLAUSE = 288,
     tPLAYACCEL = 289,
     tDOWN = 290,
     tELSE = 291,
     tNLELSIF = 292,
     tEXIT = 293,
     tFRAME = 294,
     tGLOBAL = 295,
     tGO = 296,
     tIF = 297,
     tINTO = 298,
     tLOOP = 299,
     tMACRO = 300,
     tMOVIE = 301,
     tNEXT = 302,
     tOF = 303,
     tPREVIOUS = 304,
     tPUT = 305,
     tREPEAT = 306,
     tSET = 307,
     tTHEN = 308,
     tTO = 309,
     tWHEN = 310,
     tWITH = 311,
     tWHILE = 312,
     tNLELSE = 313,
     tFACTORY = 314,
     tMETHOD = 315,
     tOPEN = 316,
     tPLAY = 317,
     tDONE = 318,
     tINSTANCE = 319,
     tGE = 320,
     tLE = 321,
     tGT = 322,
     tLT = 323,
     tEQ = 324,
     tNEQ = 325,
     tAND = 326,
     tOR = 327,
     tNOT = 328,
     tMOD = 329,
     tAFTER = 330,
     tBEFORE = 331,
     tCONCAT = 332,
     tCONTAINS = 333,
     tSTARTS = 334,
     tCHAR = 335,
     tITEM = 336,
     tLINE = 337,
     tWORD = 338,
     tSPRITE = 339,
     tINTERSECTS = 340,
     tWITHIN = 341,
     tTELL = 342,
     tPROPERTY = 343,
     tON = 344,
     tME = 345
   };
#endif
/* Tokens.  */
#define EOSTREAM 258
#define UNARY 259
#define CASTREF 260
#define VOID 261
#define VAR 262
#define POINT 263
#define RECT 264
#define ARRAY 265
#define OBJECT 266
#define REFERENCE 267
#define INT 268
#define THEENTITY 269
#define THEENTITYWITHID 270
#define FLOAT 271
#define BLTIN 272
#define BLTINNOARGS 273
#define BLTINNOARGSORONE 274
#define BLTINONEARG 275
#define BLTINARGLIST 276
#define TWOWORDBUILTIN 277
#define FBLTIN 278
#define FBLTINNOARGS 279
#define FBLTINONEARG 280
#define FBLTINARGLIST 281
#define RBLTIN 282
#define RBLTINONEARG 283
#define ID 284
#define STRING 285
#define HANDLER 286
#define SYMBOL 287
#define ENDCLAUSE 288
#define tPLAYACCEL 289
#define tDOWN 290
#define tELSE 291
#define tNLELSIF 292
#define tEXIT 293
#define tFRAME 294
#define tGLOBAL 295
#define tGO 296
#define tIF 297
#define tINTO 298
#define tLOOP 299
#define tMACRO 300
#define tMOVIE 301
#define tNEXT 302
#define tOF 303
#define tPREVIOUS 304
#define tPUT 305
#define tREPEAT 306
#define tSET 307
#define tTHEN 308
#define tTO 309
#define tWHEN 310
#define tWITH 311
#define tWHILE 312
#define tNLELSE 313
#define tFACTORY 314
#define tMETHOD 315
#define tOPEN 316
#define tPLAY 317
#define tDONE 318
#define tINSTANCE 319
#define tGE 320
#define tLE 321
#define tGT 322
#define tLT 323
#define tEQ 324
#define tNEQ 325
#define tAND 326
#define tOR 327
#define tNOT 328
#define tMOD 329
#define tAFTER 330
#define tBEFORE 331
#define tCONCAT 332
#define tCONTAINS 333
#define tSTARTS 334
#define tCHAR 335
#define tITEM 336
#define tLINE 337
#define tWORD 338
#define tSPRITE 339
#define tINTERSECTS 340
#define tWITHIN 341
#define tTELL 342
#define tPROPERTY 343
#define tON 344
#define tME 345




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
#line 239 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

