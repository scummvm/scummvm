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
     BLTINNOARGS = 267,
     ID = 268,
     STRING = 269,
     HANDLER = 270,
     tDOWN = 271,
     tELSE = 272,
     tNLELSIF = 273,
     tEND = 274,
     tEXIT = 275,
     tFRAME = 276,
     tGLOBAL = 277,
     tGO = 278,
     tIF = 279,
     tINTO = 280,
     tLOOP = 281,
     tMACRO = 282,
     tMCI = 283,
     tMCIWAIT = 284,
     tMOVIE = 285,
     tNEXT = 286,
     tOF = 287,
     tPREVIOUS = 288,
     tPUT = 289,
     tREPEAT = 290,
     tSET = 291,
     tTHEN = 292,
     tTO = 293,
     tWHEN = 294,
     tWITH = 295,
     tWHILE = 296,
     tNLELSE = 297,
     tFACTORY = 298,
     tMETHOD = 299,
     tGE = 300,
     tLE = 301,
     tGT = 302,
     tLT = 303,
     tEQ = 304,
     tNEQ = 305,
     tAND = 306,
     tOR = 307,
     tNOT = 308,
     tCONCAT = 309,
     tCONTAINS = 310,
     tSTARTS = 311,
     tSPRITE = 312,
     tINTERSECTS = 313,
     tWITHIN = 314
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
#define BLTINNOARGS 267
#define ID 268
#define STRING 269
#define HANDLER 270
#define tDOWN 271
#define tELSE 272
#define tNLELSIF 273
#define tEND 274
#define tEXIT 275
#define tFRAME 276
#define tGLOBAL 277
#define tGO 278
#define tIF 279
#define tINTO 280
#define tLOOP 281
#define tMACRO 282
#define tMCI 283
#define tMCIWAIT 284
#define tMOVIE 285
#define tNEXT 286
#define tOF 287
#define tPREVIOUS 288
#define tPUT 289
#define tREPEAT 290
#define tSET 291
#define tTHEN 292
#define tTO 293
#define tWHEN 294
#define tWITH 295
#define tWHILE 296
#define tNLELSE 297
#define tFACTORY 298
#define tMETHOD 299
#define tGE 300
#define tLE 301
#define tGT 302
#define tLT 303
#define tEQ 304
#define tNEQ 305
#define tAND 306
#define tOR 307
#define tNOT 308
#define tCONCAT 309
#define tCONTAINS 310
#define tSTARTS 311
#define tSPRITE 312
#define tINTERSECTS 313
#define tWITHIN 314




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
#line 176 "engines/director/lingo/lingo-gr.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

