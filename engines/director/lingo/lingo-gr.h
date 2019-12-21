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
     REFERENCE = 266,
     INT = 267,
     ARGC = 268,
     ARGCNORET = 269,
     THEENTITY = 270,
     THEENTITYWITHID = 271,
     FLOAT = 272,
     BLTIN = 273,
     BLTINNOARGS = 274,
     BLTINNOARGSORONE = 275,
     BLTINONEARG = 276,
     BLTINARGLIST = 277,
     TWOWORDBUILTIN = 278,
     FBLTIN = 279,
     FBLTINNOARGS = 280,
     FBLTINONEARG = 281,
     FBLTINARGLIST = 282,
     RBLTIN = 283,
     RBLTINONEARG = 284,
     ID = 285,
     STRING = 286,
     HANDLER = 287,
     SYMBOL = 288,
     ENDCLAUSE = 289,
     tPLAYACCEL = 290,
     tMETHOD = 291,
     tDOWN = 292,
     tELSE = 293,
     tELSIF = 294,
     tEXIT = 295,
     tFRAME = 296,
     tGLOBAL = 297,
     tGO = 298,
     tIF = 299,
     tINTO = 300,
     tLOOP = 301,
     tMACRO = 302,
     tMOVIE = 303,
     tNEXT = 304,
     tOF = 305,
     tPREVIOUS = 306,
     tPUT = 307,
     tREPEAT = 308,
     tSET = 309,
     tTHEN = 310,
     tTO = 311,
     tWHEN = 312,
     tWITH = 313,
     tWHILE = 314,
     tNLELSE = 315,
     tFACTORY = 316,
     tOPEN = 317,
     tPLAY = 318,
     tDONE = 319,
     tINSTANCE = 320,
     tGE = 321,
     tLE = 322,
     tEQ = 323,
     tNEQ = 324,
     tAND = 325,
     tOR = 326,
     tNOT = 327,
     tMOD = 328,
     tAFTER = 329,
     tBEFORE = 330,
     tCONCAT = 331,
     tCONTAINS = 332,
     tSTARTS = 333,
     tCHAR = 334,
     tITEM = 335,
     tLINE = 336,
     tWORD = 337,
     tSPRITE = 338,
     tINTERSECTS = 339,
     tWITHIN = 340,
     tTELL = 341,
     tPROPERTY = 342,
     tON = 343,
     tME = 344,
     tENDIF = 345,
     tENDREPEAT = 346,
     tENDTELL = 347
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
#define REFERENCE 266
#define INT 267
#define ARGC 268
#define ARGCNORET 269
#define THEENTITY 270
#define THEENTITYWITHID 271
#define FLOAT 272
#define BLTIN 273
#define BLTINNOARGS 274
#define BLTINNOARGSORONE 275
#define BLTINONEARG 276
#define BLTINARGLIST 277
#define TWOWORDBUILTIN 278
#define FBLTIN 279
#define FBLTINNOARGS 280
#define FBLTINONEARG 281
#define FBLTINARGLIST 282
#define RBLTIN 283
#define RBLTINONEARG 284
#define ID 285
#define STRING 286
#define HANDLER 287
#define SYMBOL 288
#define ENDCLAUSE 289
#define tPLAYACCEL 290
#define tMETHOD 291
#define tDOWN 292
#define tELSE 293
#define tELSIF 294
#define tEXIT 295
#define tFRAME 296
#define tGLOBAL 297
#define tGO 298
#define tIF 299
#define tINTO 300
#define tLOOP 301
#define tMACRO 302
#define tMOVIE 303
#define tNEXT 304
#define tOF 305
#define tPREVIOUS 306
#define tPUT 307
#define tREPEAT 308
#define tSET 309
#define tTHEN 310
#define tTO 311
#define tWHEN 312
#define tWITH 313
#define tWHILE 314
#define tNLELSE 315
#define tFACTORY 316
#define tOPEN 317
#define tPLAY 318
#define tDONE 319
#define tINSTANCE 320
#define tGE 321
#define tLE 322
#define tEQ 323
#define tNEQ 324
#define tAND 325
#define tOR 326
#define tNOT 327
#define tMOD 328
#define tAFTER 329
#define tBEFORE 330
#define tCONCAT 331
#define tCONTAINS 332
#define tSTARTS 333
#define tCHAR 334
#define tITEM 335
#define tLINE 336
#define tWORD 337
#define tSPRITE 338
#define tINTERSECTS 339
#define tWITHIN 340
#define tTELL 341
#define tPROPERTY 342
#define tON 343
#define tME 344
#define tENDIF 345
#define tENDREPEAT 346
#define tENDTELL 347




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 82 "engines/director/lingo/lingo-gr.y"
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
#line 243 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

